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
  fns_mid_out=.42f;fns_contrast_below=1.0f;fns_contrast_above=1.0f;fns_printer_r=fns_printer_g=fns_printer_b=25.0f;
  p_exposure=0;p_bp=0;p_contrast=1;p_pivot_offset=0;p_shadows=0;p_highlights=0;p_high_soft=0;
  chroma=1;vibrance=0;hue_rotate=0;
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

  // Transfer encode/decode roundtrip.
  std::vector<float> xs={-0.02f,-0.001f,0.0f,0.001f,0.005f,0.01f,0.018f,0.18f,1.0f,4.0f,16.0f,64.0f};
  for(int t=0;t<=17;t++){
    if(t==2)continue;
    float maxe=0.0f;int bad=0;
    for(float x:xs){float3 v={x,x,x};auto enc=tfe(v,t);auto dec=tfd(enc,t);if(!finite3(enc)||!finite3(dec)){bad++;continue;}maxe=std::max(maxe,std::fabs(dec.x-x));}
    if(bad||maxe>2.0e-4f){std::printf("FAIL transfer t=%d bad=%d err=%g\n",t,bad,maxe);fail++;}
  }
  for(int i=0;i<=10;i++){
    float maxe=0.0f;int bad=0;
    for(float x:xs){float3 v={x,x,x};auto enc=tfl3e_ei(v,i);auto dec=tfl3d_ei(enc,i);if(!finite3(enc)||!finite3(dec)){bad++;continue;}maxe=std::max(maxe,std::fabs(dec.x-x));}
    if(bad||maxe>2.0e-4f){std::printf("FAIL LogC3 EI=%d bad=%d err=%g\n",i,bad,maxe);fail++;}
  }

  // Exact neutral/bypass across every input space and EI. Permanent negative architecture must not add a hidden look.
  std::vector<float3> codes={{0,0,0},{0.18f,0.2f,0.22f},{0.4f,0.5f,0.6f},{1.0f,0.1f,-0.1f},{1.5f,-0.2f,2.0f}};
  for(int sp=0;sp<=17;sp++){
    defaults();input_space=(float)sp;
    for(int ei=0;ei<=10;ei++){
      logc3_ei=(float)ei;
      for(auto c:codes){auto o=transform(1,1,0,0,c.x,c.y,c.z);if(o.x!=c.x||o.y!=c.y||o.z!=c.z){std::printf("FAIL neutral sp=%d ei=%d\n",sp,ei);fail++;goto neutral_done;}}
    }
    neutral_done: ;
  }

  // Negative characteristic is reversible and monotonic for the full legal response range.
  {
    std::vector<float> vals={-0.25f,-0.02f,0.0f,0.001f,0.01f,0.05f,0.18f,0.42f,1.0f,4.0f,16.0f,64.0f};
    struct FP{float mo,cb,ca;};
    std::vector<FP> fps={{.42f,1,1},{.32f,.75f,.9f},{.60f,1.8f,.55f},{.12f,.2f,2.7f},{.80f,3.0f,.1f}};
    for(auto q:fps){
      float prev=-1.0e30f;
      for(int i=0;i<=20000;i++){
        float x=-.1f+8.1f*(float)i/20000.0f;
        float y=fns_forward_characteristic(x,.18f,q.mo,q.cb,q.ca,.10f);
        if(!std::isfinite(y)||y+2.0e-5f<prev){std::printf("FAIL FNS monotonic mo=%g x=%g y=%g prev=%g\n",q.mo,x,y,prev);fail++;break;}
        prev=y;
      }
      for(float x:vals){
        float3 v={x,x*.73f,x*1.21f};
        auto y=fns_forward_rgb(v,.18f,q.mo,q.cb,q.ca,.10f);
        auto z=fns_inverse_rgb(y,.18f,q.mo,q.cb,q.ca,.10f);
        float tol=3.0e-4f;
        if(!finite3(y)||!finite3(z)||maxabs3(z-v)>tol*std::max(1.0f,maxabs3(v))){std::printf("FAIL FNS roundtrip x=%g err=%g\n",x,maxabs3(z-v));fail++;break;}
      }
    }
  }

  // Printer lights are a real grade inside the sandwich and must NOT cancel on exit.
  {
    defaults();input_space=2;float3 in={.42f,.36f,.31f};auto neutral=transform(1,1,0,0,in.x,in.y,in.z);
    defaults();input_space=2;fns_printer_r=30.0f;auto red=transform(1,1,0,0,in.x,in.y,in.z);
    defaults();input_space=2;fns_printer_g=30.0f;auto green=transform(1,1,0,0,in.x,in.y,in.z);
    defaults();input_space=2;fns_printer_b=30.0f;auto blue=transform(1,1,0,0,in.x,in.y,in.z);
    if(maxabs3(red-neutral)<1e-4f||maxabs3(green-neutral)<1e-4f||maxabs3(blue-neutral)<1e-4f){std::printf("FAIL printer lights cancel or inactive\n");fail++;}
    if(!(red.x>neutral.x && green.y>neutral.y && blue.z>neutral.z)){std::printf("FAIL printer light channel direction\n");fail++;}
  }

  // Working-response controls alone are not a look; changing them with no in-sandwich grade stays exact neutral.
  {
    float3 in={.31f,.44f,.58f};
    defaults();input_space=2;fns_mid_out=.73f;fns_contrast_below=.2f;fns_contrast_above=2.7f;
    auto o=transform(1,1,0,0,in.x,in.y,in.z);
    if(o.x!=in.x||o.y!=in.y||o.z!=in.z){std::printf("FAIL response-only neutral\n");fail++;}
  }

  // But those response controls must materially change how the same tone move behaves.
  {
    float3 in={.55f,.42f,.30f};
    defaults();input_space=2;p_highlights=.7f;auto a=transform(1,1,0,0,in.x,in.y,in.z);
    defaults();input_space=2;p_highlights=.7f;fns_mid_out=.60f;fns_contrast_below=.6f;fns_contrast_above=.55f;auto b=transform(1,1,0,0,in.x,in.y,in.z);
    if(maxabs3(a-b)<1.0e-4f){std::printf("FAIL negative response controls do not affect tone feel\n");fail++;}
  }

  // Color lives after Negative Space EXIT: response-shape parameters cannot contaminate pure Chroma/Hue/Vibrance operations.
  {
    float3 in={.52f,.31f,.68f};
    defaults();input_space=2;chroma=1.3f;vibrance=.25f;hue_rotate=17;auto a=transform(1,1,0,0,in.x,in.y,in.z);
    defaults();input_space=2;chroma=1.3f;vibrance=.25f;hue_rotate=17;fns_mid_out=.75f;fns_contrast_below=.2f;fns_contrast_above=2.8f;auto b=transform(1,1,0,0,in.x,in.y,in.z);
    if(maxabs3(a-b)>2.0e-6f){std::printf("FAIL color contaminated by negative response err=%g\n",maxabs3(a-b));fail++;}
  }

  // ME_Desatch exact parity when it is the only active module.
  {
    std::vector<float3> ins={{.1f,.2f,.3f},{.8f,.3f,.15f},{1.1f,-.05f,.5f}};
    for(int sp=0;sp<=17;sp++)for(auto in:ins){
      defaults();input_space=(float)sp;global_sat=-.18f;r_sat=-.2f;g_sat=-.1f;b_sat=-.3f;c_sat=-.12f;m_sat=-.08f;y_sat=-.16f;
      auto expected=apply_me_desatch_exact(in,global_sat,r_sat,g_sat,b_sat,c_sat,m_sat,y_sat);
      auto out=transform(1,1,0,0,in.x,in.y,in.z);
      if(maxabs3(out-expected)>2.0e-6f){std::printf("FAIL ME exact parity sp=%d err=%g\n",sp,maxabs3(out-expected));fail++;goto me_done;}
    }
    me_done: ;
  }

  // Negative-space highlight helper remains monotonic and continuous.
  for(float hv : {-1.0f,-0.5f,0.5f,1.0f}){
    float gain=std::pow(2.0f,hv),mid=.42f,prev=-1.0e30f;
    for(int i=0;i<=24000;i++){
      float x=mid+(1.5f-mid)*(float)i/24000.0f;float y=keystone_highlight_gain_monotonic(x,gain,mid);
      if(y+1.0e-6f<prev){std::printf("FAIL highlight helper hv=%g x=%g\n",hv,x);fail++;break;}prev=y;
    }
    float jump=std::fabs(keystone_highlight_gain_monotonic(1.000001f,gain,mid)-keystone_highlight_gain_monotonic(0.999999f,gain,mid));
    if(jump>1.0e-4f){std::printf("FAIL highlight continuity hv=%g jump=%g\n",hv,jump);fail++;}
  }

  // Known high-risk legal color/desatch combinations stay finite and above the encoded catastrophic floor.
  struct Case{float chroma,vib,hue,gs,rs,grs,bs,cs,ms,ys;};
  std::vector<Case> cases={{2,0,0,0,0,0,0,0,0,0},{1,0,180,0,0,0,0,0,0,0},{2,1,-180,-1,-1,-1,-1,-1,-1,-1},{.5f,-1,90,-.7f,-1,0,-.5f,-.8f,-.3f,-.9f}};
  std::vector<float3> inputs={{.8f,.1f,.1f},{.1f,.8f,.1f},{.1f,.1f,.8f},{1.0f,.02f,.3f},{.02f,1.0f,.8f}};
  for(int sp=0;sp<=17;sp++)for(auto c:cases)for(auto in:inputs){
    defaults();input_space=(float)sp;chroma=c.chroma;vibrance=c.vib;hue_rotate=c.hue;global_sat=c.gs;r_sat=c.rs;g_sat=c.grs;b_sat=c.bs;c_sat=c.cs;m_sat=c.ms;y_sat=c.ys;
    auto o=transform(1,1,0,0,in.x,in.y,in.z);
    if(!finite3(o)||min3(o)<-1.0005f){std::printf("FAIL encoded safety sp=%d out=%g,%g,%g\n",sp,o.x,o.y,o.z);fail++;goto encoded_done;}
  }
  encoded_done: ;

  // Randomized stress with final finite fallback disabled by Python runner.
  unsigned long long state=0x123456789abcdefULL;
  auto rnd=[&](){state^=state<<7;state^=state>>9;state^=state<<8;return(float)((state>>11)&0xFFFFFF)/(float)0xFFFFFF;};
  auto rr=[&](float a,float b){return a+(b-a)*rnd();};
  const int N=150000;
  for(int sp=0;sp<=17;sp++){
    for(int n=0;n<N;n++){
      defaults();input_space=(float)sp;
      logc3_ei=(float)(int)(rnd()*11.0f);if(logc3_ei>10)logc3_ei=10;
      input_gamut_heal=(rnd()<0.15f)?1.0f:0.0f;temp=rr(-100,100);tint=rr(-100,100);
      fns_mid_out=rr(.10f,.80f);fns_contrast_below=rr(.1f,3.0f);fns_contrast_above=rr(.1f,3.0f);fns_printer_r=rr(0,50);fns_printer_g=rr(0,50);fns_printer_b=rr(0,50);
      balance_red=rr(-.5,.5);balance_green=rr(-.5,.5);balance_blue=rr(-.5,.5);
      p_exposure=rr(-6,6);p_bp=rr(-.05,.05);p_contrast=rr(.5,2);p_pivot_offset=rr(-.2,.2);p_shadows=rr(-1,1);p_highlights=rr(-1,1);p_high_soft=rr(0,2);
      chroma=rr(0,2);vibrance=rr(-1,1);hue_rotate=rr(-180,180);
      warm_target=rr(-25,25);warm_hue_shift=rr(-25,25);warm_chroma=rr(.5,1.5);warm_exposure=rr(-.5,.5);warm_evenness=rr(0,1);
      global_sat=rr(-1,0);r_sat=rr(-1,0);g_sat=rr(-1,0);b_sat=rr(-1,0);c_sat=rr(-1,0);m_sat=rr(-1,0);y_sat=rr(-1,0);
      native_negative_compress=rr(0,1);black_gamut_clean=rr(0,1);output_skin_protect=rr(0,1);
      float3 in={rr(-.25f,1.5f),rr(-.25f,1.5f),rr(-.25f,1.5f)};auto o=transform(1,1,0,0,in.x,in.y,in.z);
      if(!finite3(o)||maxabs3(o)>1.0e4f||min3(o)<-1.0005f){std::printf("FAIL stress sp=%d n=%d in=%g,%g,%g out=%g,%g,%g mid=%g cb=%g ca=%g pr=%g pg=%g pb=%g exp=%g con=%g chroma=%g vib=%g hue=%g gs=%g\n",sp,n,in.x,in.y,in.z,o.x,o.y,o.z,fns_mid_out,fns_contrast_below,fns_contrast_above,fns_printer_r,fns_printer_g,fns_printer_b,p_exposure,p_contrast,chroma,vibrance,hue_rotate,global_sat);fail++;goto stress_done;}
    }
  }
  stress_done: ;

  if(fail){std::printf("FAILURES=%d\n",fail);return 1;}
  std::printf("PASS Keystone RC27 behavioral suite: permanent negative tone stage, live printer lights, scene-referred color, exact ME_Desatch stage, 2700000 randomized transforms\n");
  return 0;
}
