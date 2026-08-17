#include <cmath>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <limits>

struct float2 { float x,y; };
struct float3 { float x,y,z; };
static inline float2 make_float2(float x,float y){return{x,y};}
static inline float3 make_float3(float x,float y,float z){return{x,y,z};}
static inline float3 operator+(float3 a,float3 b){return{a.x+b.x,a.y+b.y,a.z+b.z};}
static inline float3 operator-(float3 a,float3 b){return{a.x-b.x,a.y-b.y,a.z-b.z};}
static inline float3 operator*(float3 a,float3 b){return{a.x*b.x,a.y*b.y,a.z*b.z};}
static inline float3 operator*(float3 a,float b){return{a.x*b,a.y*b,a.z*b};}
static inline float3 operator*(float b,float3 a){return a*b;}
static inline float3 operator/(float3 a,float b){return{a.x/b,a.y/b,a.z/b};}
static inline float3& operator*=(float3 &a,float3 b){a.x*=b.x;a.y*=b.y;a.z*=b.z;return a;}

#define __DEVICE__ static inline
#define _fmaxf std::fmax
#define _fminf std::fmin
#define _fabs std::fabs
#define _powf std::pow
#define _expf std::exp
#define _sqrtf std::sqrt
#define _log10f std::log10
#define _log2f std::log2
#define _tanhf std::tanh
#define _fmod std::fmod
#define _sinf std::sin
#define _cosf std::cos
#define _acosf std::acos
#define _atan2f std::atan2
#define DCTLUI_COMBO_BOX 0
#define DCTLUI_SLIDER_FLOAT 1
#define DEFINE_UI_PARAMS(name,label,type,defaultv,...) float name=(float)(defaultv);

#include "Keystone_under_test.dctl"

static bool finite3(float3 a){return std::isfinite(a.x)&&std::isfinite(a.y)&&std::isfinite(a.z);}
static float maxabs3(float3 a){return std::max({std::fabs(a.x),std::fabs(a.y),std::fabs(a.z)});}
static float min3(float3 a){return std::min(a.x,std::min(a.y,a.z));}
static void defaults(){
  input_gamut_heal=0;logc3_ei=7;temp=tint=0;balance_red=balance_green=balance_blue=0;
  fns_mode=0;fns_mid_in=.18f;fns_mid_out=.42f;fns_contrast_below=1.0f;fns_contrast_above=1.0f;fns_blend_width=0.0f;
  fns_printer_r=fns_printer_g=fns_printer_b=25.0f;
  p_exposure=0;p_bp=0;p_contrast=1;p_pivot_offset=0;p_shadows=0;p_highlights=0;p_high_soft=0;
  chroma=1;vibrance=0;midtone_chroma=0;hue_rotate=0;
  warm_target=0;warm_hue_shift=0;warm_chroma=1;warm_exposure=0;warm_evenness=0;
  global_sat=r_sat=g_sat=b_sat=c_sat=m_sat=y_sat=0;
  native_negative_compress=black_gamut_clean=0;output_skin_protect=1;
}

int main(){
  int fail=0;

  // Gamut matrix inverse roundtrip.
  for(int g=0;g<=12;g++){
    float maxe=0.0f;
    std::vector<float3> v={{1,0,0},{0,1,0},{0,0,1},{0.18f,0.18f,0.18f},{1.5f,-0.2f,0.7f}};
    for(auto x:v){float3 y=x2g(g,g2x(g,x));maxe=std::max(maxe,maxabs3(y-x));}
    if(maxe>2.0e-6f){std::printf("FAIL matrix g=%d err=%g\n",g,maxe);fail++;}
  }

  // Transfer encode/decode roundtrip. Stay inside Apple Log's documented encodable negative floor.
  std::vector<float> xs={-0.02f,-0.001f,0.0f,0.001f,0.005f,0.01f,0.018f,0.18f,1.0f,4.0f,16.0f,64.0f};
  for(int t=0;t<=17;t++){
    if(t==2)continue;
    float maxe=0.0f;int bad=0;
    for(float x:xs){
      float3 v=make_float3(x,x,x);float3 enc=tfe(v,t);float3 dec=tfd(enc,t);
      if(!finite3(enc)||!finite3(dec)){bad++;continue;}
      maxe=std::max(maxe,std::fabs(dec.x-x));
    }
    if(bad||maxe>2.0e-4f){std::printf("FAIL transfer t=%d bad=%d err=%g\n",t,bad,maxe);fail++;}
  }
  for(int i=0;i<=10;i++){
    float maxe=0.0f;int bad=0;
    for(float x:xs){float3 v=make_float3(x,x,x);float3 enc=tfl3e_ei(v,i);float3 dec=tfl3d_ei(enc,i);if(!finite3(enc)||!finite3(dec)){bad++;continue;}maxe=std::max(maxe,std::fabs(dec.x-x));}
    if(bad||maxe>2.0e-4f){std::printf("FAIL LogC3 EI=%d bad=%d err=%g\n",i,bad,maxe);fail++;}
  }

  // Exact neutral/bypass across every input space and LogC3 EI selector.
  std::vector<float3> codes={{0,0,0},{0.18f,0.2f,0.22f},{0.4f,0.5f,0.6f},{1.0f,0.1f,-0.1f},{1.5f,-0.2f,2.0f}};
  for(int sp=0;sp<=17;sp++){
    defaults();input_space=(float)sp;
    for(int ei=0;ei<=10;ei++){
      logc3_ei=(float)ei;
      for(auto c:codes){auto o=transform(1,1,0,0,c.x,c.y,c.z);if(o.x!=c.x||o.y!=c.y||o.z!=c.z){std::printf("FAIL neutral sp=%d ei=%d\n",sp,ei);fail++;goto neutral_done;}}
    }
    neutral_done: ;
  }

  // RC25 off-path golden vectors: Film Negative Space Off must not alter established Keystone behavior.
  {
    float3 expected[18]={
      {0.646549463f,0.291123062f,0.701002359f},{0.509360373f,0.113423586f,0.647818863f},
      {0.467079520f,0.168804526f,0.608288407f},{0.479372203f,0.183565348f,0.615033746f},
      {0.490202695f,0.184335262f,0.639424741f},{0.459418565f,0.258158833f,0.620705962f},
      {0.459662974f,0.230263934f,0.622728288f},{0.376095474f,-0.792718470f,0.455745518f},
      {0.446129322f,0.252352238f,0.616431713f},{0.475773156f,0.192196012f,0.575091660f},
      {0.477190882f,0.185310960f,0.617368579f},{0.490490317f,0.131851256f,0.639486134f},
      {0.718812227f,0.444871396f,0.706397414f},{0.430745095f,-0.758403301f,0.568104625f},
      {0.729022384f,0.452534914f,0.694205523f},{0.470555902f,0.206555709f,0.612699747f},
      {0.481096268f,-0.0737913847f,0.617626369f},{0.531215310f,0.222337291f,0.644373059f}
    };
    for(int sp=0;sp<18;sp++){
      defaults();input_space=(float)sp;fns_mode=0;temp=12.3f;tint=-4.5f;balance_red=.08f;balance_green=-.03f;balance_blue=.05f;
      p_exposure=.7f;p_bp=-.01f;p_contrast=1.12f;p_pivot_offset=.03f;p_shadows=.2f;p_highlights=-.15f;p_high_soft=.3f;
      chroma=1.08f;vibrance=.15f;midtone_chroma=.2f;hue_rotate=12.0f;warm_target=3.0f;warm_hue_shift=-2.0f;warm_chroma=1.05f;warm_exposure=.1f;warm_evenness=.25f;
      global_sat=-.08f;r_sat=-.03f;g_sat=-.02f;b_sat=-.04f;c_sat=-.01f;m_sat=-.02f;y_sat=-.03f;native_negative_compress=.2f;black_gamut_clean=.1f;output_skin_protect=.8f;
      auto o=transform(1,1,0,0,.42f,.28f,.61f);
      if(maxabs3(o-expected[sp])>2.0e-6f){std::printf("FAIL RC25 golden sp=%d err=%g\n",sp,maxabs3(o-expected[sp]));fail++;}
    }
  }

  // Film Negative Space helper: forward/inverse roundtrip, monotonicity, and negative preservation.
  {
    std::vector<float> vals={-0.25f,-0.02f,0.0f,0.001f,0.01f,0.05f,0.18f,0.42f,1.0f,4.0f,16.0f,64.0f};
    struct FP{float mi,mo,cb,ca,bw,pr,pg,pb;};
    std::vector<FP> fps={{.18f,.42f,1,1,0,25,25,25},{.18f,.42f,.75f,.9f,0,28,23,25},{.18f,.32f,1.8f,.55f,.25f,10,40,25},{.12f,.6f,.2f,2.7f,.5f,50,0,37}};
    for(auto q:fps){
      float prev=-1.0e30f;
      for(int i=0;i<=20000;i++){
        float x=-.1f+8.1f*(float)i/20000.0f;
        float y=fns_forward_characteristic(x,q.mi,q.mo,q.cb,q.ca,q.bw);
        if(!std::isfinite(y)||y+2.0e-5f<prev){std::printf("FAIL FNS monotonic bw=%g x=%g y=%g prev=%g\n",q.bw,x,y,prev);fail++;break;}
        prev=y;
      }
      for(float x:vals){
        float3 v={x,x*.73f,x*1.21f};
        float3 y=fns_forward_rgb(v,q.mi,q.mo,q.cb,q.ca,q.bw,q.pr,q.pg,q.pb);
        float3 z=fns_inverse_rgb(y,q.mi,q.mo,q.cb,q.ca,q.bw,q.pr,q.pg,q.pb);
        float tol=q.bw>0?2.5e-4f:3.0e-5f;
        if(!finite3(y)||!finite3(z)||maxabs3(z-v)>tol*std::max(1.0f,maxabs3(v))){
          std::printf("FAIL FNS roundtrip bw=%g x=%g err=%g\n",q.bw,x,maxabs3(z-v));fail++;break;
        }
      }
    }
  }

  // Enabling the internal sandwich alone must remain exact neutral, independent of FNS settings.
  for(int sp=0;sp<=17;sp++){
    defaults();input_space=(float)sp;fns_mode=1;fns_mid_in=.12f;fns_mid_out=.65f;fns_contrast_below=.35f;fns_contrast_above=2.4f;fns_blend_width=.4f;fns_printer_r=4;fns_printer_g=43;fns_printer_b=31;
    for(auto c:codes){auto o=transform(1,1,0,0,c.x,c.y,c.z);if(o.x!=c.x||o.y!=c.y||o.z!=c.z){std::printf("FAIL FNS neutral sp=%d\n",sp);fail++;break;}}
  }

  // The internal negative space must materially change how a legal creative tone move responds.
  {
    defaults();input_space=2;p_highlights=.7f;float3 in={.55f,.42f,.30f};auto plain=transform(1,1,0,0,in.x,in.y,in.z);
    defaults();input_space=2;p_highlights=.7f;fns_mode=1;fns_mid_in=.18f;fns_mid_out=.42f;fns_contrast_below=.85f;fns_contrast_above=.7f;auto neg=transform(1,1,0,0,in.x,in.y,in.z);
    if(maxabs3(neg-plain)<1.0e-4f){std::printf("FAIL FNS response equivalence: internal sandwich had no material effect\n");fail++;}
  }

  // RC26 highlight helper: monotonic and continuous through x=1 for the full slider range.
  for(float hv : {-1.0f,-0.5f,0.5f,1.0f}){
    float gain=std::pow(2.0f,hv),mid=0.435f,prev=-1.0e30f;
    for(int i=0;i<=24000;i++){
      float x=mid+(1.5f-mid)*(float)i/24000.0f;
      float y=keystone_highlight_gain_monotonic(x,gain,mid);
      if(y+1.0e-6f<prev){std::printf("FAIL highlight helper hv=%g x=%g\n",hv,x);fail++;break;}
      prev=y;
    }
    float jump=std::fabs(keystone_highlight_gain_monotonic(1.000001f,gain,mid)-keystone_highlight_gain_monotonic(0.999999f,gain,mid));
    if(jump>1.0e-4f){std::printf("FAIL highlight continuity hv=%g jump=%g\n",hv,jump);fail++;}
  }

  // Full-transform upper-range monotonicity in every supported input space.
  for(int sp=0;sp<=17;sp++)for(float hv : {-1.0f,1.0f}){
    defaults();input_space=(float)sp;p_highlights=hv;float prev=-1.0e30f;
    for(int i=0;i<=6000;i++){
      float x=0.2f+1.0f*(float)i/6000.0f;auto o=transform(1,1,0,0,x,x,x);
      if(!finite3(o)||o.x+2.0e-5f<prev){std::printf("FAIL highlight full sp=%d hv=%g x=%g out=%g prev=%g\n",sp,hv,x,o.x,prev);fail++;break;}
      prev=o.x;
    }
  }

  // Known high-risk legal color-control combinations must not escape below the encoded safety floor.
  struct Case{float chroma,vib,midc,hue;};
  std::vector<Case> cases={{2,0,0,0},{1,0,0,180},{1,0,0,-180},{1,0,1,0},{2,1,1,180}};
  std::vector<float3> inputs={{.8f,.1f,.1f},{.1f,.8f,.1f},{.1f,.1f,.8f},{1.0f,.02f,.3f},{.02f,1.0f,.8f}};
  for(int sp=0;sp<=17;sp++)for(auto c:cases)for(auto in:inputs){
    defaults();input_space=(float)sp;chroma=c.chroma;vibrance=c.vib;midtone_chroma=c.midc;hue_rotate=c.hue;
    auto o=transform(1,1,0,0,in.x,in.y,in.z);
    if(!finite3(o)||min3(o)<-1.0005f){std::printf("FAIL encoded safety sp=%d out=%g,%g,%g\n",sp,o.x,o.y,o.z);fail++;goto encoded_done;}
  }
  encoded_done: ;

  // Randomized stress with the DCTL's last-resort finite guard disabled by the Python runner.
  unsigned long long state=0x123456789abcdefULL;
  auto rnd=[&](){state^=state<<7;state^=state>>9;state^=state<<8;return(float)((state>>11)&0xFFFFFF)/(float)0xFFFFFF;};
  auto rr=[&](float a,float b){return a+(b-a)*rnd();};
  const int N=150000;
  for(int sp=0;sp<=17;sp++){
    input_space=(float)sp;
    for(int n=0;n<N;n++){
      logc3_ei=(float)(int)(rnd()*11.0f);if(logc3_ei>10)logc3_ei=10;
      input_gamut_heal=(rnd()<0.15f)?1.0f:0.0f;temp=rr(-100,100);tint=rr(-100,100);
      fns_mode=(rnd()<0.5f)?1.0f:0.0f;fns_mid_in=rr(.01f,1.0f);fns_mid_out=rr(.01f,1.0f);
      fns_contrast_below=rr(.1f,3.0f);fns_contrast_above=rr(.1f,3.0f);fns_blend_width=0.0f;
      fns_printer_r=rr(0,50);fns_printer_g=rr(0,50);fns_printer_b=rr(0,50);
      balance_red=rr(-.5,.5);balance_green=rr(-.5,.5);balance_blue=rr(-.5,.5);
      p_exposure=rr(-6,6);p_bp=rr(-.05,.05);p_contrast=rr(.5,2);p_pivot_offset=rr(-.2,.2);p_shadows=rr(-1,1);p_highlights=rr(-1,1);p_high_soft=rr(0,2);
      chroma=rr(0,2);vibrance=rr(-1,1);midtone_chroma=rr(0,1);hue_rotate=rr(-180,180);
      warm_target=rr(-25,25);warm_hue_shift=rr(-25,25);warm_chroma=rr(.5,1.5);warm_exposure=rr(-.5,.5);warm_evenness=rr(0,1);
      global_sat=rr(-1,0);r_sat=rr(-1,0);g_sat=rr(-1,0);b_sat=rr(-1,0);c_sat=rr(-1,0);m_sat=rr(-1,0);y_sat=rr(-1,0);
      native_negative_compress=rr(0,1);black_gamut_clean=rr(0,1);output_skin_protect=rr(0,1);
      float3 in={rr(-.25f,1.5f),rr(-.25f,1.5f),rr(-.25f,1.5f)};float3 o=transform(1,1,0,0,in.x,in.y,in.z);
      if(!finite3(o)||maxabs3(o)>1.0e4f||min3(o)<-1.0005f){std::printf("FAIL stress sp=%d n=%d in=%g,%g,%g out=%g,%g,%g fns=%g mi=%g mo=%g cb=%g ca=%g pr=%g pg=%g pb=%g exp=%g bp=%g con=%g piv=%g sh=%g hi=%g roll=%g chroma=%g vib=%g midc=%g hue=%g\n",sp,n,in.x,in.y,in.z,o.x,o.y,o.z,fns_mode,fns_mid_in,fns_mid_out,fns_contrast_below,fns_contrast_above,fns_printer_r,fns_printer_g,fns_printer_b,p_exposure,p_bp,p_contrast,p_pivot_offset,p_shadows,p_highlights,p_high_soft,chroma,vibrance,midtone_chroma,hue_rotate);fail++;goto stress_done;}
    }
  }
  stress_done: ;

  // Focused blended-negative stress (Blend triggers iterative inverse and is tested separately for speed).
  {
    const int BN=2500;
    for(int sp=0;sp<=17;sp++)for(int n=0;n<BN;n++){
      defaults();input_space=(float)sp;fns_mode=1;fns_mid_in=rr(.05f,.5f);fns_mid_out=rr(.08f,.8f);
      fns_contrast_below=rr(.2f,2.5f);fns_contrast_above=rr(.2f,2.5f);fns_blend_width=rr(.01f,.5f);
      fns_printer_r=rr(8,42);fns_printer_g=rr(8,42);fns_printer_b=rr(8,42);
      p_exposure=rr(-3,3);p_contrast=rr(.7f,1.5f);p_shadows=rr(-.7f,.7f);p_highlights=rr(-.7f,.7f);p_high_soft=rr(0,1.2f);
      chroma=rr(.5f,1.5f);vibrance=rr(-.6f,.6f);hue_rotate=rr(-90,90);
      float3 in={rr(-.1f,1.2f),rr(-.1f,1.2f),rr(-.1f,1.2f)};float3 o=transform(1,1,0,0,in.x,in.y,in.z);
      if(!finite3(o)||maxabs3(o)>1.0e4f||min3(o)<-1.0005f){std::printf("FAIL FNS blend stress sp=%d n=%d out=%g,%g,%g\n",sp,n,o.x,o.y,o.z);fail++;goto blend_done;}
    }
  }
  blend_done: ;

  if(fail){std::printf("BEHAVIORAL VALIDATION FAILED (%d)\n",fail);return 1;}
  std::printf("BEHAVIORAL VALIDATION PASSED\n");
  std::printf("Random stress evaluations: %d (+ %d blended FNS)\n",N*18,2500*18);
  return 0;
}
