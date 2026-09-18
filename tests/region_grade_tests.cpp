#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include "../src/KeystoneCoreCPU.h"
#include "../src/KeystoneCPU.h"
using namespace keystone;
using namespace keystone_cpu;

static float3 logFrom709(float r,float g,float b){
  float3 awg=ks_ks_skin_rec709_to_awg4(make_float3(r,g,b));
  return logc4_encode(awg);
}
static float avgZone(const std::vector<float>& a,int w,int h,int x0,int y0,int x1,int y1){
  double s=0;int n=0;for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){s+=a[((size_t)y*w+x)*4];++n;}return n?(float)(s/n):0.f;
}
int main(){
  const int W=120,H=80;std::vector<float> img((size_t)W*H*4,1.f),out(img.size());
  auto green=logFrom709(.15f,.48f,.14f);auto skin=logFrom709(.66f,.38f,.27f);auto sky=logFrom709(.25f,.48f,.78f);
  for(int y=0;y<H;y++)for(int x=0;x<W;x++){
    float3 c=(y>50)?sky:green;
    float dx=(x-W*.5f)/(W*.18f),dy=(y-H*.48f)/(H*.34f);
    if(dx*dx+dy*dy<1.f)c=skin;
    size_t i=((size_t)y*W+x)*4;img[i]=c.x;img[i+1]=c.y;img[i+2]=c.z;img[i+3]=1;
  }
  std::vector<LutEntry> lut(35937); // show-mask path bypasses output LUT
  Params p;p.regionEnable=1;p.regionShowMask=1;p.regionTarget=0;p.regionFeather=.2f;
  processRGBA(img.data(),out.data(),W,H,W*4,W*4,p,lut);
  float subjCenter=avgZone(out,W,H,48,25,72,55), subjEdge=avgZone(out,W,H,0,15,20,45);
  assert(subjCenter>subjEdge+0.15f);
  p.regionTarget=3;processRGBA(img.data(),out.data(),W,H,W*4,W*4,p,lut);
  float foliageLeft=avgZone(out,W,H,0,10,25,45), foliageCenter=avgZone(out,W,H,50,28,70,52);
  assert(foliageLeft>foliageCenter+0.10f);
  p.regionTarget=2;processRGBA(img.data(),out.data(),W,H,W*4,W*4,p,lut);
  float skyTop=avgZone(out,W,H,20,60,100,78),skyBottom=avgZone(out,W,H,20,0,100,25);
  assert(skyTop>skyBottom+0.15f);
  std::cout<<"region_grade_tests: OK\n";
}
