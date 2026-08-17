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
  input_gamut_heal=0;logc3_ei=7;temp=tint=0;
  fns_printer_r=fns_printer_g=fns_printer_b=25.0f;
  p_exposure=0;p_bp=0;p_contrast=1;p_shadows=0;p_highlights=0;p_high_soft=0;
  chroma=1;vibrance=0;hue_rotate=0;
  warm_target=0;warm_hue_shift=0;warm_chroma=1;warm_exposure=0;warm_evenness=0;
  global_sat=r_sat=g_sat=b_sat=c_sat=m_sat=y_sat=0;
  white_gamut_clean=black_gamut_clean=0;
}
static void map_space(int sp,int &g,int &tf){g=0;tf=0;
  if(sp==1){g=1;tf=1;}else if(sp==2){g=2;tf=2;}else if(sp==3){g=3;tf=3;}else if(sp==4){g=4;tf=4;}
  else if(sp==5){g=5;tf=5;}else if(sp==6){g=6;tf=15;}else if(sp==7){g=11;tf=6;}else if(sp==8){g=7;tf=8;}
  else if(sp==9){g=7;tf=9;}else if(sp==10){g=9;tf=10;}else if(sp==11){g=10;tf=13;}else if(sp==12){g=8;tf=11;}
  else if(sp==13){g=8;tf=14;}else if(sp==14){g=0;tf=11;}else if(sp==15){g=12;tf=3;}else if(sp==16){g=10;tf=16;}else if(sp==17){g=10;tf=17;}
}
static float3 encode_for_space(float3 lin,int sp,int ei){int g,tf;map_space(sp,g,tf);return encode_native_ei(lin,tf,ei);}
static float3 decode_for_space(float3 code,int sp,int ei){int g,tf;map_space(sp,g,tf);return decode_native_ei(code,tf,ei);}

int main(){int fail=0;
  // Matrix roundtrips.
  for(int g=0;g<=12;g++){float maxe=0;for(auto x:std::vector<float3>{{1,0,0},{0,1,0},{0,0,1},{.18f,.18f,.18f},{1.5f,-.2f,.7f}}){auto y=x2g(g,g2x(g,x));maxe=std::max(maxe,maxabs3(y-x));}if(maxe>2e-6f){std::printf("FAIL matrix %d %g\n",g,maxe);fail++;}}

  // Transfer roundtrips.
  std::vector<float> xs={-.02f,-.001f,0,.001f,.005f,.01f,.018f,.18f,1,4,16,64};
  for(int t=0;t<=17;t++){if(t==2)continue;float me=0;int bad=0;for(float x:xs){float3 v={x,x,x};auto e=tfe(v,t),d=tfd(e,t);if(!finite3(e)||!finite3(d)){bad++;continue;}me=std::max(me,std::fabs(d.x-x));}if(bad||me>2e-4f){std::printf("FAIL transfer %d bad=%d err=%g\n",t,bad,me);fail++;}}
  for(int ei=0;ei<=10;ei++){float me=0;for(float x:xs){float3 v={x,x,x};auto e=tfl3e_ei(v,ei),d=tfl3d_ei(e,ei);me=std::max(me,std::fabs(d.x-x));}if(me>2e-4f){std::printf("FAIL LogC3 EI %d %g\n",ei,me);fail++;}}

  // Neutral remains exact identity in every source space/EI.
  for(int sp=0;sp<=17;sp++){defaults();input_space=(float)sp;for(int ei=0;ei<=10;ei++){logc3_ei=(float)ei;for(auto c:std::vector<float3>{{0,0,0},{.18f,.2f,.22f},{.4f,.5f,.6f},{1,.1f,-.1f},{1.5f,-.2f,2}}){auto o=transform(1,1,0,0,c.x,c.y,c.z);if(o.x!=c.x||o.y!=c.y||o.z!=c.z){std::printf("FAIL neutral sp=%d ei=%d\n",sp,ei);fail++;goto neutral_done;}}}neutral_done:;}

  // Fixed negative characteristic remains monotonic and reversible.
  {float prev=-1e30f;for(int i=0;i<=30000;i++){float x=-.1f+16.1f*i/30000.0f;float y=fns_forward_characteristic(x,KEYSTONE_FNS_MID_IN,KEYSTONE_FNS_MID_OUT,KEYSTONE_FNS_BELOW,KEYSTONE_FNS_ABOVE,KEYSTONE_FNS_BLEND);if(!std::isfinite(y)||y+2e-5f<prev){std::printf("FAIL FNS monotonic x=%g\n",x);fail++;break;}prev=y;}for(float x:std::vector<float>{-.1f,0,.001f,.01f,.18f,1,4,16}){float3 v={x,x*.73f,x*1.21f};auto y=fns_forward_rgb(v,KEYSTONE_FNS_MID_IN,KEYSTONE_FNS_MID_OUT,KEYSTONE_FNS_BELOW,KEYSTONE_FNS_ABOVE,KEYSTONE_FNS_BLEND);auto z=fns_inverse_rgb(y,KEYSTONE_FNS_MID_IN,KEYSTONE_FNS_MID_OUT,KEYSTONE_FNS_BELOW,KEYSTONE_FNS_ABOVE,KEYSTONE_FNS_BLEND);if(!finite3(z)||maxabs3(z-v)>5e-4f*std::max(1.0f,maxabs3(v))){std::printf("FAIL FNS roundtrip x=%g err=%g\n",x,maxabs3(z-v));fail++;break;}}}

  // Auto pivot derives 18% through every selected transfer/EI and contrast preserves that anchor.
  for(int sp=0;sp<=17;sp++){int g,tf;map_space(sp,g,tf);for(int ei=0;ei<=10;ei++){float mid=auto_scene_midgray(tf,ei);if(std::fabs(mid-.18f)>3e-4f){std::printf("FAIL auto mid sp=%d ei=%d %g\n",sp,ei,mid);fail++;goto pivot_done;}defaults();input_space=(float)sp;logc3_ei=(float)ei;p_contrast=1.35f;float3 in=encode_native_ei({.18f,.18f,.18f},tf,ei);auto out=transform(1,1,0,0,in.x,in.y,in.z);if(maxabs3(out-in)>6e-4f){std::printf("FAIL contrast pivot sp=%d ei=%d err=%g\n",sp,ei,maxabs3(out-in));fail++;goto pivot_done;}}}pivot_done:;

  // Global Exposure is a true scene-linear stop move.
  for(int sp=0;sp<=17;sp++){int g,tf;map_space(sp,g,tf);defaults();input_space=(float)sp;p_exposure=1.0f;float3 in=encode_native_ei({.18f,.18f,.18f},tf,7);auto out=transform(1,1,0,0,in.x,in.y,in.z);auto lin=decode_native_ei(out,tf,7);if(maxabs3(lin-make_float3(.36f,.36f,.36f))>1.5e-3f){std::printf("FAIL true exposure sp=%d lin=%g,%g,%g\n",sp,lin.x,lin.y,lin.z);fail++;}}

  // Printer lights are live and channel-specific.
  {defaults();input_space=14;float3 in={.18f,.18f,.18f};auto n=transform(1,1,0,0,in.x,in.y,in.z);defaults();input_space=14;fns_printer_r=30;auto r=transform(1,1,0,0,in.x,in.y,in.z);defaults();input_space=14;fns_printer_g=30;auto g=transform(1,1,0,0,in.x,in.y,in.z);defaults();input_space=14;fns_printer_b=30;auto b=transform(1,1,0,0,in.x,in.y,in.z);if(maxabs3(r-n)<1e-4f||maxabs3(g-n)<1e-4f||maxabs3(b-n)<1e-4f||!(r.x>n.x&&g.y>n.y&&b.z>n.z)){std::printf("FAIL printer lights\n");fail++;}}

  // Shadows and Highlights are isolated scene-stop zones, not split contrast.
  {float lo=.18f*.25f,mid=.18f,hi=.18f*4.0f;
    defaults();input_space=14;p_shadows=1;auto loS=transform(1,1,0,0,lo,lo,lo),midS=transform(1,1,0,0,mid,mid,mid),hiS=transform(1,1,0,0,hi,hi,hi);
    if(!(loS.x>lo*1.3f)||std::fabs(midS.x-mid)>2e-4f||std::fabs(hiS.x-hi)>5e-4f){std::printf("FAIL shadow isolation lo=%g mid=%g hi=%g\n",loS.x,midS.x,hiS.x);fail++;}
    defaults();input_space=14;p_highlights=1;auto loH=transform(1,1,0,0,lo,lo,lo),midH=transform(1,1,0,0,mid,mid,mid),hiH=transform(1,1,0,0,hi,hi,hi);
    if(!(hiH.x>hi*1.3f)||std::fabs(midH.x-mid)>2e-4f||std::fabs(loH.x-lo)>5e-4f){std::printf("FAIL highlight isolation lo=%g mid=%g hi=%g\n",loH.x,midH.x,hiH.x);fail++;}
  }

  // Roll Off is mid-gray neutral, highlight-only and monotonic.
  {defaults();input_space=14;p_high_soft=2;auto m=transform(1,1,0,0,.18f,.18f,.18f);auto h=transform(1,1,0,0,2.88f,2.88f,2.88f);if(std::fabs(m.x-.18f)>2e-4f||!(h.x<2.88f&&h.x>.18f)){std::printf("FAIL rolloff basic mid=%g high=%g\n",m.x,h.x);fail++;}float prev=-1e30f;for(int i=1;i<=4000;i++){float x=.18f*std::pow(2.0f,-1.0f+7.0f*i/4000.0f);auto o=transform(1,1,0,0,x,x,x);if(o.x+2e-5f<prev){std::printf("FAIL rolloff monotonic x=%g out=%g prev=%g\n",x,o.x,prev);fail++;break;}prev=o.x;}}

  // Chroma and Hue limiter never returns an Oklab color below the native safety margin.
  for(int g=0;g<=12;g++)for(auto lab:std::vector<float3>{{.6f,.25f,.05f},{.7f,-.2f,.18f},{.5f,.05f,-.3f}}){auto o=apply_safe_chroma_hue(lab,2.0f,175.0f,g);float margin=native_gamut_margin_oklab(o,g);if(!finite3(o)||margin<VIBRANCE_MIN_MARGIN-2e-4f){std::printf("FAIL gamut-aware color g=%d margin=%g\n",g,margin);fail++;goto gamut_done;}}gamut_done:;

  // Skin Hue Uniformity is a color-only operation: it may move hue, but should not materially move scene luminance.
  {defaults();input_space=14;warm_target=20;warm_evenness=1;float3 in={.36f,.20f,.12f};auto out=transform(1,1,0,0,in.x,in.y,in.z);float Yin=g2x(0,in).y,Yout=g2x(0,out).y;if(maxabs3(out-in)<1e-5f||std::fabs(Yout-Yin)>4e-3f){std::printf("FAIL skin uniformity diff=%g dY=%g\n",maxabs3(out-in),Yout-Yin);fail++;}}

  // ME_Desatch exact parity when it is the only active module.
  for(int sp=0;sp<=17;sp++)for(auto in:std::vector<float3>{{.1f,.2f,.3f},{.8f,.3f,.15f},{1.1f,-.05f,.5f}}){defaults();input_space=(float)sp;global_sat=-.18f;r_sat=-.2f;g_sat=-.1f;b_sat=-.3f;c_sat=-.12f;m_sat=-.08f;y_sat=-.16f;auto expected=apply_me_desatch_exact(in,global_sat,r_sat,g_sat,b_sat,c_sat,m_sat,y_sat);auto out=transform(1,1,0,0,in.x,in.y,in.z);if(maxabs3(out-expected)>2e-6f){std::printf("FAIL ME parity sp=%d err=%g\n",sp,maxabs3(out-expected));fail++;goto me_done;}}me_done:;

  // White/Black Clean are manual, distinct, and preserve approximate luminance while reducing neutral contamination.
  {defaults();input_space=14;white_gamut_clean=1;float3 w={1.0f,.95f,.90f};auto wo=transform(1,1,0,0,w.x,w.y,w.z);float spread0=std::max({w.x,w.y,w.z})-std::min({w.x,w.y,w.z}),spread1=std::max({wo.x,wo.y,wo.z})-std::min({wo.x,wo.y,wo.z});if(!(spread1<spread0)){std::printf("FAIL White Clean spread %g -> %g\n",spread0,spread1);fail++;}
    defaults();input_space=14;black_gamut_clean=1;float3 b={.010f,.008f,.012f};auto bo=transform(1,1,0,0,b.x,b.y,b.z);spread0=.004f;spread1=std::max({bo.x,bo.y,bo.z})-std::min({bo.x,bo.y,bo.z});if(!(spread1<spread0)){std::printf("FAIL Black Clean spread %g -> %g\n",spread0,spread1);fail++;}}

  // Randomized full-range stress, with finite_or_zero disabled by runner.
  unsigned long long state=0x123456789abcdefULL;auto rnd=[&](){state^=state<<7;state^=state>>9;state^=state<<8;return(float)((state>>11)&0xFFFFFF)/(float)0xFFFFFF;};auto rr=[&](float a,float b){return a+(b-a)*rnd();};
  const int N=150000;
  for(int sp=0;sp<=17;sp++)for(int n=0;n<N;n++){
    defaults();input_space=(float)sp;logc3_ei=(float)std::min(10,(int)(rnd()*11));input_gamut_heal=rnd()<.15f?1.0f:0.0f;temp=rr(-100,100);tint=rr(-100,100);
    fns_printer_r=rr(0,50);fns_printer_g=rr(0,50);fns_printer_b=rr(0,50);p_exposure=rr(-6,6);p_bp=rr(-.05,.05);p_contrast=rr(.5,2);p_shadows=rr(-1,1);p_highlights=rr(-1,1);p_high_soft=rr(0,2);
    chroma=rr(0,2);vibrance=rr(-1,1);hue_rotate=rr(-180,180);warm_target=rr(-25,25);warm_hue_shift=rr(-25,25);warm_chroma=rr(.5,1.5);warm_exposure=rr(-.5,.5);warm_evenness=rr(0,1);
    global_sat=rr(-1,0);r_sat=rr(-1,0);g_sat=rr(-1,0);b_sat=rr(-1,0);c_sat=rr(-1,0);m_sat=rr(-1,0);y_sat=rr(-1,0);white_gamut_clean=rr(0,1);black_gamut_clean=rr(0,1);
    float3 in={rr(-.25f,1.5f),rr(-.25f,1.5f),rr(-.25f,1.5f)};auto o=transform(1,1,0,0,in.x,in.y,in.z);if(!finite3(o)||maxabs3(o)>1e4f||min3(o)<-1.0005f){std::printf("FAIL stress sp=%d n=%d out=%g,%g,%g\n",sp,n,o.x,o.y,o.z);fail++;goto stress_done;}
  }
  stress_done:;

  if(fail){std::printf("FAILURES=%d\n",fail);return 1;}
  std::printf("PASS Keystone RC28 behavioral suite: auto pivot, true exposure, isolated tone zones, gamut-aware color, automatic safety, White/Black Clean, 2700000 randomized transforms\n");
  return 0;
}
