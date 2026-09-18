#include "KeystoneCPU.h"
#include "KeystoneCoreCPU.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

namespace keystone {
namespace {
struct RegionContext { float cx=0.5f, cy=0.46f, sx=0.23f, sy=0.46f; bool subject=false; };

static inline float smooth01(float a,float b,float x){
  if(a==b)return x>=b?1.0f:0.0f;
  float t=std::max(0.0f,std::min(1.0f,(x-a)/(b-a)));
  return t*t*(3.0f-2.0f*t);
}
static inline float hueDist(float a,float b){float d=std::fabs(a-b);return std::min(d,1.0f-d);}
static inline float hueBand(float h,float center,float half){return 1.0f-smooth01(half*0.55f,half,hueDist(h,center));}
static inline keystone_cpu::float3 detectionLinear(keystone_cpu::float3 input){
  using namespace keystone_cpu;
  float3 lin=logc4_decode(input);
  float3 xyz=awg4_to_xyz(lin); xyz=input_gamut_heal_xyz(xyz); return xyz_to_awg4(xyz);
}
static inline float skinEvidence(keystone_cpu::float3 lin){
  using namespace keystone_cpu;
  return clampf(ks_tonelab_skin_mask_awg4(lin,0.0f,46.0f,0.50f,1.35f),0.0f,1.0f);
}
static RegionContext analyzeContext(const float* src,int w,int h,int ss){
  RegionContext c; double sw=0,sx=0,sy=0,sxx=0,syy=0;
  const int step=std::max(1,std::min(w,h)/220);
  for(int y=0;y<h;y+=step){const float* row=src+y*ss;float v=(y+0.5f)/(float)h;
    for(int x=0;x<w;x+=step){const float* px=row+x*4;auto lin=detectionLinear({px[0],px[1],px[2]});float m=skinEvidence(lin);if(m<0.08f)continue;float u=(x+0.5f)/(float)w;double q=m*m;sw+=q;sx+=q*u;sy+=q*v;sxx+=q*u*u;syy+=q*v*v;}}
  if(sw>3.0){
    c.subject=true;c.cx=(float)(sx/sw);float skinCy=(float)(sy/sw);
    float vx=std::max(0.0004f,(float)(sxx/sw-c.cx*c.cx));float vy=std::max(0.0004f,(float)(syy/sw-skinCy*skinCy));
    c.sx=std::max(0.18f,std::min(0.34f,0.12f+3.0f*std::sqrt(vx)));
    c.sy=std::max(0.38f,std::min(0.62f,0.30f+4.0f*std::sqrt(vy)));
    // Skin typically sits above the body centre in OFX bottom-up coordinates.
    c.cy=std::max(0.22f,std::min(0.70f,skinCy-0.16f));
  }
  return c;
}
static float subjectMask(keystone_cpu::float3 lin,float u,float v,const RegionContext& c,float feather){
  float skin=skinEvidence(lin);
  float dx=(u-c.cx)/std::max(c.sx,0.05f),dy=(v-c.cy)/std::max(c.sy,0.08f);
  float d2=dx*dx+dy*dy;
  float edge=1.0f-smooth01(std::max(0.35f,0.85f-feather*0.35f),1.35f+feather*0.75f,std::sqrt(d2));
  if(!c.subject)edge*=0.35f;
  return std::max(skin,edge);
}
static float regionMask(keystone_cpu::float3 input,float u,float v,const RegionContext& c,const Params& p){
  using namespace keystone_cpu;
  float3 lin=detectionLinear(input);float3 rgb=ks_awg4_to_tl_rec709(lin);float3 hsv=ks_rgb_to_hsv(rgb);
  float h=hsv.x,s=clampf(hsv.y,0.0f,1.0f),val=clampf(hsv.z,0.0f,1.2f),f=clampf(p.regionFeather,0.0f,1.0f);
  float subj=subjectMask(lin,u,v,c,f);
  float blue=hueBand(h,0.58f,0.16f)*smooth01(0.05f,0.28f,s);
  float cyan=hueBand(h,0.52f,0.13f)*smooth01(0.06f,0.24f,s);
  float green=hueBand(h,0.33f,0.17f)*smooth01(0.07f,0.26f,s);
  float brown=(hueBand(h,0.075f,0.10f)+0.65f*hueBand(h,0.13f,0.07f)); brown=std::min(1.0f,brown)*smooth01(0.06f,0.28f,s);
  float top=smooth01(0.48f-f*0.12f,0.78f+f*0.10f,v),bottom=1.0f-smooth01(0.22f-f*0.08f,0.58f+f*0.12f,v);
  float sky=top*std::max(blue, smooth01(0.70f,0.96f,val)*(1.0f-smooth01(0.10f,0.32f,s))*0.72f);
  float foliage=green*(0.72f+0.28f*(1.0f-top));
  float water=std::max(blue,cyan)*(1.0f-sky)*smooth01(0.10f,0.55f,1.0f-top+0.15f);
  float terrain=brown*(0.35f+0.65f*bottom)*(1.0f-foliage);
  float ground=bottom*(1.0f-0.80f*foliage)*(1.0f-0.70f*water)*(1.0f-0.55f*subj);
  float residual=clampf(1.0f-std::max({subj,sky,foliage,water,terrain}),0.0f,1.0f);
  float built=residual*(0.55f+0.45f*(1.0f-smooth01(0.18f,0.55f,s)))*(1.0f-0.45f*ground);
  float m=0.0f;
  switch(p.regionTarget){
    case 0:m=subj;break;
    case 1:m=1.0f-subj;break;
    case 2:m=sky;break;
    case 3:m=foliage;break;
    case 4:m=water;break;
    case 5:m=ground;break;
    case 6:m=terrain;break;
    case 7:m=built;break;
    default:m=0.0f;break;
  }
  if(p.regionTarget>=2)m*=1.0f-0.80f*subj;
  float lo=0.06f+0.12f*f,hi=0.72f-0.18f*f;if(hi<=lo)hi=lo+0.05f;
  return smooth01(lo,hi,clampf(m,0.0f,1.0f));
}
}

void processRGBA(const float* src,float* dst,int w,int h,int ss,int ds,const Params& p,const std::vector<LutEntry>& lut){
  if(!src||!dst||w<=0||h<=0||std::abs(ss)<w*4||std::abs(ds)<w*4||lut.size()!=35937)return;
  RegionContext ctx;if(p.regionEnable)ctx=analyzeContext(src,w,h,ss);
  for(int y=0;y<h;y++){
    const float* srow=src+y*ss;float* drow=dst+y*ds;float v=(y+0.5f)/(float)h;
    for(int x=0;x<w;x++){
      const float* px=srow+x*4;float* q=drow+x*4;float u=(x+0.5f)/(float)w;
      float m=p.regionEnable?regionMask({px[0],px[1],px[2]},u,v,ctx,p):0.0f;
      if(p.regionEnable&&p.regionShowMask){q[0]=q[1]=q[2]=m;q[3]=px[3];continue;}
      auto o=keystone_cpu::processPixel({px[0],px[1],px[2]},p,lut.data(),m);q[0]=o.x;q[1]=o.y;q[2]=o.z;q[3]=px[3];
    }
  }
}
}
