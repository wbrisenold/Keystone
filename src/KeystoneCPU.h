#pragma once
#include <vector>
#include "KeystoneParams.h"
namespace keystone {
void processRGBA(const float* src,float* dst,int width,int height,int srcStrideFloats,int dstStrideFloats,const Params& p,const std::vector<LutEntry>& lut);
}
