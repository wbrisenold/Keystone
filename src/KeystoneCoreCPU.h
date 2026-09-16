#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "KeystoneParams.h"

namespace keystone_cpu {
struct float2 { float x,y; };
struct float3 { float x,y,z; };
inline float2 make_float2(float x,float y){return {x,y};}
inline float3 make_float3(float x,float y,float z){return {x,y,z};}
inline float2 operator+(float2 a,float2 b){return {a.x+b.x,a.y+b.y};}
inline float2 operator-(float2 a,float2 b){return {a.x-b.x,a.y-b.y};}
inline float2 operator*(float2 a,float2 b){return {a.x*b.x,a.y*b.y};}
inline float2 operator/(float2 a,float2 b){return {a.x/b.x,a.y/b.y};}
inline float2 operator*(float2 a,float s){return {a.x*s,a.y*s};}
inline float2 operator*(float s,float2 a){return a*s;}
inline float2 operator/(float2 a,float s){return {a.x/s,a.y/s};}
inline float2& operator+=(float2& a,float2 b){a.x+=b.x;a.y+=b.y;return a;}
inline float3 operator+(float3 a,float3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline float3 operator-(float3 a,float3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline float3 operator-(float3 a){return {-a.x,-a.y,-a.z};}
inline float3 operator*(float3 a,float3 b){return {a.x*b.x,a.y*b.y,a.z*b.z};}
inline float3 operator/(float3 a,float3 b){return {a.x/b.x,a.y/b.y,a.z/b.z};}
inline float3 operator*(float3 a,float s){return {a.x*s,a.y*s,a.z*s};}
inline float3 operator*(float s,float3 a){return a*s;}
inline float3 operator/(float3 a,float s){return {a.x/s,a.y/s,a.z/s};}
inline float3& operator+=(float3& a,float3 b){a.x+=b.x;a.y+=b.y;a.z+=b.z;return a;}
inline float3& operator-=(float3& a,float3 b){a.x-=b.x;a.y-=b.y;a.z-=b.z;return a;}
inline float3& operator*=(float3& a,float3 b){a.x*=b.x;a.y*=b.y;a.z*=b.z;return a;}
inline float3& operator*=(float3& a,float s){a.x*=s;a.y*=s;a.z*=s;return a;}
inline float3& operator/=(float3& a,float s){a.x/=s;a.y/=s;a.z/=s;return a;}

#define __DEVICE__ inline
#define _powf std::pow
#define _fabs std::fabs
#define _fmaxf std::fmax
#define _fminf std::fmin
#define _logf std::log
#define _log10f std::log10
#define _log2f std::log2
#define _expf std::exp
#define _sqrtf std::sqrt
#define _atan2f std::atan2
#define _sinf std::sin
#define _cosf std::cos
#define _tanf std::tan
#define _acosf std::acos
#define _floorf std::floor
#include "../generated/KeystoneConstants.inc"
#include "../generated/KeystoneMath.inc"
#undef __DEVICE__

using keystone::Params;
using keystone::LutEntry;

inline float3 preNeutral(float3 input,const Params& p){
  float3 lin=logc4_decode(input);
  { float3 repair_xyz=awg4_to_xyz(lin); repair_xyz=input_gamut_heal_xyz(repair_xyz); lin=xyz_to_awg4(repair_xyz); }
  int nd=p.ndFilter; if(nd<0||nd>11)nd=0;
  if(nd!=0)lin=ks_apply_hoya_nd(lin,nd);
  if(p.uvFilter||p.irFilter)lin=ks_apply_spektrafilm_uv_ir(lin,p.uvFilter,clampf(p.uvCutNm,350.0f,500.0f),p.irFilter,clampf(p.irCutNm,600.0f,800.0f));
  float wt=clampf(p.wbTemp,-100.0f,100.0f), wi=clampf(p.wbTint,-100.0f,100.0f);
  if(_fabs(wt)>1e-7f||_fabs(wi)>1e-7f)lin=ks_apply_opponent_wb_awg4(lin,wt,wi);
  float exp=clampf(p.exposure,-6.0f,6.0f);
  if(_fabs(exp)>1e-7f){float k=_powf(2.0f,exp);lin*=k;}
  float bp=clampf(p.blackPoint,-0.05f,0.05f);
  if(_fabs(bp)>1e-9f)lin=primera_black_luma(lin,bp);
  lin=primera_tone_luma(lin,clampf(p.contrast,0.5f,2.0f),clampf(p.shadows,-1.0f,1.0f),clampf(p.highlights,-1.0f,1.0f),clampf(p.roll,0.0f,2.0f));
  int curve=p.curvePreset;if(curve<0)curve=0;if(curve>4)curve=4;
  if(curve>0)lin=ks_apply_curve_preset(lin,curve,KS_INTERNAL_CURVE_AMOUNT);
  float ss=clampf(p.satSplit,0.5f,2.0f),den=clampf(p.density,0.0f,1.0f),ps=clampf(p.posSat,0.0f,1.0f),il=clampf(p.interlayer,0.0f,3.0f);
  if(_fabs(ss-1.0f)>1e-7f){lin=ks_apply_contour_split_sat(lin,ss);lin=ks_soft_gamut_bend(lin);}
  if(den>1e-7f){lin=ks_apply_contour_density(lin,den);lin=ks_soft_gamut_bend(lin);}
  if(ps>1e-7f){lin=apply_pos_sat_filmic(lin,ps);lin=ks_soft_gamut_bend(lin);}
  if(il>1e-7f){lin=ks_apply_genesis_interlayer(lin,il);lin=ks_soft_gamut_bend(lin);}
  return lin;
}

inline float3 applyStoredNeutral(float3 lin,const Params& p){
  float a=clampf(p.neutralAmount,0.0f,1.0f);
  if(a<=1e-7f)return lin;
  float3 matched=make_float3(lin.x*p.neutralGainR,lin.y*p.neutralGainG,lin.z*p.neutralGainB);
  return lin+(matched-lin)*a;
}

inline float3 lutFetch(const LutEntry* lut,int r,int g,int b){
  const auto &v=lut[r+33*(g+33*b)]; return make_float3(v.r,v.g,v.b);
}
inline float3 referentSample(float3 result,const LutEntry* lut){
  float xr=result.x*32.0f,xg=result.y*32.0f,xb=result.z*32.0f;
  int r0,g0,b0;float fr,fg,fb;
  if(xr<=0){r0=0;fr=xr;}else if(xr>=32){r0=31;fr=xr-31;}else{r0=(int)_floorf(xr);if(r0>31)r0=31;fr=xr-r0;}
  if(xg<=0){g0=0;fg=xg;}else if(xg>=32){g0=31;fg=xg-31;}else{g0=(int)_floorf(xg);if(g0>31)g0=31;fg=xg-g0;}
  if(xb<=0){b0=0;fb=xb;}else if(xb>=32){b0=31;fb=xb-31;}else{b0=(int)_floorf(xb);if(b0>31)b0=31;fb=xb-b0;}
  int r1=r0+1,g1=g0+1,b1=b0+1;
  float3 c000=lutFetch(lut,r0,g0,b0),c100=lutFetch(lut,r1,g0,b0),c010=lutFetch(lut,r0,g1,b0),c110=lutFetch(lut,r1,g1,b0);
  float3 c001=lutFetch(lut,r0,g0,b1),c101=lutFetch(lut,r1,g0,b1),c011=lutFetch(lut,r0,g1,b1),c111=lutFetch(lut,r1,g1,b1);
  float3 c00=mix3(c000,c100,fr),c10=mix3(c010,c110,fr),c01=mix3(c001,c101,fr),c11=mix3(c011,c111,fr);
  return mix3(mix3(c00,c10,fg),mix3(c01,c11,fg),fb);
}

inline float3 processPixel(float3 input,const Params& p,const LutEntry* lut){
  float3 lin=applyStoredNeutral(preNeutral(input,p),p);
  float sa=clampf(p.splitAmount,0.0f,2.0f);
  if(sa>1e-7f){lin=ks_apply_contour_split_tone(lin,sa,clampf(p.splitShadowHue,0.0f,360.0f),clampf(p.splitHighlightHue,0.0f,360.0f),clampf(p.splitBalance,0.0f,1.0f),clampf(p.splitSubtractive,0.0f,1.0f));lin=ks_soft_gamut_bend(lin);}
  float hb=clampf(p.hiBleach,0.0f,1.0f),lb=clampf(p.loBleach,0.0f,1.0f);
  if(hb>1e-7f) lin=ks_highlight_bleach(lin,hb);
  if(lb>1e-7f) lin=ks_shadow_bleach(lin,lb);
  float fade=clampf(p.fade,0.0f,100.0f)/100.0f;if(fade>1e-7f)lin=ks_apply_tonelab_fade(lin,fade);
  int look=p.lookColor;if(look<0)look=0;if(look>11)look=11;float la=clampf(p.lookAmount,0.0f,1.5f);if(look>0&&la>1e-7f)lin=ks_apply_color_look(lin,look,la);
  if(p.skinShowMask){float m=ks_tonelab_skin_mask_awg4(lin,p.skinCenter,p.skinRange,p.skinBrightness,p.skinBrightnessRange);return make_float3(m,m,m);}
  if(p.skinEnable)lin=ks_apply_tonelab_skin(lin,p.skinEnable,p.skinSaturate,p.skinColour,p.skinPop,p.skinCenter,p.skinRange,p.skinBrightness,p.skinBrightnessRange,p.skinIntensity);
  float3 xyz=awg4_to_xyz(lin);int cw=p.creativeWhite;if(cw<0)cw=0;if(cw>5)cw=5;if(cw!=2){xyz=ks_apply_creative_white_xyz(xyz,cw,KS_INTERNAL_CREATIVE_WHITE_LIMIT);lin=xyz_to_awg4(xyz);}
  lin=technical_encoded_negative_guard(lin,xyz.y);xyz=awg4_to_xyz(lin);float3 cam=xyz_to_awg4(xyz);cam=technical_scene_ceiling_guard(cam);float currentY=awg4_to_xyz(cam).y;cam=technical_encoded_negative_guard(cam,currentY);float3 result=logc4_encode(cam);
  if(!(finitef(result.x)&&finitef(result.y)&&finitef(result.z)))result=(finitef(input.x)&&finitef(input.y)&&finitef(input.z))?input:make_float3(0,0,0);
  float3 display=referentSample(result,lut);
  float bleach=clampf(p.colorBleach,0.0f,80.0f)/100.0f;if(bleach>1e-8f)display=tl_bleach_bypass_exact(display,bleach);
  return (finitef(display.x)&&finitef(display.y)&&finitef(display.z))?display:make_float3(0,0,0);
}
}
