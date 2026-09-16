#include "NeutralAnalyzer.h"
#include "KeystoneCoreCPU.h"
#include <array>
#include <cmath>
#include <cstddef>
namespace keystone {
using namespace keystone_cpu;
static constexpr int NX=6,NY=4,N=24; static constexpr float P=6.0f;
static float l2(float x){return std::log(std::max(x,1e-12f))/0.6931471805599453f;}
static float e2(float x){return std::pow(2.0f,x);}
static float max3(float3 v){return std::max(v.x,std::max(v.y,v.z));}
static float min3(float3 v){return std::min(v.x,std::min(v.y,v.z));}
static float3 clampGain(float3 g){return make_float3(clampf(g.x,.25f,4.f),clampf(g.y,.25f,4.f),clampf(g.z,.25f,4.f));}
static float3 gainFromIllum(float3 e){
  e=make_float3(std::max(e.x,1e-6f),std::max(e.y,1e-6f),std::max(e.z,1e-6f));float m=(e.x+e.y+e.z)/3.f;
  float3 g=clampGain(make_float3(m/e.x,m/e.y,m/e.z));float gy=awg4_to_xyz(g).y;if(gy>1e-8f)g/=gy;return clampGain(g);
}
static float gainDist(float3 a,float3 b){return std::max(std::fabs(l2(a.x/b.x)),std::max(std::fabs(l2(a.y/b.y)),std::fabs(l2(a.z/b.z))));}
static float3 logMix(float3 a,float wa,float3 b,float wb,float3 c,float wc,float3 d,float wd){
  float sum=std::max(wa+wb+wc+wd,1e-8f);float3 l=make_float3((l2(a.x)*wa+l2(b.x)*wb+l2(c.x)*wc+l2(d.x)*wd)/sum,(l2(a.y)*wa+l2(b.y)*wb+l2(c.y)*wc+l2(d.y)*wd)/sum,(l2(a.z)*wa+l2(b.z)*wb+l2(c.z)*wc+l2(d.z)*wd)/sum);return make_float3(e2(l.x),e2(l.y),e2(l.z));
}
static float hueDist(float a,float b){float d=std::fabs(a-b);return std::min(d,1.f-d);}

// Skin-first Auto Neutral. When credible skin is present, skin-line alignment is
// the primary objective. The scene-neutral estimate remains a sanity/fallback
// reference rather than overriding the skin solution.
static float3 rgbToYCbCr709(float3 rgb){
  // Full-range BT.709 Y'CbCr. rgb is Keystone-style Rec.709 / gamma 2.4.
  float y = 0.2126f*rgb.x + 0.7152f*rgb.y + 0.0722f*rgb.z;
  float cb = 0.5f + (rgb.z-y)/1.8556f;
  float cr = 0.5f + (rgb.x-y)/1.5748f;
  return make_float3(y,cb,cr);
}

// Image-adaptive skin detector for Auto Match. The detector deliberately does
// NOT use the skin-line hue as its classifier. It first builds a broad Y'CbCr
// candidate population, finds the dominant chroma cluster for this frame, then
// keeps only spatially coherent cells around that cluster. The skin line is
// used later only as the correction destination.
static float3 skinDrivenGain(const float* src,int width,int height,int stride,const Params& params,
                             float3 fallback,float &supportOut,float &improveOut){
  struct Cand { float3 lin; float cb,cr,y,w; int cell; };
  static constexpr int MAXC=8192, CX=12, CY=8, NC=CX*CY;
  std::array<Cand,MAXC> c{}; std::array<float,NC> cellW{}; std::array<int,NC> cellN{};
  int n=0; const int step=std::max(3,std::min(width,height)/128);
  // Stage 1: intentionally broad skin plausibility in display chroma. This is
  // only a seed population; it is not trusted until clustering + spatial tests.
  for(int y=step/2;y<height;y+=step) for(int x=step/2;x<width;x+=step){
    const float* px=src+(std::ptrdiff_t)y*stride+x*4;
    float3 lin=preNeutral(make_float3(px[0],px[1],px[2]),params);
    if(!(keystone_cpu::finitef(lin.x)&&keystone_cpu::finitef(lin.y)&&keystone_cpu::finitef(lin.z))) continue;
    float3 rgb=ks_awg4_to_ks_skin_rec709(lin); if(!(keystone_cpu::finitef(rgb.x)&&keystone_cpu::finitef(rgb.y)&&keystone_cpu::finitef(rgb.z))) continue;
    float3 yc=rgbToYCbCr709(rgb); float Y=yc.x,Cb=yc.y,Cr=yc.z;
    if(Y<.055f||Y>.94f) continue;
    // Broad human-skin envelope; adaptive clustering below chooses the actual
    // frame-specific center. Avoid a hue/skin-line test here on purpose.
    if(Cb<.20f||Cb>.62f||Cr<.38f||Cr>.78f) continue;
    float chroma=std::sqrt((Cb-.5f)*(Cb-.5f)+(Cr-.5f)*(Cr-.5f));
    if(chroma<.018f||chroma>.34f) continue;
    float lumW=smootherstep(.055f,.16f,Y)*(1.f-smootherstep(.78f,.94f,Y));
    float w=std::max(lumW,.05f); int cx=std::min(CX-1,std::max(0,x*CX/std::max(width,1))); int cy=std::min(CY-1,std::max(0,y*CY/std::max(height,1))); int cell=cy*CX+cx;
    if(n<MAXC){c[n++]={lin,Cb,Cr,Y,w,cell};cellW[cell]+=w;cellN[cell]++;}
  }
  if(n<18){supportOut=improveOut=0.f;return fallback;}

  // Stage 2: robust frame-specific chroma center. Iterative trimmed mean makes
  // the center follow the dominant skin-like population instead of a fixed hue.
  double sw=0,scb=0,scr=0; for(int i=0;i<n;i++){sw+=c[i].w;scb+=c[i].w*c[i].cb;scr+=c[i].w*c[i].cr;}
  float mcb=(float)(scb/std::max(sw,1e-8)), mcr=(float)(scr/std::max(sw,1e-8));
  float radius=.085f;
  for(int it=0;it<3;it++){
    sw=scb=scr=0; double sd=0; int kept=0;
    for(int i=0;i<n;i++){float db=c[i].cb-mcb,dr=c[i].cr-mcr,d=std::sqrt(db*db+dr*dr);if(d>radius)continue;float w=c[i].w*(1.f-d/std::max(radius,1e-6f));sw+=w;scb+=w*c[i].cb;scr+=w*c[i].cr;sd+=w*d*d;kept++;}
    if(kept<10||sw<1e-6)break; mcb=(float)(scb/sw);mcr=(float)(scr/sw);float sigma=std::sqrt((float)(sd/sw));radius=clampf(2.35f*sigma,.028f,.080f);
  }

  // Stage 3: require spatial coherence. A valid cell needs several samples near
  // the adaptive center and at least one neighboring valid cell. This rejects
  // isolated wood/clothing/highlight pixels that merely share skin chroma.
  std::array<float,NC> nearW{}; std::array<int,NC> nearN{};
  for(int i=0;i<n;i++){float db=c[i].cb-mcb,dr=c[i].cr-mcr;if(std::sqrt(db*db+dr*dr)<=radius){nearW[c[i].cell]+=c[i].w;nearN[c[i].cell]++;}}
  std::array<unsigned char,NC> good{};
  for(int cy=0;cy<CY;cy++)for(int cx=0;cx<CX;cx++){int k=cy*CX+cx;if(nearN[k]<3||nearW[k]<.35f)continue;bool neighbor=false;for(int yy=std::max(0,cy-1);yy<=std::min(CY-1,cy+1);yy++)for(int xx=std::max(0,cx-1);xx<=std::min(CX-1,cx+1);xx++){if(xx==cx&&yy==cy)continue;int j=yy*CX+xx;if(nearN[j]>=3&&nearW[j]>=.35f)neighbor=true;}if(neighbor)good[k]=1;}

  struct SkinSample { float3 lin; float w; }; std::array<SkinSample,MAXC> ss{}; int ns=0; double skinW=0;
  for(int i=0;i<n;i++){float db=c[i].cb-mcb,dr=c[i].cr-mcr,d=std::sqrt(db*db+dr*dr);if(d>radius||!good[c[i].cell])continue;float w=c[i].w*(1.f-d/std::max(radius,1e-6f));if(w<=0)continue;ss[ns++]={c[i].lin,w};skinW+=w;}
  supportOut=clampf((float)(skinW/18.0),0.f,1.f);
  if(ns<12||skinW<2.0){supportOut=improveOut=0.f;return fallback;}

  auto score=[&](float3 g){double e=0,w=0;for(int i=0;i<ns;i++){float3 v=make_float3(ss[i].lin.x*g.x,ss[i].lin.y*g.y,ss[i].lin.z*g.z);float3 h=ks_rgb_to_hsv(logc4_encode(v));float d=hueDist(h.x,keystone_skin_SKIN_HUE_CENTER);e+=ss[i].w*d*d;w+=ss[i].w;}return (float)(e/std::max(w,1e-8));};
  float base=score(make_float3(1,1,1)),best=1e9f;float3 bg=fallback;
  // Tighter search than v1.2: skin can steer WB, but cannot throw the entire
  // image toward purple/green. Neutral estimate remains a real safety prior.
  for(int ir=-12;ir<=12;ir++)for(int ib=-12;ib<=12;ib++){
    float3 g=make_float3(e2(ir/40.f),1.f,e2(ib/40.f));float gy=awg4_to_xyz(g).y;if(gy>1e-8f)g/=gy;g=clampGain(g);
    float skin=score(g),dist=gainDist(g,fallback);float total=skin+.11f*dist*dist;
    if(total<best){best=total;bg=g;}
  }
  float after=score(bg);improveOut=(base>1e-8f)?clampf((base-after)/base,0.f,1.f):0.f;
  // Weak/ambiguous clusters cannot dominate. Strong coherent clusters may steer
  // substantially, but the fallback neutral estimate always contributes.
  float a=clampf(.20f+.60f*supportOut,0.f,.80f);
  return make_float3(e2(l2(fallback.x)*(1-a)+l2(bg.x)*a),e2(l2(fallback.y)*(1-a)+l2(bg.y)*a),e2(l2(fallback.z)*(1-a)+l2(bg.z)*a));
}
NeutralResult analyzeNeutralRGBA(const float* src,int width,int height,int stride,const Params& params){
  NeutralResult out; if(!src||width<2||height<2||std::abs(stride)<width*4)return out;
  std::array<float3,N> samples{};std::array<float,N> ys{};float3 gw=make_float3(0,0,0),sog=gw,neu=gw;float neuW=0,wbN=0;float3 qsum[4]={gw,gw,gw,gw};float qn[4]={0,0,0,0};int n=0;
  for(int gy=0;gy<NY;gy++)for(int gx=0;gx<NX;gx++){
    int sx=(int)(((float)gx+.5f)*width/NX);int sy=(int)(((float)gy+.5f)*height/NY);sx=std::min(std::max(sx,0),width-1);sy=std::min(std::max(sy,0),height-1);
    const float* px=src+sy*stride+sx*4;float3 lin=preNeutral(make_float3(px[0],px[1],px[2]),params);float Y=awg4_to_xyz(lin).y;
    if(!(keystone_cpu::finitef(lin.x)&&keystone_cpu::finitef(lin.y)&&keystone_cpu::finitef(lin.z)&&keystone_cpu::finitef(Y))||Y<=1e-5f){samples[n]=make_float3(0,0,0);ys[n]=0;n++;continue;}
    samples[n]=lin;ys[n]=Y;n++;
    if(Y>.003f&&Y<1.25f&&lin.x>1e-6f&&lin.y>1e-6f&&lin.z>1e-6f){
      gw+=lin;wbN+=1;sog+=make_float3(std::pow(lin.x,P),std::pow(lin.y,P),std::pow(lin.z,P));float mx=max3(lin),mn=min3(lin);float chroma=(mx-mn)/std::max(mx,1e-6f);float nw=1.f-smootherstep(.08f,.48f,chroma);float midw=smootherstep(.006f,.03f,Y)*(1.f-smootherstep(.65f,1.25f,Y));nw*=midw;neu+=lin*nw;neuW+=nw;int q=(gx>=NX/2?1:0)+2*(gy>=NY/2?1:0);qsum[q]+=lin;qn[q]+=1;
    }
  }
  if(wbN<4)return out;
  float3 edgep=make_float3(0,0,0);float edgeN=0;
  for(int y=0;y<NY;y++)for(int x=0;x<NX;x++){int i=y*NX+x;if(ys[i]<=1e-5f)continue;if(x+1<NX){int j=i+1;if(ys[j]>1e-5f){float3 d=samples[j]-samples[i];d=make_float3(std::fabs(d.x),std::fabs(d.y),std::fabs(d.z));edgep+=make_float3(std::pow(d.x,P),std::pow(d.y,P),std::pow(d.z,P));edgeN++;}}if(y+1<NY){int j=i+NX;if(ys[j]>1e-5f){float3 d=samples[j]-samples[i];d=make_float3(std::fabs(d.x),std::fabs(d.y),std::fabs(d.z));edgep+=make_float3(std::pow(d.x,P),std::pow(d.y,P),std::pow(d.z,P));edgeN++;}}}
  float invp=1.f/P;float3 egw=gw/wbN,esog=make_float3(std::pow(sog.x/wbN,invp),std::pow(sog.y/wbN,invp),std::pow(sog.z/wbN,invp));float3 ggw=gainFromIllum(egw),gsog=gainFromIllum(esog),gedge=gsog;float edgeSupport=0;
  if(edgeN>=6){float3 ee=make_float3(std::pow(edgep.x/edgeN,invp),std::pow(edgep.y/edgeN,invp),std::pow(edgep.z/edgeN,invp));gedge=gainFromIllum(ee);edgeSupport=1;}
  float3 gneu=(neuW>.15f)?gainFromIllum(neu/neuW):make_float3(1,1,1);float neutralSupport=clampf(neuW/std::max(wbN*.35f,1e-6f),0.f,1.f);float agreeGS=1.f-smootherstep(.15f,.75f,gainDist(ggw,gsog));float agreeGE=edgeSupport?1.f-smootherstep(.20f,.90f,gainDist(gsog,gedge)):agreeGS;float estimatorAgree=.65f*agreeGS+.35f*agreeGE;
  float3 global=logMix(gsog,.45f,ggw,.25f,gedge,.20f*edgeSupport,gneu,.10f*neutralSupport);float maxQ=0;for(int q=0;q<4;q++)if(qn[q]>=2){float3 qg=gainFromIllum(qsum[q]/qn[q]);maxQ=std::max(maxQ,gainDist(qg,global));}float mixedConf=1.f-smootherstep(.18f,.80f,maxQ);
  float skinSupport=0.f,skinImprove=0.f;
  float3 skinFirst=skinDrivenGain(src,width,height,stride,params,global,skinSupport,skinImprove);
  float neutralGuard=.35f+.65f*smootherstep(.05f,.35f,neutralSupport);
  float baseConf=clampf((.25f+.45f*estimatorAgree+.30f*neutralSupport)*mixedConf*neutralGuard,0.f,1.f);
  // Credible skin raises confidence because it is now the primary chromatic reference.
  float conf=(skinSupport>.15f)?clampf(.55f+.35f*skinSupport+.10f*skinImprove,0.f,1.f):baseConf;
  float3 chosen=(skinSupport>.15f)?skinFirst:global;
  float3 lg=make_float3(l2(chosen.x),l2(chosen.y),l2(chosen.z));
  float applyConf=(skinSupport>.15f)?1.f:conf;
  float3 applied=make_float3(e2(lg.x*applyConf),e2(lg.y*applyConf),e2(lg.z*applyConf));
  out.gainR=applied.x;out.gainG=applied.y;out.gainB=applied.z;
  out.slopeR=out.slopeG=out.slopeB=1.f; out.offsetR=out.offsetG=out.offsetB=0.f;
  out.confidence=conf;out.valid=conf>0.01f;return out;
}
}
