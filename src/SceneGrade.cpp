#include "SceneGrade.h"
#include "KeystoneCoreCPU.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <numeric>
#include <sstream>
#include <vector>
#ifdef __APPLE__
#include <dlfcn.h>
#endif

namespace keystone {
namespace {
using keystone_cpu::float3;
using keystone_cpu::make_float3;

struct Sample { float r=0,g=0,b=0,u=0,v=0; uint8_t band=0,skin=0,region=SceneOther; };
struct RegionStat { float cover=0,L=0,a=0,b=0; };
struct Choice { int subject=-1,mode=-1,sign=0,option=0,options=0; float cover=0,subjL=0,restL=0,subjB=0,restB=0; bool ok=false; };

static constexpr float kSalience[SceneRegionCount]={0.7f,1.2f,3.0f,1.0f,0.7f,0.6f,0.6f,0.4f};
static constexpr int kMinCover=6;
static constexpr float kDominant=94.0f;
static constexpr float kSkinDecisionMax=25.0f;
static constexpr float kColorUnit=6.0f;
static constexpr float kColorStep=0.05f;

static inline float clampf(float x,float a,float b){return std::max(a,std::min(b,x));}
static inline float signedPow(float x,float p){return x<0?-std::pow(-x,p):std::pow(x,p);}
static inline float displayLinear(float x){return signedPow(x,2.4f);}
static inline float labf(float t){const float d=6.0f/29.0f;return t>d*d*d?std::cbrt(t):(t/(3*d*d)+4.0f/29.0f);}
static void rgbToLab(float r,float g,float b,float& L,float& a,float& bb){
  const float R=displayLinear(r),G=displayLinear(g),B=displayLinear(b);
  const float X=0.4123908f*R+0.3575843f*G+0.1804808f*B;
  const float Y=0.2126390f*R+0.7151687f*G+0.0721923f*B;
  const float Z=0.0193308f*R+0.1191948f*G+0.9505322f*B;
  const float fx=labf(X/0.95047f),fy=labf(Y),fz=labf(Z/1.08883f);
  L=116*fy-16;a=500*(fx-fy);bb=200*(fy-fz);
}
static void rgb2hsv(float r,float g,float b,float& h,float& s,float& v){
  float mx=std::max(r,std::max(g,b)),mn=std::min(r,std::min(g,b)),d=mx-mn;v=mx;s=mx>1e-12f?d/mx:0.0f;
  if(d<=1e-12f){h=0;return;} if(mx==r)h=(g-b)/d+(g<b?6.0f:0.0f);else if(mx==g)h=(b-r)/d+2.0f;else h=(r-g)/d+4.0f;h/=6.0f;
}
static bool skinChroma(float r,float g,float b){float h,s,v;rgb2hsv(r,g,b,h,s,v);return h>=0.01f&&h<=0.11f&&s>=0.10f&&s<=0.65f&&v>=0.03f&&v<=1.05f;}

static const unsigned char kAdeToRegion[150]={
 SceneBuilt,SceneBuilt,SceneSky,SceneGround,SceneFoliage,SceneGround, SceneGround,SceneBuilt,SceneBuilt,SceneFoliage,SceneOther,SceneGround,
 SceneSkin,SceneTerrain,SceneBuilt,SceneBuilt,SceneTerrain,SceneFoliage, SceneBuilt,SceneBuilt,SceneBuilt,SceneWater,SceneOther,SceneBuilt,
 SceneBuilt,SceneBuilt,SceneWater,SceneOther,SceneOther,SceneFoliage, SceneOther,SceneBuilt,SceneBuilt,SceneOther,SceneTerrain,SceneBuilt,
 SceneOther,SceneOther,SceneBuilt,SceneBuilt,SceneOther,SceneBuilt, SceneBuilt,SceneBuilt,SceneOther,SceneOther,SceneTerrain,SceneOther,
 SceneBuilt,SceneOther,SceneOther,SceneOther,SceneTerrain,SceneGround, SceneGround,SceneOther,SceneWater,SceneOther,SceneBuilt,SceneGround,
 SceneWater,SceneGround,SceneBuilt,SceneOther,SceneBuilt,SceneOther, SceneFoliage,SceneOther,SceneTerrain,SceneOther,SceneOther,SceneOther,
 SceneFoliage,SceneOther,SceneOther,SceneBuilt,SceneOther,SceneOther, SceneOther,SceneOther,SceneBuilt,SceneOther,SceneOther,SceneBuilt,
 SceneBuilt,SceneOther,SceneOther,SceneBuilt,SceneOther,SceneOther, SceneOther,SceneTerrain,SceneOther,SceneBuilt,SceneTerrain,SceneOther,
 SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,SceneOther, SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,
 SceneOther,SceneWater,SceneOther,SceneOther,SceneOther,SceneWater, SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,
 SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,SceneOther, SceneOther,SceneOther,SceneWater,SceneOther,SceneBuilt,SceneOther,
 SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,SceneOther, SceneOther,SceneOther,SceneGround,SceneBuilt,SceneOther,SceneOther,
 SceneOther,SceneOther,SceneOther,SceneOther,SceneOther,SceneOther};

static std::string sceneModelDir(){
  if(const char* e=std::getenv("KEYSTONE_SCENE_MODEL"))return e;
#ifdef __APPLE__
  Dl_info info{}; if(dladdr((const void*)&sceneModelDir,&info)&&info.dli_fname){std::string p=info.dli_fname;auto s=p.find_last_of('/');if(s!=std::string::npos){p=p.substr(0,s);s=p.find_last_of('/');if(s!=std::string::npos)return p.substr(0,s)+"/Resources/SceneModel";}}
#endif
  return "resources/SceneModel";
}

// The semantic inference runtime is intentionally NOT linked into the main OFX.
// Resolve must be able to load Keystone even if the optional scene engine or model is missing.
// The sidecar is opened lazily only when Analyze Scene is pressed.
using SceneSegmentFn = int (*)(const unsigned char*, int, int, const char*, unsigned char*, int, int*, int*);

static std::string sceneEnginePath(){
#ifdef __APPLE__
  Dl_info info{};
  if(dladdr((const void*)&sceneEnginePath,&info)&&info.dli_fname){
    std::string p=info.dli_fname;
    auto s=p.find_last_of('/');
    if(s!=std::string::npos){
      p=p.substr(0,s); // .../Contents/MacOS
      s=p.find_last_of('/');
      if(s!=std::string::npos)return p.substr(0,s)+"/Resources/KeystoneSceneEngine.dylib";
    }
  }
#endif
  return "KeystoneSceneEngine.dylib";
}

static bool runSemanticSidecar(const unsigned char* rgb,int w,int h,
                               std::vector<unsigned char>& regions,int& ow,int& oh){
#ifdef __APPLE__
  struct LazyEngine {
    void* handle=nullptr;
    SceneSegmentFn fn=nullptr;
    bool tried=false;
    std::mutex mu;
  };
  static LazyEngine e;
  {
    std::lock_guard<std::mutex> lock(e.mu);
    if(!e.tried){
      e.tried=true;
      e.handle=dlopen(sceneEnginePath().c_str(),RTLD_LAZY|RTLD_LOCAL);
      if(e.handle)e.fn=reinterpret_cast<SceneSegmentFn>(dlsym(e.handle,"KeystoneSceneSegment"));
    }
  }
  if(!e.fn||!rgb||w<=0||h<=0)return false;
  // The sidecar caps its label map at 256x256. This buffer is deliberately fixed so
  // a malformed sidecar cannot ask the host plugin to allocate an arbitrary size.
  regions.assign(256u*256u,(unsigned char)SceneOther);
  int rw=0,rh=0;
  const std::string md=sceneModelDir();
  const int ok=e.fn(rgb,w,h,md.c_str(),regions.data(),(int)regions.size(),&rw,&rh);
  if(!ok||rw<=0||rh<=0||(size_t)rw*(size_t)rh>regions.size()){
    regions.clear();ow=oh=0;return false;
  }
  ow=rw;oh=rh;regions.resize((size_t)ow*(size_t)oh);return true;
#else
  (void)rgb;(void)w;(void)h;(void)regions;(void)ow;(void)oh;
  return false;
#endif
}

static Params analysisParams(const Params& in){Params p=in;p.neutralAmount=0.0f;p.neutralGainR=p.neutralGainG=p.neutralGainB=1.0f;p.matchSlopeR=p.matchSlopeG=p.matchSlopeB=1.0f;p.matchOffsetR=p.matchOffsetG=p.matchOffsetB=0.0f;p.sceneGainTemp=p.sceneOffsetTemp=p.sceneExposure=p.sceneShadows=p.sceneHighlights=0.0f;p.skinShowMask=0;return p;}
static float3 render(const Sample& s,const Params& p,const std::vector<LutEntry>& lut){return keystone_cpu::processPixel(make_float3(s.r,s.g,s.b),p,lut.data());}
static float lum(float3 c){return 0.2126f*c.x+0.7152f*c.y+0.0722f*c.z;}
static float pct(std::vector<float> v,double q){if(v.empty())return 0;size_t k=(size_t)(q*(v.size()-1));std::nth_element(v.begin(),v.begin()+k,v.end());return v[k];}

static void regionStats(const std::vector<Sample>& s,const Params& p,const std::vector<LutEntry>& lut,RegionStat st[SceneRegionCount]){
  for(int i=0;i<SceneRegionCount;++i)st[i]=RegionStat();std::array<double,SceneRegionCount> L{},A{},B{};std::array<int,SceneRegionCount> N{};
  for(const auto& q:s){int r=q.region;if(r<0||r>=SceneRegionCount)continue;float3 d=render(q,p,lut);float l,a,b;rgbToLab(d.x,d.y,d.z,l,a,b);L[r]+=l;A[r]+=a;B[r]+=b;N[r]++;}
  for(int r=0;r<SceneRegionCount;++r)if(N[r]){st[r].cover=100.0f*(float)N[r]/(float)s.size();st[r].L=(float)(L[r]/N[r]);st[r].a=(float)(A[r]/N[r]);st[r].b=(float)(B[r]/N[r]);}
}
static bool protectedRegion(int r){return r==SceneSkin;}
static bool sceneEligible(const RegionStat* st){int n=0;float biggest=0;for(int r=0;r<SceneRegionCount;++r)if(st[r].cover>=kMinCover){++n;biggest=std::max(biggest,st[r].cover);}return n>=2&&biggest<kDominant;}
static Choice choiceForSubject(const RegionStat* st,int s){Choice c;if(!sceneEligible(st)||s<0||s>=SceneRegionCount||st[s].cover<kMinCover)return c;double ws=0,wL=0,wB=0;for(int r=0;r<SceneRegionCount;++r){if(r==s||st[r].cover<kMinCover)continue;double w=st[r].cover;ws+=w;wL+=w*st[r].L;wB+=w*st[r].b;}if(ws<=0)return c;float restL=(float)(wL/ws),restB=(float)(wB/ws),db=st[s].b-restB;if(std::fabs(db)<0.5f)db=std::fabs(restB)>0.5f?-restB:1.0f;c.subject=s;c.cover=st[s].cover;c.subjL=st[s].L;c.restL=restL;c.subjB=st[s].b;c.restB=restB;if(protectedRegion(s)){c.mode=(restL>st[s].L)?0:1;c.sign=(db>0)?-1:+1;}else{c.mode=(st[s].L>restL)?0:1;c.sign=(db>0)?+1:-1;}c.ok=true;return c;}
static Choice choosePrimary(const RegionStat* st,int requested){if(requested>=0)return choiceForSubject(st,requested);Choice out;if(!sceneEligible(st))return out;std::vector<int> idx;for(int r=0;r<SceneRegionCount;++r)if(st[r].cover>=kMinCover)idx.push_back(r);std::sort(idx.begin(),idx.end(),[&](int A,int B){bool fa=A==SceneSkin&&st[A].cover<=kSkinDecisionMax,fb=B==SceneSkin&&st[B].cover<=kSkinDecisionMax;if(fa!=fb)return fa;return st[A].cover*kSalience[A]>st[B].cover*kSalience[B];});std::vector<Choice> cands;for(int s:idx){Choice c=choiceForSubject(st,s);if(!c.ok)continue;bool dup=false;for(auto& q:cands)dup|=(q.mode==c.mode&&q.sign==c.sign);if(!dup)cands.push_back(c);}if(cands.empty())return out;out=cands[0];out.option=0;out.options=(int)cands.size();return out;}
static float regionB(const std::vector<Sample>& s,const Params& p,const std::vector<LutEntry>& lut,int region){double sum=0;int n=0;size_t step=std::max<size_t>(1,s.size()/8000);for(size_t i=0;i<s.size();i+=step)if(s[i].region==region){float3 d=render(s[i],p,lut);float L,a,b;rgbToLab(d.x,d.y,d.z,L,a,b);sum+=b;n++;}return n?(float)(sum/n):0;}
static float solveColorBase(const std::vector<Sample>& s,const RegionStat* st,const Choice& c,const Params& base,const std::vector<LutEntry>& lut){if(!c.ok)return 0;int measured=c.subject;if(protectedRegion(c.subject)){float best=-1;for(int r=0;r<SceneRegionCount;++r)if(r!=c.subject&&st[r].cover>best){best=st[r].cover;measured=r;}}Params p0=base,p1=base;if(c.mode==0)p1.sceneGainTemp+=kColorStep;else p1.sceneOffsetTemp+=kColorStep;float grip=regionB(s,p1,lut,measured)-regionB(s,p0,lut,measured);if(std::fabs(grip)<=1e-4f)return 0;return clampf(c.sign*kColorUnit*kColorStep/std::fabs(grip),-0.35f,0.35f);}

struct ToneMetrics{float lo=0,mid=0,top=0;bool ok=false;};
static ToneMetrics toneMetrics(const std::vector<Sample>& s,const Params& p,const std::vector<LutEntry>& lut,int subject){std::vector<float> sv,frame;sv.reserve(s.size()/4);frame.reserve(s.size());for(const auto& q:s){float3 d=render(q,p,lut);float y=lum(d);frame.push_back(std::max(d.x,std::max(d.y,d.z)));if(q.region==subject)sv.push_back(y);}ToneMetrics m;if(sv.size()<64||frame.empty())return m;m.lo=pct(sv,.10);m.mid=pct(sv,.50);m.top=pct(frame,.999);m.ok=true;return m;}
template<class F> static float gridSolve(float lo,float hi,float start,F metric,float target){float best=start,bestE=1e9f;float a=lo,b=hi;for(int pass=0;pass<3;++pass){for(int i=0;i<=20;++i){float x=a+(b-a)*(float)i/20.0f;float e=std::fabs(metric(x)-target);if(e<bestE){bestE=e;best=x;}}float span=(b-a)/10.0f;a=std::max(lo,best-span);b=std::min(hi,best+span);}return best;}
static void solveTone(const std::vector<Sample>& s,const Choice& c,const Params& base,const std::vector<LutEntry>& lut,float bias,float& e,float& sh,float& hi){e=sh=hi=0;if(!c.ok)return;float floorT=0,midT=0,maxCover=0;if(c.subject==SceneSkin){floorT=0.125f;midT=0.278f;maxCover=60.0f;}else if(c.subject==SceneSky){floorT=0.475f;midT=0.602f;maxCover=90.0f;}else return;if(c.cover>maxCover)return;floorT=clampf(floorT+bias*0.06f,0.0f,0.40f);float ceilT=clampf(0.968f-bias*0.03f,0.60f,0.990f);Params p=base;p.sceneGainTemp=base.sceneGainTemp;p.sceneOffsetTemp=base.sceneOffsetTemp;
  e=gridSolve(-3.0f,3.0f,0.0f,[&](float x){Params q=p;q.sceneExposure=x;auto m=toneMetrics(s,q,lut,c.subject);return m.ok?m.mid:midT;},midT);p.sceneExposure=e;
  sh=gridSolve(-1.0f,1.0f,0.0f,[&](float x){Params q=p;q.sceneShadows=x;auto m=toneMetrics(s,q,lut,c.subject);return m.ok?m.lo:floorT;},floorT);p.sceneShadows=sh;
  hi=gridSolve(-1.0f,1.0f,0.0f,[&](float x){Params q=p;q.sceneHighlights=x;auto m=toneMetrics(s,q,lut,c.subject);return m.ok?m.top:ceilT;},ceilT);
}

static bool buildSamples(const float* rgba,int w,int h,int stride,const Params& p,const std::vector<LutEntry>& lut,std::vector<Sample>& samples,std::vector<unsigned char>& thumb,bool& modelReady){if(!rgba||w<=0||h<=0||stride<w*4||lut.size()!=35937)return false;const int T=512;thumb.assign((size_t)T*T*3,0);for(int ty=0;ty<T;++ty){int sy=h-1-(int)((long long)ty*h/T);sy=std::max(0,std::min(h-1,sy));for(int tx=0;tx<T;++tx){int sx=(int)((long long)tx*w/T);sx=std::max(0,std::min(w-1,sx));const float* q=rgba+(size_t)sy*stride+sx*4;Sample z;z.r=q[0];z.g=q[1];z.b=q[2];float3 d=render(z,p,lut);size_t o=((size_t)ty*T+tx)*3;thumb[o]=(unsigned char)std::lround(clampf(d.x,0,1)*255);thumb[o+1]=(unsigned char)std::lround(clampf(d.y,0,1)*255);thumb[o+2]=(unsigned char)std::lround(clampf(d.z,0,1)*255);}}
  std::vector<unsigned char> mask;int mw=0,mh=0;modelReady=false;
  modelReady=runSemanticSidecar(thumb.data(),T,T,mask,mw,mh);
  int step=std::max(1,(int)std::sqrt((double)w*h/25000.0));samples.clear();samples.reserve((size_t)((w+step-1)/step)*((h+step-1)/step));double meanL=0;std::vector<float> ls;
  for(int y=0;y<h;y+=step)for(int x=0;x<w;x+=step){const float* q=rgba+(size_t)y*stride+x*4;Sample z;z.r=q[0];z.g=q[1];z.b=q[2];z.u=(float)x/(float)w;z.v=(float)y/(float)h;z.band=(uint8_t)std::min(2,std::max(0,(int)((long long)y*3/h)));float3 d=render(z,p,lut);float L,a,b;rgbToLab(d.x,d.y,d.z,L,a,b);ls.push_back(L);meanL+=L;z.skin=skinChroma(d.x,d.y,d.z)?1:0;if(modelReady){int mx=std::min(mw-1,std::max(0,(int)(z.u*mw)));int my=std::min(mh-1,std::max(0,(int)((1.0f-z.v)*mh)));z.region=mask[(size_t)my*mw+mx];if(z.region==SceneSkin&&!z.skin)z.region=SceneOther;}samples.push_back(z);}if(samples.empty())return false;meanL/=samples.size();if(!modelReady){for(size_t i=0;i<samples.size();++i){Sample& z=samples[i];float3 d=render(z,p,lut);float L,a,b;rgbToLab(d.x,d.y,d.z,L,a,b);if(z.skin)z.region=SceneSkin;else if(z.band==2&&L>meanL)z.region=SceneSky;else if(b<-6&&L<meanL)z.region=SceneWater;else if(a<-5&&b>0)z.region=SceneFoliage;else if(L<meanL)z.region=SceneTerrain;else z.region=SceneBuilt;}}return true;}
}

const char* sceneRegionName(int r){static const char* n[SceneRegionCount]={"Sky","Water","Skin","Foliage","Terrain","Ground","Built","Other"};return r>=0&&r<SceneRegionCount?n[r]:"None";}

bool analyzeSceneGrade(const float* rgba,int width,int height,int strideFloats,const Params& current,const std::vector<LutEntry>& lut,int requestedRegion,float separation,float bias,SceneGradeResult& out){out=SceneGradeResult();Params base=analysisParams(current);std::vector<Sample> s;std::vector<unsigned char> thumb;bool model=false;if(!buildSamples(rgba,width,height,strideFloats,base,lut,s,thumb,model)){out.note="Could not analyze frame";return false;}RegionStat st[SceneRegionCount];regionStats(s,base,lut,st);for(int r=0;r<SceneRegionCount;++r)if(st[r].cover>=kMinCover)out.regionMask|=(1u<<r);Choice c=choosePrimary(st,requestedRegion);out.modelReady=model;if(!c.ok){out.note=model?"No separable subject found":"No separable subject found (heuristic regions)";return false;}float cb=solveColorBase(s,st,c,base,lut);float sep=clampf(separation,0,2);if(c.mode==0)base.sceneGainTemp=clampf(cb*sep,-1,1);else base.sceneOffsetTemp=clampf(cb*sep,-1,1);float ex=0,sh=0,hi=0;solveTone(s,c,base,lut,clampf(bias,-2,2),ex,sh,hi);out.ok=true;out.subject=c.subject;out.option=c.option;out.options=c.options;out.colorMode=c.mode;out.colorSign=c.sign;out.subjectCover=c.cover;out.subjectL=c.subjL;out.surroundL=c.restL;out.subjectB=c.subjB;out.surroundB=c.restB;out.colorBase=cb;out.sceneGainTemp=base.sceneGainTemp;out.sceneOffsetTemp=base.sceneOffsetTemp;out.sceneExposure=ex;out.sceneShadows=sh;out.sceneHighlights=hi;out.confidence=clampf((model?0.65f:0.40f)+std::min(c.cover,25.0f)/100.0f,0,0.95f);std::ostringstream ss;ss<<sceneRegionName(c.subject)<<" "<<(int)std::lround(c.cover)<<"% -> "<<(c.mode==0?"Gain Temp":"Offset Temp")<<(cb>=0?" +":" ")<<cb<<(model?" [model]":" [heuristic]");out.note=ss.str();return true;}

} // namespace keystone
