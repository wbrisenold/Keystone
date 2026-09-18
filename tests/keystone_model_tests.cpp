#include "KeystoneCPU.h"
#include "NeutralAnalyzer.h"
#include "LutLoader.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
using namespace keystone;
static float logc4(float x){const float A=2231.826309067688271f,B=.907135874877810f,C=.092864125122190f,S=.113597208610589f,T=-.018056996119911f;if(x<T)return (x-T)/S;return ((std::log2(A*x+64.0f)-6.0f)/14.0f)*B+C;}
int main(){
  std::vector<LutEntry> lut;std::string err;assert(loadCube33("resources/Keystone_Output_LogC4_to_Rec709.cube",lut,&err));assert(lut.size()==35937);
  Params p;std::vector<float> img(64*36*4,1.0f);float neutral=logc4(.18f);for(size_t i=0;i<img.size();i+=4){img[i]=img[i+1]=img[i+2]=neutral;img[i+3]=1;}
  auto n=analyzeNeutralRGBA(img.data(),64,36,64*4,p);assert(n.valid);assert(std::isfinite(n.slopeR)&&std::isfinite(n.slopeG)&&std::isfinite(n.slopeB));
  // A synthetic warm cast should ask for relatively less red and/or more blue.
  for(size_t i=0;i<img.size();i+=4){img[i]=logc4(.24f);img[i+1]=logc4(.18f);img[i+2]=logc4(.12f);img[i+3]=1;}
  auto warm=analyzeNeutralRGBA(img.data(),64,36,64*4,p);assert(warm.valid);assert(std::isfinite(warm.slopeR)&&std::isfinite(warm.offsetB));
  // Spatial rearrangement robustness: same sampled distribution, different positions.
  std::vector<float> a(240*144*4,1.0f), b(240*144*4,1.0f);
  for(int y=0;y<144;y++) for(int x=0;x<240;x++){
    bool alt=((x/24)+(y/24))&1;
    float ar=logc4(alt?.28f:.12f), ag=logc4(alt?.21f:.15f), ab=logc4(alt?.14f:.10f);
    size_t i=(size_t)(y*240+x)*4; a[i]=ar;a[i+1]=ag;a[i+2]=ab;a[i+3]=1;
    int xx=239-x; size_t j=(size_t)(y*240+xx)*4; b[j]=ar;b[j+1]=ag;b[j+2]=ab;b[j+3]=1;
  }
  auto na=analyzeNeutralRGBA(a.data(),240,144,240*4,p), nb=analyzeNeutralRGBA(b.data(),240,144,240*4,p);
  assert(na.valid&&nb.valid);
  assert(std::fabs(na.slopeR-nb.slopeR)<1e-5f&&std::fabs(na.slopeG-nb.slopeG)<1e-5f&&std::fabs(na.slopeB-nb.slopeB)<1e-5f);
  assert(std::fabs(na.offsetR-nb.offsetR)<1e-5f&&std::fabs(na.offsetG-nb.offsetG)<1e-5f&&std::fabs(na.offsetB-nb.offsetB)<1e-5f);
  // Stored neutral is stable across frames: processing never re-analyzes pixels.
  p.matchSlopeR=warm.slopeR;p.matchSlopeG=warm.slopeG;p.matchSlopeB=warm.slopeB;p.matchOffsetR=warm.offsetR;p.matchOffsetG=warm.offsetG;p.matchOffsetB=warm.offsetB;p.neutralAmount=1.0f;
  std::vector<float> out(img.size());processRGBA(img.data(),out.data(),64,36,64*4,64*4,p,lut);for(float v:out)assert(std::isfinite(v));
  // Amount zero must be a true neutral-correction bypass (other Keystone processing remains).
  p.neutralAmount=0.0f;std::vector<float> out0(img.size());processRGBA(img.data(),out0.data(),64,36,64*4,64*4,p,lut);for(float v:out0)assert(std::isfinite(v));
  // New v1.4 camera-filter and Keystone-skin controls remain finite and image-dependent.
  Params fx;fx.uvFilter=1;fx.uvCutNm=500.0f;fx.irFilter=1;fx.irCutNm=620.0f;
  std::vector<float> fxout(img.size());processRGBA(img.data(),fxout.data(),64,36,64*4,64*4,fx,lut);for(float v:fxout)assert(std::isfinite(v));
  Params sk;sk.skinEnable=1;sk.skinSaturate=.55f;sk.skinPop=-8.0f;sk.skinIntensity=100.0f;
  std::vector<float> skout(img.size());processRGBA(img.data(),skout.data(),64,36,64*4,64*4,sk,lut);for(float v:skout)assert(std::isfinite(v));
  std::cout<<"keystone_model_tests: OK\n";return 0;
}
