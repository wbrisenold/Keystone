__DEVICE__ float clampf(float v,float lo,float hi){return _fmaxf(lo,_fminf(hi,v));}
__DEVICE__ float3 mmul(float3 c0,float3 c1,float3 c2,float3 v){return make_float3(c0.x*v.x+c1.x*v.y+c2.x*v.z,c0.y*v.x+c1.y*v.y+c2.y*v.z,c0.z*v.x+c1.z*v.y+c2.z*v.z);}
__DEVICE__ int finitef(float x){return (x==x&&_fabs(x)<3.402823e38f)?1:0;}
__DEVICE__ float cbrt_s(float x){return(x<0.0f?-1.0f:1.0f)*_powf(_fabs(x),1.0f/3.0f);}
__DEVICE__ float smootherstep(float e0,float e1,float x){float span=e1-e0;if(span<=1.0e-12f)return x>=e1?1.0f:0.0f;x=clampf((x-e0)/span,0.0f,1.0f);return x*x*x*(x*(x*6.0f-15.0f)+10.0f);}
// ---- Contour 2.1.6 procedural color core (recovered from uploaded OFX) ----
// Only image-processing math used by Density, Split Saturation, and Split Toning is ported.
// The plugin's 65^3 LUT export/cube renderer, graph overlays, watermarking, and host plumbing are omitted.
#define CONTOUR_PI 3.14159265358979323846f
#define CONTOUR_TAU 6.28318530717958647692f

__DEVICE__ float contour_dvi_to_lin(float in){
  return in>0.02740668f?_powf(2.0f,(in/0.07329248f)-7.0f)-0.0075f:in/10.44426855f;
}
__DEVICE__ float contour_lin_to_dvi(float in){
  return in>0.00262409f?(_log2f(in+0.0075f)+7.0f)*0.07329248f:in*10.44426855f;
}
__DEVICE__ float3 contour_lin_to_dvi3(float3 v){return make_float3(contour_lin_to_dvi(v.x),contour_lin_to_dvi(v.y),contour_lin_to_dvi(v.z));}
__DEVICE__ float3 contour_dvi_to_lin3(float3 v){return make_float3(contour_dvi_to_lin(v.x),contour_dvi_to_lin(v.y),contour_dvi_to_lin(v.z));}
__DEVICE__ float3 contour_clamp01_3(float3 v){return make_float3(clampf(v.x,0.0f,1.0f),clampf(v.y,0.0f,1.0f),clampf(v.z,0.0f,1.0f));}
__DEVICE__ float contour_sign(float x){return x>0.0f?1.0f:(x<0.0f?-1.0f:0.0f);}
__DEVICE__ float contour_spow(float a,float b){return contour_sign(a)*_powf(_fabs(a),b);}

__DEVICE__ float contour_normalize_gain(float hue,float chroma){
  float angle=hue*CONTOUR_TAU-CONTOUR_PI/6.0f;
  (void)chroma;
  return 1.0f/(2.0f*_cosf((_acosf(_sinf(3.0f*angle)))/3.0f));
}

// Contour rgb_to_subChenYCH / RGB_to_NormSpherical(nonorm): x=norm, y=cone chroma, z=hue turns.
__DEVICE__ float3 contour_rgb_to_subchen(float3 in){
  float norm=_fmaxf(in.x,_fmaxf(in.y,in.z));
  float rx=0.8164965809277260f*in.x-0.4082482904638630f*in.y-0.4082482904638630f*in.z;
  float ry=0.7071067811865475f*in.y-0.7071067811865475f*in.z;
  float rz=0.5773502691896258f*(in.x+in.y+in.z);
  float theta=_atan2f(ry,rx)/CONTOUR_TAU;
  float hue=theta<0.0f?theta+1.0f:theta;
  float chroma=_atan2f(_sqrtf(rx*rx+ry*ry),rz)/_atan2f(_sqrtf(2.0f),1.0f);
  if(chroma<0.0001f&&chroma>0.0f)hue=0.0f;
  if(in.x==0.0f&&in.y==0.0f&&in.z==0.0f)chroma=0.0f;
  return make_float3(norm,chroma,hue);
}

__DEVICE__ float3 contour_subchen_to_rgb(float3 v){
  // source stores [norm,chroma,hue], then swaps norm/hue before reconstruction
  float hue=v.z,chroma=v.y,norm=v.x;
  float hue2=hue+0.5f;
  float cg1=contour_normalize_gain(hue,chroma);
  float cg2=contour_normalize_gain(hue2,chroma);
  float y=_tanf(chroma*_atan2f(_sqrtf(2.0f),1.0f))/_sqrtf(2.0f);
  y/=cg1;
  float ng=(cg2+cg1*y)/cg2;
  norm/=ng;
  y*=cg1;
  float rx=_cosf(hue*CONTOUR_TAU)*y*norm*_sqrtf(3.0f)/(_sqrtf(2.0f)/2.0f);
  float ry=_sinf(hue*CONTOUR_TAU)*y*norm*_sqrtf(3.0f)/(_sqrtf(2.0f)/2.0f);
  float rz=norm*_sqrtf(3.0f);
  return make_float3(
    0.8164965809277260f*rx+0.5773502691896258f*rz,
   -0.4082482904638630f*rx+0.7071067811865475f*ry+0.5773502691896258f*rz,
   -0.4082482904638630f*rx-0.7071067811865475f*ry+0.5773502691896258f*rz);
}

__DEVICE__ float3 contour_chen_to_rgb(float3 sph){
  float r=sph.x*1.7320508075688772f;
  float p=sph.y*0.9553166181245093f;
  float t=sph.z*CONTOUR_TAU;
  float ctr=r*_sinf(p)*_cosf(t);
  float ctg=r*_sinf(p)*_sinf(t);
  float ctb=r*_cosf(p);
  return make_float3(
    ctr*0.81649658f+ctb*0.57735027f,
    ctr*-0.40824829f+ctg*0.70710678f+ctb*0.57735027f,
    ctr*-0.40824829f+ctg*-0.70710678f+ctb*0.57735027f);
}

__DEVICE__ float contour_bez4(float in,float y1,float y2,float y3,float y4){
  float om=1.0f-in;
  return y1*om*om*om+3.0f*y2*om*om*in+3.0f*y3*om*in*in+y4*in*in*in;
}
__DEVICE__ float contour_bez_mask(float in,float threshold,float delta){
  float x1=threshold,x2=threshold+delta;
  if(in<x1)return 1.0f;
  if(in>x2)return 0.0f;
  if(_fabs(delta)<=1.0e-12f)return in<=x1?1.0f:0.0f;
  float y1=(1.0f-x1)/delta,y2=(1.0f-x1)/delta,y3=(0.0f-x1)/delta,y4=(0.0f-x1)/delta;
  float t=(in-x1)/delta;
  return contour_bez4(t,y1,y2,y3,y4)*delta+x1;
}
__DEVICE__ float contour_bell_mask(float in,float center,float width,float strength,float curveEase){
  float out=0.0f;
  float c=center,left=c-width*0.5f,right=c+width*0.5f;
  if(in>left&&in<right)out+=contour_spow(_sinf(CONTOUR_PI*((c-in-width*0.5f)/width)),curveEase)*strength;
  if(left<0.0f){c=center+1.0f;left=c-width*0.5f;right=c+width*0.5f;if(in>left&&in<right)out+=contour_spow(_sinf(CONTOUR_PI*((c-in-width*0.5f)/width)),curveEase)*strength;}
  if(center+width*0.5f>1.0f){c=center-1.0f;left=c-width*0.5f;right=c+width*0.5f;if(in>left&&in<right)out+=contour_spow(_sinf(CONTOUR_PI*((c-in-width*0.5f)/width)),curveEase)*strength;}
  return out;
}

// Contour Density with its default Focus=50 and equal complementary hue weighting (50/50).
// Working transfer is DaVinci Intermediate because those are Contour's recovered DVI hue centers.
__DEVICE__ float3 ks_apply_contour_density(float3 lin,float amount){
  float a=clampf(amount,0.0f,1.0f);
  if(a<=1.0e-8f)return lin;
  float3 dvi_orig=contour_lin_to_dvi3(lin);
  float3 dvi=contour_clamp01_3(dvi_orig);
  float3 lin_base=contour_dvi_to_lin3(dvi);
  float3 logCyl=contour_rgb_to_subchen(dvi);
  float3 linCyl=contour_rgb_to_subchen(contour_clamp01_3(lin_base));
  float focus=0.5f;
  float satPow=4.0f*focus+1.5f*(1.0f-focus); // 2.75 at Contour default Focus 50
  float satMask=_powf(clampf(linCyl.y,0.0f,1.0f),satPow);
  const float w=120.0f/360.0f, ease=2.0f;
  const float cR=2.0f/360.0f,cM=288.0f/360.0f,cB=230.0f/360.0f,cC=164.0f/360.0f,cG=100.0f/360.0f,cY=54.0f/360.0f;
  float hueMask=0.5f*(
    contour_bell_mask(logCyl.z,cR,w,-1.0f,ease)+contour_bell_mask(logCyl.z,cM,w,-1.0f,ease)+
    contour_bell_mask(logCyl.z,cB,w,-1.0f,ease)+contour_bell_mask(logCyl.z,cC,w,-1.0f,ease)+
    contour_bell_mask(logCyl.z,cG,w,-1.0f,ease)+contour_bell_mask(logCyl.z,cY,w,-1.0f,ease));
  float netMask=clampf(satMask*hueMask,0.0f,1.0f);
  float densityGain=(1.0f-a)*0.89f+0.11f; // exact Contour mapping of 0..100 -> 1..0.11
  float3 dense_lin=lin_base*make_float3(densityGain,densityGain,densityGain);
  float3 dense_dvi=contour_lin_to_dvi3(dense_lin);
  float3 candidate=dense_dvi*make_float3(netMask,netMask,netMask)+dvi*make_float3(1.0f-netMask,1.0f-netMask,1.0f-netMask);
  // Preserve Keystone's extended-range carrier: inside [0,1] this is exact Contour behavior;
  // outside it, apply only the Contour delta rather than clipping the base signal.
  float3 out_dvi=dvi_orig+(candidate-dvi);
  return contour_dvi_to_lin3(out_dvi);
}

// Recovered Contour Split Saturation. UI satGain is kept in the source's 0.5..2 domain; 1 bypasses.
__DEVICE__ float3 ks_apply_contour_split_sat(float3 lin,float satGain){
  float g=clampf(satGain,0.5f,2.0f);
  if(_fabs(g-1.0f)<=1.0e-8f)return lin;
  const float pivot_ui=0.5f,ratio=0.5f;
  float d=g-1.0f;
  float loGain=1.0f+contour_sign(d)*_fabs(d)*(1.0f-ratio);
  float hiGain=1.0f+contour_sign(d)*_fabs(d)*ratio;
  float pivot=_powf(pivot_ui,1.5747f);
  float3 orig=contour_lin_to_dvi3(lin);
  float3 in=contour_clamp01_3(orig);
  float lum=in.x*0.2126f+in.y*0.7152f+in.z*0.0722f;
  float loMask=contour_bez_mask(lum,pivot-0.165f,0.33f);
  float hiMask=1.0f-loMask;
  float3 ych=contour_rgb_to_subchen(in);
  float3 loOut=contour_subchen_to_rgb(make_float3(ych.x,ych.y*loGain,ych.z));
  float3 hiOut=contour_subchen_to_rgb(make_float3(ych.x,ych.y*(1.0f/hiGain),ych.z));
  float3 candidate=loOut*make_float3(loMask,loMask,loMask)+hiOut*make_float3(hiMask,hiMask,hiMask);
  float3 out_dvi=orig+(candidate-in);
  return contour_dvi_to_lin3(out_dvi);
}

__DEVICE__ float2 contour_eval_point(float t,float2 p0,float2 p1,float2 p0c,float2 p1c){
  float ax=2.0f*p0.x-2.0f*p1.x+(p0c.x-p0.x)+(p1.x-p1c.x);
  float bx=-3.0f*p0.x+3.0f*p1.x-2.0f*(p0c.x-p0.x)-(p1.x-p1c.x);
  float cx=p0c.x-p0.x,dx=p0.x;
  float ay=2.0f*p0.y-2.0f*p1.y+(p0c.y-p0.y)+(p1.y-p1c.y);
  float by=-3.0f*p0.y+3.0f*p1.y-2.0f*(p0c.y-p0.y)-(p1.y-p1c.y);
  float cy=p0c.y-p0.y,dy=p0.y;
  return make_float2(ax*t*t*t+bx*t*t+cx*t+dx,ay*t*t*t+by*t*t+cy*t+dy);
}
// Exact equivalent of Contour's 20-sample runtime Hermite table, evaluated procedurally.
// No baked table is stored: adjacent samples are generated from the current control points on demand.
__DEVICE__ float contour_hermite20(float x,float2 p0,float2 p1,float2 p0c,float2 p1c){
  float2 a=contour_eval_point(0.0f,p0,p1,p0c,p1c);
  for(int i=0;i<19;i++){
    float t1=(float)(i+1)/19.0f;
    float2 b=contour_eval_point(t1,p0,p1,p0c,p1c);
    if(x>=a.x&&x<b.x){float den=b.x-a.x;if(_fabs(den)<=1.0e-12f)return a.y;float q=(x-a.x)/den;return a.y+(b.y-a.y)*q;}
    a=b;
  }
  return a.y;
}

__DEVICE__ float contour_apply_split_curve(float in,float toeStrength,float shoulderStrength,float pivotToe,float pivotShoulder,float curveStart,float curveEnd,float toeDivergence,float shoulderDivergence){
  float out=in;
  float startEaseToe=0.125f,peakEaseToe=0.4f,endEaseToe=0.15f,toeCenter=0.35f;
  float startEaseShoulder=0.125f,peakEaseShoulder=0.5f,endEaseShoulder=0.3f,shoulderCenter=0.43f,peakEasePivot=0.5f;
  toeCenter=curveStart+toeCenter*(pivotToe-curveStart);
  shoulderCenter=pivotShoulder+shoulderCenter*(curveEnd-pivotShoulder);
  startEaseToe+=curveStart;endEaseToe=pivotToe-endEaseToe;
  startEaseShoulder+=pivotShoulder;endEaseShoulder=curveEnd-endEaseShoulder;
  float2 p0,p1,p0c,p1c;
  if(in<=pivotToe){
    if(in>=curveStart&&in<=toeCenter){
      p0=make_float2(curveStart,curveStart+toeStrength*toeDivergence);
      p1=make_float2(toeCenter,toeStrength+toeCenter);
      p0c=make_float2(startEaseToe,p0.y+(startEaseToe-p0.x));
      p1c=make_float2(p1.x-peakEasePivot*peakEaseToe,p1.y-peakEasePivot*peakEaseToe);
      out=contour_hermite20(in,p0,p1,p0c,p1c);
    }else if(in>toeCenter){
      p0=make_float2(toeCenter,toeStrength+toeCenter);
      p1=make_float2(pivotToe,pivotToe);
      p0c=make_float2(p0.x+(1.0f-peakEasePivot)*peakEaseToe,p0.y+(1.0f-peakEasePivot)*peakEaseToe);
      p1c=make_float2(endEaseToe,p1.y-(pivotToe-endEaseToe));
      out=contour_hermite20(in,p0,p1,p0c,p1c);
    }
  }else if(in>=pivotShoulder&&in<=curveEnd){
    if(in<=shoulderCenter){
      p0=make_float2(pivotShoulder,pivotShoulder);
      p1=make_float2(shoulderCenter,shoulderStrength+shoulderCenter);
      p0c=make_float2(startEaseShoulder,p0.y+(startEaseShoulder-p0.x));
      p1c=make_float2(p1.x-peakEasePivot*peakEaseShoulder,p1.y-peakEasePivot*peakEaseShoulder);
      out=contour_hermite20(in,p0,p1,p0c,p1c);
    }else{
      p0=make_float2(shoulderCenter,shoulderStrength+shoulderCenter);
      p1=make_float2(curveEnd,curveEnd+shoulderStrength*shoulderDivergence);
      p0c=make_float2(shoulderCenter+(1.0f-peakEasePivot)*peakEaseShoulder,p0.y+(1.0f-peakEasePivot)*peakEaseShoulder);
      p1c=make_float2(endEaseShoulder,p1.y-(p1.x-endEaseShoulder));
      out=contour_hermite20(in,p0,p1,p0c,p1c);
    }
  }
  return out;
}

__DEVICE__ float3 ks_apply_contour_split_tone(float3 lin,float amount,float shadowHueDeg,float highlightHueDeg,float balance,float subtractive){
  float strength=clampf(amount,0.0f,2.0f);
  if(strength<=1.0e-8f)return lin;
  float loHi=clampf(balance,0.0f,1.0f),sub=clampf(subtractive,0.0f,1.0f);
  float shadowHue=clampf(shadowHueDeg,0.0f,360.0f)/360.0f;
  float highlightHue=clampf(highlightHueDeg,0.0f,360.0f)/360.0f;
  const float curveStart=0.0f,curveEnd=1.0f,neutralWidthUi=0.0f,floorDiverge=0.0f,peakDiverge=0.0f;
  const float minToe=0.233f,minShoulder=0.38f;
  float pivot=contour_lin_to_dvi(0.18f); // Contour split domain is DaVinci Intermediate
  float neutralWidth=neutralWidthUi*0.33f*(curveEnd-curveStart);
  pivot=_fmaxf(pivot,curveStart+minToe);
  float maxNeutralToe=pivot-curveStart-minToe;
  float pivotToe=pivot-_fminf(neutralWidth*pivot,maxNeutralToe);
  pivot=_fminf(pivot,curveEnd-minShoulder);
  float maxNeutralShoulder=curveEnd-pivot-minShoulder;
  float pivotShoulder=pivot+_fminf(neutralWidth*(1.0f-pivot),maxNeutralShoulder);
  float maxStrength=0.1f*_fmaxf(1.0f,strength);
  float scaled=strength*maxStrength;
  float toeStrength=_fminf(scaled*(1.0f-loHi),maxStrength*0.5f);
  float shoulderStrength=_fminf(scaled*loHi,maxStrength*0.5f);

  float3 toe=contour_chen_to_rgb(make_float3(0.5f,toeStrength,-shadowHue))-make_float3(0.5f,0.5f,0.5f);
  float toeMin=_fminf(toe.x,_fminf(toe.y,toe.z));
  float toeMax=_fmaxf(toe.x,_fmaxf(toe.y,toe.z));
  if(toeMin<0.0f){toe+=make_float3(_fabs(toeMin)*(1.0f-sub),_fabs(toeMin)*(1.0f-sub),_fabs(toeMin)*(1.0f-sub));toeMax=_fmaxf(toe.x,_fmaxf(toe.y,toe.z));}
  if(toeMax>0.0f)toe-=make_float3(toeMax*sub,toeMax*sub,toeMax*sub);

  float3 shoulder=contour_chen_to_rgb(make_float3(0.5f,shoulderStrength,-highlightHue))-make_float3(0.5f,0.5f,0.5f);
  float shMin=_fminf(shoulder.x,_fminf(shoulder.y,shoulder.z));
  float shMax=_fmaxf(shoulder.x,_fmaxf(shoulder.y,shoulder.z));
  if(shMin<0.0f){shoulder+=make_float3(_fabs(shMin)*(1.0f-sub),_fabs(shMin)*(1.0f-sub),_fabs(shMin)*(1.0f-sub));shMax=_fmaxf(shoulder.x,_fmaxf(shoulder.y,shoulder.z));}
  if(shMax>0.0f)shoulder-=make_float3(shMax*sub,shMax*sub,shMax*sub);

  float3 orig=contour_lin_to_dvi3(lin);
  float3 base=contour_clamp01_3(orig);
  float3 candidate=make_float3(
    contour_apply_split_curve(base.x,toe.x,shoulder.x,pivotToe,pivotShoulder,curveStart,curveEnd,floorDiverge,peakDiverge),
    contour_apply_split_curve(base.y,toe.y,shoulder.y,pivotToe,pivotShoulder,curveStart,curveEnd,floorDiverge,peakDiverge),
    contour_apply_split_curve(base.z,toe.z,shoulder.z,pivotToe,pivotShoulder,curveStart,curveEnd,floorDiverge,peakDiverge));
  float3 out_dvi=orig+(candidate-base);
  return contour_dvi_to_lin3(out_dvi);
}

// ---- Density-domain safety gates ----
// Density transforms are defined only for positive linear-light channels.
// Fade the effect out continuously as any channel approaches the invalid domain.
__DEVICE__ float ks_density_gate(float3 rgb){
  float m=_fminf(rgb.x,_fminf(rgb.y,rgb.z));
  return smootherstep(1.0e-5f,2.0e-3f,m);
}
__DEVICE__ float ks_neutral_activity(float3 rgb){
  float mx=_fmaxf(rgb.x,_fmaxf(rgb.y,rgb.z));
  float mn=_fminf(rgb.x,_fminf(rgb.y,rgb.z));
  float denom=_fmaxf(_fabs(mx),1.0e-5f);
  float c=(mx-mn)/denom;
  return smootherstep(0.008f,0.055f,c);
}
__DEVICE__ float ks_soft_limit01(float x){
  x=clampf(x,0.0f,1.0f);
  return x/(1.0f+0.35f*x);
}

__DEVICE__ float smooth01(float t){t=clampf(t,0.0f,1.0f);return t*t*(3.0f-2.0f*t);}
__DEVICE__ float3 mix3(float3 a,float3 b,float t){return a+(b-a)*make_float3(t,t,t);}


// ---- Referent sampling note ----
// The embedded LUT is sampled at exact 33^3 lattice nodes from inside transform().
// Resolve's hidden LUT parameter object is transform-scoped, so APPLY_LUT must not be
// called from a helper function. Keystone performs the interpolation/extrapolation itself.
// Boundary-cell linear continuation is used outside [0,1] to avoid hard cube-edge clipping.

__DEVICE__ float3 awg4_to_xyz(float3 v){return mmul(AWG4_TO_XYZ_C0,AWG4_TO_XYZ_C1,AWG4_TO_XYZ_C2,v);}
__DEVICE__ float3 xyz_to_awg4(float3 v){return mmul(XYZ_TO_AWG4_C0,XYZ_TO_AWG4_C1,XYZ_TO_AWG4_C2,v);}
__DEVICE__ float3 awg3_to_xyz_legacy(float3 v){return mmul(AWG3_TO_XYZ_C0,AWG3_TO_XYZ_C1,AWG3_TO_XYZ_C2,v);}
__DEVICE__ float3 xyz_to_awg3_legacy(float3 v){return mmul(XYZ_TO_AWG3_C0,XYZ_TO_AWG3_C1,XYZ_TO_AWG3_C2,v);}

__DEVICE__ float logc4_encode_scalar(float x){
  if(x<LOGC4_T)return (x-LOGC4_T)/LOGC4_S;
  return ((_log2f(LOGC4_A*x+64.0f)-6.0f)/14.0f)*LOGC4_B+LOGC4_C;
}
__DEVICE__ float logc4_decode_scalar(float y){
  if(y<0.0f)return y*LOGC4_S+LOGC4_T;
  return (_powf(2.0f,14.0f*(y-LOGC4_C)/LOGC4_B+6.0f)-64.0f)/LOGC4_A;
}
__DEVICE__ float3 logc4_decode(float3 v){return make_float3(logc4_decode_scalar(v.x),logc4_decode_scalar(v.y),logc4_decode_scalar(v.z));}
__DEVICE__ float3 logc4_encode(float3 v){return make_float3(logc4_encode_scalar(v.x),logc4_encode_scalar(v.y),logc4_encode_scalar(v.z));}


__DEVICE__ float3 ks_mul_rows(float3 r0,float3 r1,float3 r2,float3 v){
  return make_float3(
    r0.x*v.x+r0.y*v.y+r0.z*v.z,
    r1.x*v.x+r1.y*v.y+r1.z*v.z,
    r2.x*v.x+r2.y*v.y+r2.z*v.z);
}

// OpenDRT Creative White port for Keystone.
// OpenDRT applies its CAT after its display tonescale and weights it with:
//   cwp_f = pow(tsn, 2 * CreativeWhiteLimit)
// Keystone intentionally has no ODT. We therefore keep OpenDRT's selector,
// CAT02 matrices, and power-law mix exactly, but derive tsn from scene
// luminance encoded through LogC4 and normalized from scene black to code 1.
// This keeps D65 exact identity and makes Limit act as a highlight bias.
__DEVICE__ float3 ks_apply_creative_white_xyz(float3 xyz,int cwp,float limit){
  if(cwp==2)return xyz; // D65 is exact bypass.

  float3 shifted=xyz;
  if(cwp==0)shifted=ks_mul_rows(ODRT_D65_D93_R0,ODRT_D65_D93_R1,ODRT_D65_D93_R2,xyz);
  else if(cwp==1)shifted=ks_mul_rows(ODRT_D65_D75_R0,ODRT_D65_D75_R1,ODRT_D65_D75_R2,xyz);
  else if(cwp==3)shifted=ks_mul_rows(ODRT_D65_D60_R0,ODRT_D65_D60_R1,ODRT_D65_D60_R2,xyz);
  else if(cwp==4)shifted=ks_mul_rows(ODRT_D65_D55_R0,ODRT_D65_D55_R1,ODRT_D65_D55_R2,xyz);
  else if(cwp==5)shifted=ks_mul_rows(ODRT_D65_D50_R0,ODRT_D65_D50_R1,ODRT_D65_D50_R2,xyz);

  float lm=clampf(limit,0.0f,1.0f);
  float f=1.0f;
  if(lm>1.0e-7f){
    const float black_code=LOGC4_C;
    float y=_fmaxf(xyz.y,0.0f);
    float code=logc4_encode_scalar(y);
    float tsn=clampf((code-black_code)/(1.0f-black_code),0.0f,1.0f);
    f=_powf(tsn,2.0f*lm);
  }
  return mix3(xyz,shifted,f);
}

// ---- OKLab (unchanged from Keystone v4.0.0) ----
__DEVICE__ float3 xyz_to_oklab(float3 xyz){float3 lms=mmul(X65_TO_OKLMS_C0,X65_TO_OKLMS_C1,X65_TO_OKLMS_C2,xyz);lms=make_float3(cbrt_s(lms.x),cbrt_s(lms.y),cbrt_s(lms.z));return mmul(O_C0,O_C1,O_C2,lms);}
__DEVICE__ float3 oklab_to_xyz(float3 lab){float3 lms=mmul(P_C0,P_C1,P_C2,lab);lms=make_float3(lms.x*lms.x*lms.x,lms.y*lms.y*lms.y,lms.z*lms.z*lms.z);return mmul(OKLMS3_TO_X65_C0,OKLMS3_TO_X65_C1,OKLMS3_TO_X65_C2,lms);}
__DEVICE__ float3 oklab_to_xyz_preserve_y_safe(float3 lab,float targetY){
  float3 out=oklab_to_xyz(lab);
  if(targetY<=1.0e-20f)return out;
  if(out.y<=1.0e-10f){
    float lo=0.0f,hi=1.0f;
    for(int i=0;i<20;i++){
      float mid=(lo+hi)*0.5f;
      float3 test=make_float3(lab.x,lab.y*mid,lab.z*mid);
      float y=oklab_to_xyz(test).y;
      if(y>targetY*1.0e-8f)lo=mid;else hi=mid;
    }
    lab.y*=lo;lab.z*=lo;out=oklab_to_xyz(lab);
  }
  if(out.y<=1.0e-20f)return make_float3(WHITE_D65.x*targetY,targetY,WHITE_D65.z*targetY);
  float k=targetY/out.y;
  return out*make_float3(k,k,k);
}

// ---- Keystone v4.3.18 normalized opponent Temp/Tint white balance ----
// Pipeline:
//   LogC4 -> linear AWG4 -> normalized opponent Temp/Tint balance
//   -> luminance normalization -> neutral-axis preservation -> rest of pipeline
//
// Temp and Tint are built as orthogonal vectors in log-gain space. Both vectors
// are also orthogonal to the neutral [1,1,1] axis, which keeps the controls
// chromatic by construction. The resulting gains are normalized so a neutral
// AWG4 sample retains unit scene luminance. A second per-pixel normalization
// restores each pixel's original XYZ Y, then OKLab L is restored to the input
// value while retaining the corrected opponent chroma. Creative White is not
// part of this WB operation and is applied later in the finish stage.
__DEVICE__ float3 ks_wb_opponent_gains(float temp_value,float tint_value){
  float t=clampf(temp_value/100.0f,-1.0f,1.0f)*WB_TEMP_MAX_STOPS;
  float m=clampf(tint_value/100.0f,-1.0f,1.0f)*WB_TINT_MAX_STOPS;

  float3 log_gain=make_float3(
    t*WB_TEMP_AXIS_R + m*WB_TINT_AXIS_R,
    t*WB_TEMP_AXIS_G + m*WB_TINT_AXIS_G,
    t*WB_TEMP_AXIS_B + m*WB_TINT_AXIS_B);

  float3 gains=make_float3(_powf(2.0f,log_gain.x),_powf(2.0f,log_gain.y),_powf(2.0f,log_gain.z));

  // Neutral-sample luminance normalization. Because AWG4 [1,1,1] maps to Y=1,
  // the Y of the gain vector is exactly the neutral luminance change introduced
  // by the opponent gains. Divide it out before touching image pixels.
  float neutral_y=awg4_to_xyz(gains).y;
  if(neutral_y>1.0e-8f){
    float k=1.0f/neutral_y;
    gains*=make_float3(k,k,k);
  }
  return gains;
}

__DEVICE__ float3 ks_wb_neutral_axis_preserve_xyz(float3 input_xyz,float3 balanced_xyz){
  // OKLab L is used only as the neutral-axis/lightness carrier. Keep the
  // opponent chroma generated by Temp/Tint, but restore the input L. Then
  // convert back while preserving the original scene Y.
  float target_y=input_xyz.y;
  float3 in_lab=xyz_to_oklab(input_xyz);
  float3 out_lab=xyz_to_oklab(balanced_xyz);
  out_lab.x=in_lab.x;
  return oklab_to_xyz_preserve_y_safe(out_lab,target_y);
}

__DEVICE__ float3 ks_apply_opponent_wb_awg4(float3 lin,float temp_value,float tint_value){
  if(_fabs(temp_value)<=1.0e-7f&&_fabs(tint_value)<=1.0e-7f)return lin;

  float3 input_xyz=awg4_to_xyz(lin);
  float input_y=input_xyz.y;
  float3 gains=ks_wb_opponent_gains(temp_value,tint_value);
  float3 balanced=lin*gains;
  float3 balanced_xyz=awg4_to_xyz(balanced);

  // Per-pixel luminance normalization: Temp/Tint should change balance, not
  // behave like an exposure control. Preserve sign so extended-range values
  // are not forcibly clipped or folded.
  if(_fabs(input_y)>1.0e-10f&&_fabs(balanced_xyz.y)>1.0e-10f&&input_y*balanced_xyz.y>0.0f){
    float k=input_y/balanced_xyz.y;
    balanced_xyz*=make_float3(k,k,k);
  }

  balanced_xyz=ks_wb_neutral_axis_preserve_xyz(input_xyz,balanced_xyz);
  return xyz_to_awg4(balanced_xyz);
}

// ---- HOYA PRO ND filter-factor family ----
// HOYA publishes ND2/4/8/... as filter factors. Neutral transmission = 1 / factor.
// This is an equal-channel exposure model; no spectral neutrality claim is made.
__DEVICE__ float ks_hoya_nd_transmittance(int p){
  if(p==1)return 0.500000000f; // ND2
  if(p==2)return 0.250000000f; // ND4
  if(p==3)return 0.125000000f; // ND8
  if(p==4)return 0.062500000f; // ND16
  if(p==5)return 0.031250000f; // ND32
  if(p==6)return 0.015625000f; // ND64
  if(p==7)return 0.010000000f; // ND100
  if(p==8)return 0.005000000f; // ND200
  if(p==9)return 0.002500000f; // ND400
  if(p==10)return 0.002000000f; // ND500
  if(p==11)return 0.001000000f; // ND1000
  return 1.0f;
}
__DEVICE__ float3 ks_apply_hoya_nd(float3 rgb,int p){float t=ks_hoya_nd_transmittance(p);return rgb*make_float3(t,t,t);}

// ---- Primera v0.6.0 scalar tone primitives; v4.1 drives them from LogC4-encoded scene luminance ----
__DEVICE__ float primera_soft_black_point(float x,float bp,float knee){
  if(x<=0.0f)return x;
  float corrected=x-bp;
  if(corrected>=knee)return corrected;
  return knee*_expf(corrected/knee-1.0f);
}
__DEVICE__ float primera_shadow_fill(float x,float gain,float mid){
  if(x<=0.0f)return x;
  if(x>=mid)return x;
  float t=x/mid;
  float blend=t*t*(3.0f-2.0f*t);
  return x*(gain+(1.0f-gain)*blend);
}
__DEVICE__ float primera_highlight_gain(float x,float gain,float mid){
  if(x<=mid)return x;
  if(x>=1.0f)return x;
  float t=(x-mid)/(1.0f-mid);
  float blend=t*t*(3.0f-2.0f*t);
  return x*(1.0f+(gain-1.0f)*blend);
}
// Algebraic tanh using _expf because _expf is already used by the working Keystone Metal path.
__DEVICE__ float ks_tanh_positive(float x){
  float z=clampf(x,0.0f,20.0f);
  float e=_expf(-2.0f*z);
  return (1.0f-e)/(1.0f+e);
}
__DEVICE__ float primera_soft_highlight(float x,float amount,float mid){
  if(x<=0.0f)return x;
  float knee_amount=_fminf(amount,1.0f);
  float strength=1.0f+_fmaxf(amount-1.0f,0.0f)*2.0f;
  float knee=1.0f-knee_amount*(1.0f-mid);
  if(x<=knee)return x;
  float range=1.0f-knee;
  float t=(x-knee)/range*strength;
  return knee+range*ks_tanh_positive(t)/strength;
}
__DEVICE__ float primera_rolling_contrast(float x,float contrast,float pivot){
  if(x<=0.0f)return x;
  if(x>=1.0f)return x;
  if(x<=pivot)return _powf(x/pivot,contrast)*pivot;
  float inv=1.0f-x;
  float inv_pivot=1.0f-pivot;
  return 1.0f-_powf(inv/inv_pivot,contrast)*inv_pivot;
}
// ---- Keystone v4.1 luminance-carrier tone wrapper ----
// Keep the Primera scalar response shapes, but drive them from scene Y and rescale
// AWG4 RGB uniformly. This decouples tone shaping from hue/chroma shaping.
__DEVICE__ float3 ks_scale_awg4_to_target_y(float3 rgb,float targetY){
  float Y=awg4_to_xyz(rgb).y;
  if(!finitef(Y)||!finitef(targetY)||_fabs(Y)<=1.0e-12f)return rgb;
  float k=targetY/Y;
  if(!finitef(k))return rgb;
  return rgb*make_float3(k,k,k);
}
__DEVICE__ float3 primera_black_luma(float3 rgb,float amount){
  if(_fabs(amount)<=1.0e-9f)return rgb;
  float Y=awg4_to_xyz(rgb).y;
  if(Y<=0.0f)return rgb;
  float targetY=primera_soft_black_point(Y,-amount,0.005f);
  return ks_scale_awg4_to_target_y(rgb,targetY);
}
__DEVICE__ float3 primera_tone_luma(float3 rgb,float contrast,float shadows,float highlights,float roll){
  if(_fabs(contrast-1.0f)<=1.0e-8f&&_fabs(shadows)<=1.0e-8f&&_fabs(highlights)<=1.0e-8f&&roll<=1.0e-8f)return rgb;
  float Y=awg4_to_xyz(rgb).y;
  if(Y<=0.0f)return rgb;
  float y=logc4_encode_scalar(Y);
  float mid=logc4_encode_scalar(0.18f);
  if(_fabs(contrast-1.0f)>1.0e-8f)y=primera_rolling_contrast(y,contrast,mid);
  if(_fabs(shadows)>1.0e-8f)y=primera_shadow_fill(y,_powf(2.0f,shadows),mid);
  if(_fabs(highlights)>1.0e-8f)y=primera_highlight_gain(y,_powf(2.0f,highlights),mid);
  if(roll>1.0e-8f)y=primera_soft_highlight(y,roll,mid);
  return ks_scale_awg4_to_target_y(rgb,logc4_decode_scalar(y));
}

// ---- Characteristic curve presets ----
// Speak source: exact MIT-licensed H&D closed form and the source preset parameters.
// Keystone applies the scalar result to scene Y and uniformly rescales AWG4 RGB,
// so a curve changes tone without importing per-channel color behavior.
__DEVICE__ float ks_softplus(float z){
  float az=_fabs(z);
  return (z>0.0f?z:0.0f)+_logf(1.0f+_expf(-az));
}
__DEVICE__ float ks_hd_curve(float logH,float Dmin,float Dmax,float gamma,float toe,float shoulder,float speed){
  float t=_fmaxf(toe,0.05f),s=_fmaxf(shoulder,0.05f);
  float d1=Dmin+(gamma/t)*ks_softplus(t*(logH-speed));
  return Dmax-(1.0f/s)*ks_softplus(s*(Dmax-d1));
}
__DEVICE__ float ks_curve_density(float stops,int p){
  float nDmin=0.15f,nDmax=2.90f,nGamma=0.62f,nToe=2.5f,nShoulder=2.5f,nSpeed=-1.5f;
  float pDmin=0.10f,pDmax=3.30f,pGamma=2.60f,pToe=3.5f,pShoulder=2.2f,pSpeed=-1.75f;
  if(p==2){nDmax=3.10f;nGamma=0.58f;nToe=2.0f;nShoulder=1.6f;nSpeed=-1.8f;pGamma=2.65f;pToe=3.0f;pShoulder=1.9f;}
  else if(p==3){nGamma=0.69f;nToe=2.8f;nShoulder=2.6f;pDmax=3.40f;pGamma=2.70f;pToe=5.0f;pShoulder=2.6f;}
  else if(p==4){nGamma=0.70f;nToe=3.0f;nShoulder=3.2f;pDmax=3.60f;pGamma=2.78f;pToe=6.0f;pShoulder=3.4f;pSpeed=-1.65f;}
  float logH=stops*KS_LOG10_2;
  float dn=ks_hd_curve(logH,nDmin,nDmax,nGamma,nToe,nShoulder,nSpeed);
  return ks_hd_curve(-dn,pDmin,pDmax,pGamma,pToe,pShoulder,pSpeed);
}
__DEVICE__ float3 ks_apply_curve_preset(float3 rgb,int p,float amount){
  float a=clampf(amount,0.0f,1.0f);if(p<=0||a<=1.0e-8f)return rgb;
  float Y=awg4_to_xyz(rgb).y;if(Y<=1.0e-10f)return rgb;
  float stops=_log2f(Y/0.18f);
  float d=ks_curve_density(stops,p),d0=ks_curve_density(0.0f,p);
  float target=0.18f*_powf(10.0f,-(d-d0));
  if(target<=1.0e-12f)return rgb;
  float outStops=_log2f(target/0.18f);
  float mixedStops=stops+(outStops-stops)*a;
  float mixedY=0.18f*_powf(2.0f,mixedStops);
  return ks_scale_awg4_to_target_y(rgb,mixedY);
}

// ---- Cone-coordinate color density ----
// Behavioral reimplementation of the dense-color principle used by Moaz Elgabry's
// historical ME_Desatch.dctl: chromatic distance from the achromatic RGB axis drives
// a reduction in radial/code-value magnitude, so stronger color becomes darker rather
// than moving toward a washed-out gray. This is intentionally a single global control.
//
// We only need the cone angle for the global density behavior. The full hue-warped
// coordinate transform used for selective RGB/CMY DeSatch is unnecessary here because
// hue and angular saturation stay fixed; scaling the radial term is equivalent to a
// uniform scale of the encoded RGB triplet.
__DEVICE__ float3 ks_apply_cone_density(float3 enc,float amount){
  float a=clampf(amount,0.0f,1.0f);
  if(a<=1.0e-7f)return enc;

  // Orthonormal decomposition around the achromatic (1,1,1) axis.
  const float INV_SQRT6=0.4082482904638631f;
  const float INV_SQRT2=0.7071067811865475f;
  const float INV_SQRT3=0.5773502691896258f;
  const float PRIMARY_CONE_ANGLE=0.9553166181245093f; // acos(1/sqrt(3))

  float u=(2.0f*enc.x-enc.y-enc.z)*INV_SQRT6;
  float v=(enc.y-enc.z)*INV_SQRT2;
  float n=(enc.x+enc.y+enc.z)*INV_SQRT3;
  float chroma_radius=_sqrtf(_fmaxf(u*u+v*v,0.0f));

  // For non-negative RGB, PRIMARY_CONE_ANGLE maps a pure primary to 1.0.
  // Clamp out-of-gamut/negative excursions so density can never flip RGB sign.
  float phi=_atan2f(chroma_radius,n);
  float chroma_angle=clampf(phi/PRIMARY_CONE_ANGLE,0.0f,1.0f);
  float scale=1.0f-a*chroma_angle;

  return enc*make_float3(scale,scale,scale);
}

// ---- Primera v0.6.0 positive saturation functional form, ported to LogC4 ----
__DEVICE__ float density10(float lin){return -_log2f(_fmaxf(lin,1.0e-6f))*KS_LOG10_2;}
__DEVICE__ float3 ks_rgb_to_hsv(float3 rgb){
  float r=rgb.x,g=rgb.y,b=rgb.z;
  float cmax=_fmaxf(r,_fmaxf(g,b)),cmin=_fminf(r,_fminf(g,b)),delta=cmax-cmin,h=0.0f;
  if(delta>0.0f){
    if(cmax==r)h=(g-b)/delta;
    else if(cmax==g)h=(b-r)/delta+2.0f;
    else h=(r-g)/delta+4.0f;
    if(h<0.0f)h+=6.0f;
    h/=6.0f;
  }
  return make_float3(h,(cmax>0.0f)?delta/cmax:0.0f,cmax);
}
__DEVICE__ float3 ks_hsv_to_rgb(float3 hsv){
  float h=hsv.x*6.0f,s=hsv.y,v=hsv.z;
  float c=v*s,x=c*(1.0f-_fabs(h-2.0f*_floorf(h*0.5f)-1.0f)),m=v-c,r,g,b;
  if(h<1.0f){r=c;g=x;b=0.0f;}
  else if(h<2.0f){r=x;g=c;b=0.0f;}
  else if(h<3.0f){r=0.0f;g=c;b=x;}
  else if(h<4.0f){r=0.0f;g=x;b=c;}
  else if(h<5.0f){r=x;g=0.0f;b=c;}
  else{r=c;g=0.0f;b=x;}
  return make_float3(r+m,g+m,b+m);
}
__DEVICE__ float3 apply_pos_sat_filmic(float3 rgb,float amount){
  float a=clampf(amount,0.0f,1.0f);
  if(a<=1.0e-8f)return rgb;
  float gate=ks_density_gate(rgb)*ks_neutral_activity(rgb);
  if(gate<=1.0e-8f)return rgb;

  // Optical-density model: D = -log10(T).  The minimum channel density is the
  // neutral/common absorption shared by all three channels.  Scale only the
  // chromatic density above that floor.  The dominant RGB channel therefore
  // does not get brighter as saturation rises; weaker channels absorb more.
  float3 d=make_float3(-_log10f(rgb.x),-_log10f(rgb.y),-_log10f(rgb.z));
  float neutral=_fminf(d.x,_fminf(d.y,d.z));
  float3 chroma=d-make_float3(neutral,neutral,neutral);

  // One slider: full scale doubles chromatic dye density.  Smooth parameter
  // response keeps the lower half useful for normal grading.
  float gain=1.0f+smooth01(a);
  float3 dout=make_float3(neutral,neutral,neutral)+chroma*make_float3(gain,gain,gain);
  float3 candidate=make_float3(_powf(10.0f,-dout.x),_powf(10.0f,-dout.y),_powf(10.0f,-dout.z));

  // Fade continuously near neutrals / invalid density domain.
  return rgb+(candidate-rgb)*make_float3(gate,gate,gate);
}

// ---- Genesis 5207 strength-dependent interlayer term ----
__DEVICE__ float3 ks_genesis_5207_iie_poly(float3 v,float strength){
  float R=v.x,G=v.y,B=v.z;
  float RR=R*R,RG=R*G,RB=R*B,GG=G*G,GB=G*B,BB=B*B;
  float s=strength;
  return make_float3(
    R*GEN_5207_IIE_R0_0+G*GEN_5207_IIE_R0_1+B*GEN_5207_IIE_R0_2+
    s*(RR*GEN_5207_IIE_R0_3+GG*GEN_5207_IIE_R0_4+BB*GEN_5207_IIE_R0_5+RG*GEN_5207_IIE_R0_6+RB*GEN_5207_IIE_R0_7+GB*GEN_5207_IIE_R0_8)+GEN_5207_IIE_R0_9,
    R*GEN_5207_IIE_R1_0+G*GEN_5207_IIE_R1_1+B*GEN_5207_IIE_R1_2+
    s*(RR*GEN_5207_IIE_R1_3+GG*GEN_5207_IIE_R1_4+BB*GEN_5207_IIE_R1_5+RG*GEN_5207_IIE_R1_6+RB*GEN_5207_IIE_R1_7+GB*GEN_5207_IIE_R1_8)+GEN_5207_IIE_R1_9,
    R*GEN_5207_IIE_R2_0+G*GEN_5207_IIE_R2_1+B*GEN_5207_IIE_R2_2+
    s*(RR*GEN_5207_IIE_R2_3+GG*GEN_5207_IIE_R2_4+BB*GEN_5207_IIE_R2_5+RG*GEN_5207_IIE_R2_6+RB*GEN_5207_IIE_R2_7+GB*GEN_5207_IIE_R2_8)+GEN_5207_IIE_R2_9);
}

__DEVICE__ float3 ks_apply_genesis_interlayer(float3 rgb,float amount){
  float a=clampf(amount,0.0f,3.0f);
  if(a<=1.0e-8f)return rgb;
  float gate=ks_density_gate(rgb)*ks_neutral_activity(rgb);
  if(gate<=1.0e-8f)return rgb;

  // Build scale-invariant chromatic optical density. The common density floor
  // carries exposure; only channel-to-channel dye-density differences are fed
  // to the Genesis IIE term, so neutral RGB is an exact fixed axis.
  float3 d=make_float3(-_log10f(rgb.x),-_log10f(rgb.y),-_log10f(rgb.z));
  float neutral=_fminf(d.x,_fminf(d.y,d.z));
  float3 chroma=d-make_float3(neutral,neutral,neutral);

  // Genesis's UI strength changes only the quadratic columns of the 3x10
  // matrix. Subtracting strength=0 isolates that exact user-controlled term
  // without importing Genesis's fixed stock transform.
  float3 base=ks_genesis_5207_iie_poly(chroma,0.0f);
  float3 active=ks_genesis_5207_iie_poly(chroma,a);
  float3 out_chroma=chroma+(active-base);
  float3 dout=make_float3(neutral,neutral,neutral)+out_chroma;
  float3 candidate=make_float3(_powf(10.0f,-dout.x),_powf(10.0f,-dout.y),_powf(10.0f,-dout.z));

  if(!finitef(candidate.x)||!finitef(candidate.y)||!finitef(candidate.z))return rgb;
  return rgb+(candidate-rgb)*make_float3(gate,gate,gate);
}

// ---- Keystone v4.1 look-development safeguards ----
// Inspired by the interview's separation of highlight bleach from tone rendering and
// gamut-bending workflow ideas. These are Keystone-authored operators; no Baselight
// Chromogen/XGrade proprietary math is copied.
__DEVICE__ float3 ks_soft_gamut_bend(float3 rgb){
  float Y=awg4_to_xyz(rgb).y;
  if(Y<=1.0e-12f)return rgb;
  float3 enc=logc4_encode(rgb);
  float mn=_fminf(enc.x,_fminf(enc.y,enc.z));
  // Ordinary extended values pass unchanged. Compression ramps in only as the
  // most-negative encoded channel moves from -0.25 toward the final -1.0 guard.
  float stress=1.0f-smootherstep(-0.75f,-0.25f,mn);
  if(stress<=1.0e-7f)return rgb;
  float3 xyz=awg4_to_xyz(rgb);
  float3 lab=xyz_to_oklab(xyz);
  float C=_sqrtf(lab.y*lab.y+lab.z*lab.z);
  if(C<=1.0e-10f)return rgb;
  float scale=1.0f/(1.0f+1.50f*stress);
  lab.y*=scale;lab.z*=scale;
  return xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
}
__DEVICE__ float3 ks_highlight_bleach(float3 rgb,float amount){
  float a=clampf(amount,0.0f,1.0f);
  if(a<=1.0e-8f)return rgb;
  float3 xyz=awg4_to_xyz(rgb);
  float Y=xyz.y;
  if(Y<=1.0e-12f)return rgb;
  float ev=_log2f(_fmaxf(Y,1.0e-10f)/0.18f);
  // Starts gently above key and reaches the full requested amount high in the scale.
  float gate=smootherstep(1.50f,4.50f,ev);
  if(gate<=1.0e-8f)return rgb;
  float3 lab=xyz_to_oklab(xyz);
  float keep=1.0f-smooth01(a)*gate;
  lab.y*=keep;lab.z*=keep;
  return xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
}
__DEVICE__ float3 ks_shadow_bleach(float3 rgb,float amount){
  float a=clampf(amount,0.0f,1.0f);
  if(a<=1.0e-8f)return rgb;
  float3 xyz=awg4_to_xyz(rgb);
  float Y=xyz.y;
  if(Y<=1.0e-12f)return rgb;
  float ev=_log2f(_fmaxf(Y,1.0e-10f)/0.18f);
  // Mirror of Hi Bleach around 18 percent gray: no effect above -1.5 EV,
  // progressively stronger below it, full requested amount by -4.5 EV.
  float gate=1.0f-smootherstep(-4.50f,-1.50f,ev);
  if(gate<=1.0e-8f)return rgb;
  float3 lab=xyz_to_oklab(xyz);
  float keep=1.0f-smooth01(a)*gate;
  lab.y*=keep;lab.z*=keep;
  return xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
}

// ---- Curated color-look presets ----
// Looks 1-6 preserve selected legacy Keystone v2.7 density-tone presets inside an AWG3 bridge.
// Looks 7-9 are new Keystone-authored Oklab split-tone looks chosen to be distinct and useful.
__DEVICE__ float3 ks_legacy_look_shadow(int p){
  if(p==1)return make_float3(0.08863f,-0.02323f,-0.06541f); // Warm Cool
  if(p==2)return make_float3(-0.06268f,0.02009f,0.04259f); // Cool Warm
  if(p==6)return make_float3(0.09741f,-0.05113f,-0.04628f); // Teal Orange
  if(p==11)return make_float3(-0.03306f,0.01196f,0.02110f); // Sepia
  if(p==12)return make_float3(0.04640f,-0.01395f,-0.03245f); // Bleach Cool
  if(p==14)return make_float3(0.07062f,-0.01234f,-0.05828f); // Dusk Purple
  float3 fallback=make_float3(0.0f,0.0f,0.0f);
  return fallback;
}
__DEVICE__ float3 ks_legacy_look_high(int p){
  if(p==1)return make_float3(-0.06768f,0.01133f,0.05634f);
  if(p==2)return make_float3(0.08370f,-0.02188f,-0.06182f);
  if(p==6)return make_float3(-0.08350f,0.02334f,0.06017f);
  if(p==11)return make_float3(-0.07740f,0.01721f,0.06019f);
  if(p==12)return make_float3(0.02638f,-0.00909f,-0.01729f);
  if(p==14)return make_float3(0.00313f,0.01911f,-0.02224f);
  float3 fallback=make_float3(0.0f,0.0f,0.0f);
  return fallback;
}
__DEVICE__ int ks_look_remap(int p){
  if(p==1)return 1;   // Warm Cool
  if(p==2)return 2;   // Cool Warm
  if(p==3)return 6;   // Teal Orange
  if(p==4)return 11;  // Sepia
  if(p==5)return 12;  // Bleach Cool
  if(p==6)return 14;  // Dusk Purple
  return 0;
}
__DEVICE__ float3 ks_apply_legacy_color_look(float3 awg4,int p,float amount){
  float a=clampf(amount,0.0f,1.5f);int legacy=ks_look_remap(p);if(legacy<=0||a<=1.0e-8f)return awg4;
  float3 awg3=xyz_to_awg3_legacy(awg4_to_xyz(awg4));
  float gate=ks_density_gate(awg3);if(gate<=1.0e-8f)return awg4;
  float DR=density10(awg3.x),DG=density10(awg3.y),DB=density10(awg3.z);
  float Dbar=(DR+DG+DB)*(1.0f/3.0f);
  float x=(Dbar-KS_GRAY_D)/(0.25f+1.5f*0.5f); // legacy pivot=0, width=0.5
  float wS=smooth01(x),wH=smooth01(-x);
  float3 sh=ks_legacy_look_shadow(legacy)*make_float3(a,a,a);
  float3 hi=ks_legacy_look_high(legacy)*make_float3(a,a,a);
  float3 candidate=make_float3(
    _powf(10.0f,-(DR+wS*sh.x+wH*hi.x)),
    _powf(10.0f,-(DG+wS*sh.y+wH*hi.y)),
    _powf(10.0f,-(DB+wS*sh.z+wH*hi.z)));
  awg3=awg3+(candidate-awg3)*make_float3(gate,gate,gate);
  return xyz_to_awg4(awg3_to_xyz_legacy(awg3));
}
__DEVICE__ float2 ks_authored_look_shadow_ab(int p){
  if(p==7)return make_float2(-0.0340f,-0.0100f); // Cyan Rose: cyan/teal shadows
  if(p==8)return make_float2(-0.0180f, 0.0180f); // Green Gold: olive/green shadows
  if(p==9)return make_float2( 0.0170f,-0.0310f); // Purple Gold: violet shadows
  return make_float2(0.0f,0.0f);
}
__DEVICE__ float2 ks_authored_look_high_ab(int p){
  if(p==7)return make_float2( 0.0260f,-0.0060f); // rose highlights
  if(p==8)return make_float2( 0.0100f, 0.0310f); // gold highlights
  if(p==9)return make_float2( 0.0070f, 0.0290f); // gold highlights
  return make_float2(0.0f,0.0f);
}
__DEVICE__ float3 ks_apply_authored_color_look(float3 awg4,int p,float amount){
  float a=clampf(amount,0.0f,1.5f);
  if(p<7||p>9||a<=1.0e-8f)return awg4;
  float gate=ks_density_gate(awg4);if(gate<=1.0e-8f)return awg4;
  float3 xyz=awg4_to_xyz(awg4);
  float3 lab=xyz_to_oklab(xyz);
  float C=_sqrtf(lab.y*lab.y+lab.z*lab.z);
  float sat_gate=1.0f-smootherstep(0.10f,0.28f,C);
  float DR=density10(awg4.x),DG=density10(awg4.y),DB=density10(awg4.z);
  float Dbar=(DR+DG+DB)*(1.0f/3.0f);
  float x=(Dbar-KS_GRAY_D)/(0.25f+1.5f*0.5f);
  float wS=smooth01(x),wH=smooth01(-x);
  float w=gate*(0.25f+0.75f*sat_gate);
  float2 sh=ks_authored_look_shadow_ab(p);
  float2 hi=ks_authored_look_high_ab(p);
  lab.y+=a*w*(wS*sh.x+wH*hi.x);
  lab.z+=a*w*(wS*sh.y+wH*hi.y);
  return xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,xyz.y));
}
// ---- CHI DRX behavioral translation ----
// Source: supplied Resolve 17 grade, node order Offset -> Contrast -> Saturation -> LGG.
// Source values recovered from the DRX include Contrast 1.208 and base saturation 0.736.
// Resolve primary-wheel math is project/timeline-state dependent, so the RGB wheel values
// are translated perceptually into the scene-referred AWG4/LogC4 architecture rather than
// copied as literal linear RGB offsets. The design intent is preserved:
// warm global bias -> contrast -> desaturate -> cool shadows / warm mids-highs.
__DEVICE__ float3 ks_chi_chroma_scale(float3 awg4,float scale){
  float3 xyz=awg4_to_xyz(awg4);
  float Y=xyz.y;
  if(Y<=1.0e-12f)return awg4;
  float3 lab=xyz_to_oklab(xyz);
  float k=clampf(scale,0.0f,2.0f);
  lab.y*=k;
  lab.z*=k;
  return xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
}
__DEVICE__ float3 ks_apply_chi_drx_look(float3 awg4,float amount){
  float a=clampf(amount,0.0f,1.5f);
  if(a<=1.0e-8f)return awg4;

  float3 out=awg4;

  // 1) Offset node: strong warm/yellow base bias. Preserve scene luminance while
  // reproducing the original channel relationship instead of injecting exposure.
  float3 xyz=awg4_to_xyz(out);
  float Y=xyz.y;
  if(Y>1.0e-12f){
    float3 lab=xyz_to_oklab(xyz);
    lab.y+=0.0045f*a;  // subtle red component
    lab.z+=0.0180f*a;  // dominant yellow/warm component
    out=xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
  }

  // 2) Contrast node. At Amount 1 this lands on the source grade's 1.208 value.
  out=primera_tone_luma(out,1.0f+0.208f*a,0.0f,0.0f,0.0f);

  // 3) Saturation node. Source DRX stores 0.736 with 1.0 as neutral.
  // Oklab chroma scaling keeps this reduction perceptual and luminance-stable.
  float sat_scale=clampf(1.0f-0.264f*a,0.45f,1.0f);
  out=ks_chi_chroma_scale(out,sat_scale);

  // 4) LGG node translated as luminance-selective perceptual color separation.
  // Lift restores relatively more blue in the low end; Gamma/Gain remove more blue
  // through the mids/highs. The global warm bias above supplies the warm midrange.
  xyz=awg4_to_xyz(out);
  Y=xyz.y;
  if(Y>1.0e-12f){
    float3 lab=xyz_to_oklab(xyz);
    float stops=_log2f(_fmaxf(Y,1.0e-8f)/0.18f);
    float wS=1.0f-smootherstep(-2.0f,0.0f,stops);
    float wH=smootherstep(-0.5f,2.0f,stops);
    float C=_sqrtf(lab.y*lab.y+lab.z*lab.z);
    float chroma_protect=1.0f-0.35f*smootherstep(0.12f,0.30f,C);

    // Cool/cyan-blue shadow restoration from Lift relationship.
    float2 sh=make_float2(-0.0060f,-0.0220f);
    // Additional red/yellow warmth from Gamma/Gain relationship.
    float2 hi=make_float2(0.0040f,0.0120f);

    lab.y+=a*chroma_protect*(wS*sh.x+wH*hi.x);
    lab.z+=a*chroma_protect*(wS*sh.y+wH*hi.y);
    out=xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
  }

  return ks_soft_gamut_bend(out);
}
// ---- Island look: subtract first, then selectively rebuild tropical color ----
// Uses the same design principle as CHI without copying CHI's palette:
// modest contrast -> restrained base chroma -> selective tropical hue recovery ->
// cyan/aqua shade and warm golden mids/highs. Neutral/low-chroma pixels are protected.
__DEVICE__ float ks_hue_gauss(float h,float center,float width){
  float d=_fabs(h-center);
  d=_fminf(d,KS_TAU-d);
  float x=d/_fmaxf(width,1.0e-5f);
  return _expf(-2.0f*x*x);
}
__DEVICE__ float3 ks_apply_island_look(float3 awg4,float amount){
  float a=clampf(amount,0.0f,1.5f);
  if(a<=1.0e-8f)return awg4;

  float3 out=awg4;

  // 1) Open, sunlit contrast: enough separation for bright exterior scenes,
  // deliberately gentler than CHI so beaches/clouds do not get crunchy.
  out=primera_tone_luma(out,1.0f+0.105f*a,0.0f,0.0f,0.0f);

  // 2) Pull generic scene chroma down first. This prevents the preset from
  // becoming a blanket saturation boost and leaves room to rebuild chosen hues.
  float base_scale=clampf(1.0f-0.115f*a,0.62f,1.0f);
  out=ks_chi_chroma_scale(out,base_scale);

  // 3) Selectively recover tropical colors in Oklab while preserving Y.
  // Aqua/cyan + foliage receive the strongest recovery; warm skin/sunlight is
  // recovered more gently so faces do not become orange.
  float3 xyz=awg4_to_xyz(out);
  float Y=xyz.y;
  if(Y>1.0e-12f){
    float3 lab=xyz_to_oklab(xyz);
    float C=_sqrtf(lab.y*lab.y+lab.z*lab.z);
    if(C>1.0e-7f){
      float h=_atan2f(lab.z,lab.y);if(h<0.0f)h+=KS_TAU;
      // Oklab approximate centers: orange/yellow ~70-100 deg, green ~145 deg,
      // aqua/cyan ~205 deg. Wide lobes keep transitions photographic.
      float aqua=ks_hue_gauss(h,3.58f,0.58f);
      float green=ks_hue_gauss(h,2.48f,0.55f);
      float sun=ks_hue_gauss(h,1.35f,0.62f);
      float chroma_gate=smootherstep(0.018f,0.075f,C);
      float recover=a*chroma_gate*(0.30f*aqua+0.22f*green+0.13f*sun);
      float k=1.0f+recover;
      lab.y*=k;lab.z*=k;
      out=xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
    }
  }

  // 4) Tonal color rebuild: turquoise/cyan in shade, neutral-warm center,
  // soft gold in the upper range. Chroma protection reduces contamination
  // in already-strong colors and keeps the look from becoming two-tone.
  xyz=awg4_to_xyz(out);Y=xyz.y;
  if(Y>1.0e-12f){
    float3 lab=xyz_to_oklab(xyz);
    float stops=_log2f(_fmaxf(Y,1.0e-8f)/0.18f);
    float wS=1.0f-smootherstep(-2.2f,-0.15f,stops);
    float wH=smootherstep(-0.15f,2.25f,stops);
    float C=_sqrtf(lab.y*lab.y+lab.z*lab.z);
    float neutral_gate=1.0f-smootherstep(0.14f,0.32f,C);

    // Negative a + negative b = aqua/cyan. Positive a + b = warm gold.
    float2 sh=make_float2(-0.0100f,-0.0155f);
    float2 hi=make_float2( 0.0055f, 0.0150f);
    float tone_strength=a*(0.45f+0.55f*neutral_gate);
    lab.y+=tone_strength*(wS*sh.x+wH*hi.x);
    lab.z+=tone_strength*(wS*sh.y+wH*hi.y);
    out=xyz_to_awg4(oklab_to_xyz_preserve_y_safe(lab,Y));
  }

  return ks_soft_gamut_bend(out);
}

__DEVICE__ float3 ks_apply_color_look_base(float3 awg4,int p,float amount){
  if(p==10)return ks_apply_chi_drx_look(awg4,amount);
  if(p==11)return ks_apply_island_look(awg4,amount);
  int legacy=ks_look_remap(p);
  if(legacy>0)return ks_apply_legacy_color_look(awg4,p,amount);
  return ks_apply_authored_color_look(awg4,p,amount);
}

// ---- Recovered ToneLab display-referred Basic operators ----
// Source path traced through ToneLab.ofx OpenCL/WGSL processing assets.
// ToneLab's Basic block runs after its conversion stage. Keystone bridges only
// this block AWG4 linear -> XYZ D65 -> Rec.709 linear -> gamma 2.4, then back.
__DEVICE__ float tl_sign_pow(float x,float p){
  if(x==0.0f)return 0.0f;
  return (x<0.0f?-1.0f:1.0f)*_powf(_fabs(x),p);
}
__DEVICE__ float3 tl_rec709_gamma24_encode(float3 lin){
  const float inv=0.4166666666666667f;
  return make_float3(tl_sign_pow(lin.x,inv),tl_sign_pow(lin.y,inv),tl_sign_pow(lin.z,inv));
}
__DEVICE__ float3 tl_rec709_gamma24_decode(float3 enc){
  return make_float3(tl_sign_pow(enc.x,2.4f),tl_sign_pow(enc.y,2.4f),tl_sign_pow(enc.z,2.4f));
}
__DEVICE__ float3 ks_awg4_to_tl_rec709(float3 awg4){
  float3 xyz=awg4_to_xyz(awg4);
  float3 lin709=ks_mul_rows(XYZ_TO_REC709_R0,XYZ_TO_REC709_R1,XYZ_TO_REC709_R2,xyz);
  return tl_rec709_gamma24_encode(lin709);
}
__DEVICE__ float3 ks_tl_rec709_to_awg4(float3 rgb709){
  float3 lin709=tl_rec709_gamma24_decode(rgb709);
  float3 xyz=ks_mul_rows(REC709_TO_XYZ_R0,REC709_TO_XYZ_R1,REC709_TO_XYZ_R2,lin709);
  return xyz_to_awg4(xyz);
}
__DEVICE__ float tl_luma709(float3 rgb){return 0.2126f*rgb.x+0.7152f*rgb.y+0.0722f*rgb.z;}

// Direct port of ToneLab Basic Adjustments Bleach Bypass.
// Input must be ToneLab's display-referred Rec.709/Gamma 2.4 signal.
// No skin mask, gamut clamp, safety mix, or other Keystone modification.
__DEVICE__ float3 tl_bleach_bypass_exact(float3 rgb,float amount){
  float l=tl_luma709(rgb);
  float3 lum=make_float3(l,l,l);
  float3 base=mix3(rgb,lum,0.5f*amount);
  float overlay_mix=clampf((l-0.45f)*10.0f,0.0f,1.0f);
  float3 multiply=base*make_float3(2.0f*l,2.0f*l,2.0f*l);
  float one_minus_l=1.0f-l;
  float3 screen=make_float3(1.0f,1.0f,1.0f)
              -(make_float3(1.0f,1.0f,1.0f)-base)
               *make_float3(2.0f*one_minus_l,2.0f*one_minus_l,2.0f*one_minus_l);
  return mix3(rgb,mix3(multiply,screen,overlay_mix),amount);
}

// Standard HSL hue/lightness carrier used by ToneLab's recovered skin selector.
// Returns hue in degrees [0,360) in x and HSL lightness in y.
__DEVICE__ float2 tl_hsl_hue_lightness(float3 rgb){
  float mx=_fmaxf(rgb.x,_fmaxf(rgb.y,rgb.z));
  float mn=_fminf(rgb.x,_fminf(rgb.y,rgb.z));
  float d=mx-mn;
  float h=0.0f;
  if(d>1.0e-12f){
    if(mx==rgb.x){h=60.0f*((rgb.y-rgb.z)/d);if(h<0.0f)h+=360.0f;}
    else if(mx==rgb.y)h=60.0f*(((rgb.z-rgb.x)/d)+2.0f);
    else h=60.0f*(((rgb.x-rgb.y)/d)+4.0f);
  }
  return make_float2(h,0.5f*(mx+mn));
}
__DEVICE__ float tl_hue_distance_deg(float a,float b){
  float d=_fabs(a-b);
  return _fminf(d,360.0f-d);
}
__DEVICE__ float tl_skin_mask(float3 rgb){
  float2 hl=tl_hsl_hue_lightness(rgb);
  float hd=tl_hue_distance_deg(hl.x,TL_SKIN_TARGET_HUE_DEG);
  float hue_den=TL_SKIN_RANGE_DEG*TL_SKIN_RANGE_DEG;
  float ld=hl.y-TL_SKIN_LIGHTNESS_CENTER;
  float lum_den=TL_SKIN_LIGHTNESS_WIDTH*TL_SKIN_LIGHTNESS_WIDTH;
  float hue_mask=_expf(-(hd*hd)/_fmaxf(hue_den,1.0e-12f));
  float luma_mask=_expf(-(ld*ld)/_fmaxf(lum_den,1.0e-12f));
  return clampf(hue_mask*luma_mask,0.0f,1.0f);
}

__DEVICE__ float3 tl_fade(float3 rgb,float amount){
  float a=clampf(amount,0.0f,1.0f);
  if(a<=1.0e-8f)return rgb;
  float lift=a*0.25f;
  float3 t=make_float3(clampf(1.0f-rgb.x,0.0f,1.0f),clampf(1.0f-rgb.y,0.0f,1.0f),clampf(1.0f-rgb.z,0.0f,1.0f));
  return rgb+make_float3(lift*t.x*t.x,lift*t.y*t.y,lift*t.z*t.z);
}

__DEVICE__ float3 ks_apply_tonelab_fade(float3 awg4,float fade){
  if(fade<=1.0e-8f)return awg4;
  float3 rgb=ks_awg4_to_tl_rec709(awg4);
  float skin=tl_skin_mask(rgb);
  float3 processed=tl_fade(rgb,fade);
  // Preserve the existing ToneLab-derived skin protection around Fade.
  float3 protected_rgb=mix3(rgb,processed,1.0f-skin);
  return ks_tl_rec709_to_awg4(protected_rgb);
}


// ToneLab-derived building blocks for Keystone's existing Look / Color presets.
// The operator equations are recovered ToneLab math. These recipe amounts and
// their combination with Keystone hue recipes are intentionally Keystone-authored.
__DEVICE__ float ks_look_recipe_fade(int p){
  if(p==1)return 0.04f;
  if(p==2)return 0.03f;
  if(p==3)return 0.02f;
  if(p==4)return 0.18f;
  if(p==5)return 0.06f;
  if(p==6)return 0.14f;
  if(p==7)return 0.04f;
  if(p==8)return 0.03f;
  if(p==9)return 0.04f;
  if(p==10)return 0.03f;
  if(p==11)return 0.02f;
  return 0.0f;
}
__DEVICE__ float3 ks_apply_color_look(float3 awg4,int p,float amount){
  float a=clampf(amount,0.0f,1.5f);
  if(p<=0||p>11||a<=1.0e-8f)return awg4;

  float3 out=awg4;
  float fade=ks_look_recipe_fade(p)*a;
  if(fade>1.0e-8f)
    out=ks_apply_tonelab_fade(out,fade);

  // Preserve each preset's established Keystone hue/tone identity.
  out=ks_apply_color_look_base(out,p,a);
  return out;
}

// ---- PrimeraSkin v0.6.0 scalar qualifier/edit core, evaluated in LogC4/AWG4 ----
#define PRIMERA_SKIN_HUE_CENTER (28.0f/360.0f)
#define PRIMERA_SKIN_HUE_WIDTH (28.0f/360.0f)
#define PRIMERA_SKIN_COMPRESS_EXTRA 0.4f
__DEVICE__ float primera_hue_distance(float h1,float h2){float d=_fabs(h1-h2);return _fminf(d,1.0f-d);}
__DEVICE__ float primera_smoothstep(float e0,float e1,float x){float t=clampf((x-e0)/(e1-e0),0.0f,1.0f);return t*t*(3.0f-2.0f*t);}
__DEVICE__ float primera_skin_mask_gated(float r,float g,float b,float range,float lo,float hi){
  float sum=r+g+b;
  if(sum<0.01f)return 0.0f;
  float rn=r/sum,gn=g/sum,bn=b/sum;
  float gn_gate=primera_smoothstep(0.22f,0.33f,gn)*(1.0f-primera_smoothstep(0.37f,0.48f,gn));
  float rb_gate=primera_smoothstep(0.0f,0.04f,rn-bn);
  float3 hsv=ks_rgb_to_hsv(make_float3(r,g,b));
  float width=PRIMERA_SKIN_HUE_WIDTH*range;
  float dist=primera_hue_distance(hsv.x,PRIMERA_SKIN_HUE_CENTER);
  float hue_gate=1.0f-primera_smoothstep(0.0f,width,dist);
  float lo_gate=(lo>0.001f)?primera_smoothstep(lo*0.5f,lo,hsv.z):1.0f;
  float hi_gate=(hi<0.999f)?1.0f-primera_smoothstep(hi-(1.0f-hi)*0.5f,hi,hsv.z):1.0f;
  return hue_gate*gn_gate*rb_gate*lo_gate*hi_gate;
}
__DEVICE__ float3 apply_primera_skin_scalar(float3 enc,float hue,float sat,float density,float range,float evenness,float lo,float hi){
  float r=enc.x,g=enc.y,b=enc.z;
  if(evenness>0.0f){
    float cm=primera_skin_mask_gated(r,g,b,range+PRIMERA_SKIN_COMPRESS_EXTRA,lo,hi);
    if(cm>1.0e-6f){
      float3 hsv=ks_rgb_to_hsv(make_float3(r,g,b));
      float d=hsv.x-PRIMERA_SKIN_HUE_CENTER;
      if(d>0.5f)d-=1.0f;
      if(d<-0.5f)d+=1.0f;
      d*=(1.0f-evenness*cm);
      hsv.x=PRIMERA_SKIN_HUE_CENTER+d;
      if(hsv.x<0.0f)hsv.x+=1.0f;
      if(hsv.x>1.0f)hsv.x-=1.0f;
      float3 n=ks_hsv_to_rgb(hsv);
      r=n.x;g=n.y;b=n.z;
    }
  }
  float mask=primera_skin_mask_gated(r,g,b,range,lo,hi);
  if(mask>1.0e-6f){
    float3 hsv=ks_rgb_to_hsv(make_float3(r,g,b));
    if(_fabs(hue)>1.0e-9f){
      hsv.x-=mask*hue/360.0f;
      if(hsv.x<0.0f)hsv.x+=1.0f;
      if(hsv.x>1.0f)hsv.x-=1.0f;
    }
    if(_fabs(sat)>1.0e-9f)hsv.y=_fmaxf(0.0f,hsv.y*(1.0f+mask*sat));
    if(_fabs(density)>1.0e-9f)hsv.z=_fmaxf(0.0f,hsv.z*(1.0f-mask*density));
    return ks_hsv_to_rgb(hsv);
  }
  return make_float3(r,g,b);
}

// ---- Technical guards + ACES 1.3 Reference Gamut Compression ----
__DEVICE__ float3 technical_scene_ceiling_guard(float3 rgb){float m=_fmaxf(_fabs(rgb.x),_fmaxf(_fabs(rgb.y),_fabs(rgb.z)));if(m<=1.0e8f)return rgb;float k=1.0e8f/m;return rgb*make_float3(k,k,k);}
__DEVICE__ float3 technical_encoded_negative_guard(float3 rgb,float Y){
  const float code_floor=-1.0f;float3 enc=logc4_encode(rgb);float mn=_fminf(enc.x,_fminf(enc.y,enc.z));if(mn>=code_floor)return rgb;
  if(Y>0.0f){float3 neutral=make_float3(Y,Y,Y),delta=rgb-neutral;float3 neutral_enc=logc4_encode(neutral);float neutral_min=_fminf(neutral_enc.x,_fminf(neutral_enc.y,neutral_enc.z));
    if(neutral_min>=code_floor){float lo=0.0f,hi=1.0f;for(int i=0;i<16;i++){float mid=(lo+hi)*0.5f;float3 candidate=neutral+delta*make_float3(mid,mid,mid);float3 candidate_enc=logc4_encode(candidate);float candidate_min=_fminf(candidate_enc.x,_fminf(candidate_enc.y,candidate_enc.z));if(candidate_min>=code_floor)lo=mid;else hi=mid;}return neutral+delta*make_float3(lo,lo,lo);}}
  float lo=0.0f,hi=1.0f;for(int i=0;i<16;i++){float mid=(lo+hi)*0.5f;float3 candidate=rgb*make_float3(mid,mid,mid);float3 candidate_enc=logc4_encode(candidate);float candidate_min=_fminf(candidate_enc.x,_fminf(candidate_enc.y,candidate_enc.z));if(candidate_min>=code_floor)lo=mid;else hi=mid;}return rgb*make_float3(lo,lo,lo);
}
__DEVICE__ float rgc_comp_dist(float dn,float t,float ss){if(dn<t)return dn;float x=(dn-t)/ss;float dp=_powf(_fmaxf(x,0.0f),1.2f);return t+(dn-t)/_powf(1.0f+dp,1.0f/1.2f);}
__DEVICE__ float3 rgc_core_ap1(float3 rgb){float A=_fmaxf(rgb.x,_fmaxf(rgb.y,rgb.z)),aa=_fabs(A);if(aa<=1e-20f)return rgb;float dnR=(A-rgb.x)/aa,dnG=(A-rgb.y)/aa,dnB=(A-rgb.z)/aa;float dcR=rgc_comp_dist(dnR,0.815f,0.327301877468f),dcG=rgc_comp_dist(dnG,0.803f,0.285938028927f),dcB=rgc_comp_dist(dnB,0.880f,0.146821457838f);return make_float3(A-dcR*aa,A-dcG*aa,A-dcB*aa);}
__DEVICE__ float3 input_gamut_heal_xyz(float3 xyz){float3 ap1=mmul(X65_TO_AP1_C0,X65_TO_AP1_C1,X65_TO_AP1_C2,xyz);ap1=rgc_core_ap1(ap1);return mmul(AP1_TO_X65_C0,AP1_TO_X65_C1,AP1_TO_X65_C2,ap1);}


// ============================================================================
// Keystone v4.5.6 Auto Neutral — sparse content-adaptive AWG4/LogC4 analysis
// Sources/derivation:
//   * Gray World / Shades-of-Gray / diagonal von Kries are classical color-
//     constancy methods. Keystone evaluates them directly in linear AWG4.
//   * Exposure follows the robust idea of targeting 18% from median linear Y,
//     with a highlight-safe cap. No correction LUT is generated or sampled.
//   * Mixed-illuminant handling is conservative: quadrant disagreement lowers
//     the confidence of a single global correction rather than pretending one
//     global illuminant can solve spatially different light sources.
//   * Skin is diagnostic only: likely-skin samples can REDUCE confidence when
//     a proposed WB would move their hue farther from Keystone's skin axis.
//
// Performance: a fixed 6x4 grid = 24 RGB texture samples per output pixel.
// All threads request the same sparse coordinates, which is texture-cache
// Neutral / Amount == 0 is a cheap exact bypass; no separate mode selector is needed.
// ============================================================================


// ---- SpektraFilm camera UV/IR filter shape, compact AWG4 RGB projection ----
// SpektraFilm applies this wavelength-resolved before film exposure. Keystone does
// not carry a spectrum, so the same erf cutoff law is evaluated at a documented
// 610/550/450 nm RGB basis. This preserves the source cutoff behavior but is not
// spectral equivalence.
__DEVICE__ float ks_sf_erf_approx(float x){
  float sg=x<0.0f?-1.0f:1.0f;float a=_fabs(x);float t=1.0f/(1.0f+0.3275911f*a);
  float y=1.0f-(((((1.061405429f*t-1.453152027f)*t+1.421413741f)*t-0.284496736f)*t+0.254829592f)*t)*_expf(-a*a);
  return sg*y;
}
__DEVICE__ float ks_sf_cut(float wl,float center,float width){float w=_fmaxf(_fabs(width),1.0e-6f);return 0.5f*ks_sf_erf_approx((wl-center)/w)+0.5f;}
__DEVICE__ float3 ks_apply_spektrafilm_uv_ir(float3 rgb,int uvOn,float uvEdge,int irOn,float irEdge){
  if(!uvOn&&!irOn)return rgb;
  float tr=1.0f,tg=1.0f,tb=1.0f;
  if(uvOn){tr*=ks_sf_cut(610.0f,uvEdge,8.0f);tg*=ks_sf_cut(550.0f,uvEdge,8.0f);tb*=ks_sf_cut(450.0f,uvEdge,8.0f);}
  if(irOn){tr*=1.0f-ks_sf_cut(610.0f,irEdge,15.0f);tg*=1.0f-ks_sf_cut(550.0f,irEdge,15.0f);tb*=1.0f-ks_sf_cut(450.0f,irEdge,15.0f);}
  float mean=(tr+tg+tb)*(1.0f/3.0f);if(mean<=1.0e-8f)return rgb;
  return make_float3(rgb.x*tr/mean,rgb.y*tg/mean,rgb.z*tb/mean);
}

// ---- ToneLab Skin Tones block ----
// Parameter names, ranges, defaults and preset names are recovered from ToneLab.ofx.
// This implementation uses ToneLab's recovered HSL hue/lightness Gaussian selector.
__DEVICE__ float tl_skin_mask_controls(float3 rgb,float centerOffset,float range,float brightCenter,float brightRange){
  float2 hl=tl_hsl_hue_lightness(rgb);
  float center=TL_SKIN_TARGET_HUE_DEG+clampf(centerOffset,-30.0f,30.0f);
  while(center<0.0f)center+=360.0f;while(center>=360.0f)center-=360.0f;
  float hd=tl_hue_distance_deg(hl.x,center);
  float hr=clampf(range,15.0f,65.0f);
  float bc=clampf(brightCenter,0.0f,1.0f),br=clampf(brightRange,0.25f,1.75f);
  // ToneLab's original fixed brightness width was 0.4 at the neutral range value 1.
  float lw=0.4f*br;
  float ld=hl.y-bc;
  return clampf(_expf(-(hd*hd)/(hr*hr))*_expf(-(ld*ld)/(lw*lw)),0.0f,1.0f);
}
__DEVICE__ float3 tl_skin_tones_apply_rgb(float3 rgb,int enabled,float sat,float lum,float hue,float center,float range,float bcenter,float brange,float intensity){
  if(!enabled)return rgb;
  float master=clampf(intensity,0.0f,200.0f)*0.01f;if(master<=1.0e-8f)return rgb;
  float mask=tl_skin_mask_controls(rgb,center,range,bcenter,brange);
  float w=clampf(mask*master,0.0f,1.0f);
  if(w<=1.0e-8f)return rgb;
  float3 hsv=ks_rgb_to_hsv(rgb);
  hsv.x += clampf(hue,-60.0f,60.0f)/360.0f; if(hsv.x<0.0f)hsv.x+=1.0f; if(hsv.x>=1.0f)hsv.x-=1.0f;
  hsv.y=clampf(hsv.y*(1.0f+clampf(sat,-1.0f,1.0f)),0.0f,4.0f);
  float3 changed=ks_hsv_to_rgb(hsv);
  // Luminance/density trim while retaining the hue/saturation result.
  float Y0=0.2126f*changed.x+0.7152f*changed.y+0.0722f*changed.z;
  float gain=_powf(2.0f,clampf(lum,-1.0f,1.0f));
  changed*=gain;
  // keep adjustment smooth through the soft selector
  return mix3(rgb,changed,w);
}
__DEVICE__ float3 ks_apply_tonelab_skin(float3 awg4,int enabled,float sat,float lum,float hue,float center,float range,float bcenter,float brange,float intensity){
  if(!enabled)return awg4;
  float3 rgb=ks_awg4_to_tl_rec709(awg4);
  rgb=tl_skin_tones_apply_rgb(rgb,enabled,sat,lum,hue,center,range,bcenter,brange,intensity);
  return ks_tl_rec709_to_awg4(rgb);
}
__DEVICE__ float ks_tonelab_skin_mask_awg4(float3 awg4,float center,float range,float bcenter,float brange){return tl_skin_mask_controls(ks_awg4_to_tl_rec709(awg4),center,range,bcenter,brange);}
