#include "KeystoneCPU.h"
#include "KeystoneCoreCPU.h"
#include <cstdlib>
namespace keystone {
void processRGBA(const float* src,float* dst,int w,int h,int ss,int ds,const Params& p,const std::vector<LutEntry>& lut){
  if(!src||!dst||w<=0||h<=0||std::abs(ss)<w*4||std::abs(ds)<w*4||lut.size()!=35937)return;
  for(int y=0;y<h;y++){
    const float* s=src+y*ss; float* d=dst+y*ds;
    for(int x=0;x<w;x++){
      const float* px=s+x*4; float* q=d+x*4;
      auto o=keystone_cpu::processPixel({px[0],px[1],px[2]},p,lut.data()); q[0]=o.x;q[1]=o.y;q[2]=o.z;q[3]=px[3];
    }
  }
}
}
