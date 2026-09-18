#pragma once
#include "KeystoneParams.h"
namespace keystone { bool runMetal(void* commandQueue,int width,int height,void* srcHandle,void* dstHandle,int srcStrideFloats,int dstStrideFloats,const Params& p); }
