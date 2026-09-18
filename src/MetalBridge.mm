#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <dlfcn.h>
#include <mutex>
#include <vector>
#include "MetalBridge.h"
#include "LutLoader.h"
namespace keystone {
struct MetalState { id<MTLDevice> device=nil; id<MTLLibrary> library=nil; id<MTLComputePipelineState> pipeline=nil; id<MTLBuffer> lut=nil; bool ready=false; };
static std::mutex gMutex; static MetalState gState;
struct MetalParams {
  float neutralAmount; float neutralGainR,neutralGainG,neutralGainB; float matchSlopeR,matchSlopeG,matchSlopeB,matchOffsetR,matchOffsetG,matchOffsetB; int ndFilter;
  int uvFilter; float uvCutNm; int irFilter; float irCutNm;
  float sceneGainTemp,sceneOffsetTemp,sceneExposure,sceneShadows,sceneHighlights;
  float wbTemp,wbTint; float exposure,blackPoint,contrast,shadows,highlights,roll; int curvePreset;
  float density,posSat,interlayer,satSplit; float splitAmount,splitShadowHue,splitHighlightHue,splitBalance,splitSubtractive;
  float hiBleach,loBleach,colorBleach,fade; int lookColor; float lookAmount; int creativeWhite; int skinEnable,skinPreset; float skinSaturate,skinColour,skinPop,skinCenter,skinRange,skinBrightness,skinBrightnessRange; int skinShowMask; float skinIntensity;
  int width,height,srcStrideFloats,dstStrideFloats;
};
static NSURL* metallibURL(){
  Dl_info i{}; if(dladdr((const void*)&metallibURL,&i)==0||!i.dli_fname)return nil;
  NSString* b=[NSString stringWithUTF8String:i.dli_fname]; NSString* contents=[[b stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
  return [NSURL fileURLWithPath:[[[contents stringByAppendingPathComponent:@"Resources"] stringByAppendingPathComponent:@"KeystoneKernels"] stringByAppendingPathExtension:@"metallib"]];
}
static bool initMetal(id<MTLCommandQueue> q){
  std::lock_guard<std::mutex> lock(gMutex); if(!q)return false; id<MTLDevice> d=[q device]; if(!d)return false;
  if(gState.ready&&gState.device==d)return true; gState=MetalState{};gState.device=d;NSError* err=nil;NSURL* u=metallibURL();if(!u)return false;
  gState.library=[d newLibraryWithURL:u error:&err];if(!gState.library)return false;id<MTLFunction> f=[gState.library newFunctionWithName:@"keystoneKernel"];if(!f)return false;
  gState.pipeline=[d newComputePipelineStateWithFunction:f error:&err];if(!gState.pipeline)return false;
  std::vector<LutEntry> lut;std::string why;if(!loadCube33(bundledReferentPath(),lut,&why)||lut.size()!=35937)return false;
  gState.lut=[d newBufferWithBytes:lut.data() length:lut.size()*sizeof(LutEntry) options:MTLResourceStorageModeShared];if(!gState.lut)return false;
  gState.ready=true;return true;
}
bool runMetal(void* cq,int w,int h,void* srcH,void* dstH,int ss,int ds,const Params& p){
  id<MTLCommandQueue> q=(__bridge id<MTLCommandQueue>)cq;id<MTLBuffer> inB=(__bridge id<MTLBuffer>)srcH;id<MTLBuffer> outB=(__bridge id<MTLBuffer>)dstH;
  if(!q||!inB||!outB||w<=0||h<=0||ss<w*4||ds<w*4||(ss%4)||(ds%4)||!initMetal(q))return false;
  NSUInteger sb=(NSUInteger)ss*h*sizeof(float),db=(NSUInteger)ds*h*sizeof(float);if([inB length]<sb||[outB length]<db)return false;
  MetalParams mp{p.neutralAmount,p.neutralGainR,p.neutralGainG,p.neutralGainB,p.matchSlopeR,p.matchSlopeG,p.matchSlopeB,p.matchOffsetR,p.matchOffsetG,p.matchOffsetB,p.ndFilter,p.uvFilter,p.uvCutNm,p.irFilter,p.irCutNm,p.sceneGainTemp,p.sceneOffsetTemp,p.sceneExposure,p.sceneShadows,p.sceneHighlights,p.wbTemp,p.wbTint,p.exposure,p.blackPoint,p.contrast,p.shadows,p.highlights,p.roll,p.curvePreset,p.density,p.posSat,p.interlayer,p.satSplit,p.splitAmount,p.splitShadowHue,p.splitHighlightHue,p.splitBalance,p.splitSubtractive,p.hiBleach,p.loBleach,p.colorBleach,p.fade,p.lookColor,p.lookAmount,p.creativeWhite,p.skinEnable,p.skinPreset,p.skinSaturate,p.skinColour,p.skinPop,p.skinCenter,p.skinRange,p.skinBrightness,p.skinBrightnessRange,p.skinShowMask,p.skinIntensity,w,h,ss,ds};
  id<MTLCommandBuffer> cb=[q commandBuffer];if(!cb)return false;id<MTLComputeCommandEncoder> enc=[cb computeCommandEncoder];if(!enc)return false;
  [enc setComputePipelineState:gState.pipeline];[enc setBuffer:inB offset:0 atIndex:0];[enc setBuffer:outB offset:0 atIndex:1];[enc setBytes:&mp length:sizeof(mp) atIndex:2];[enc setBuffer:gState.lut offset:0 atIndex:3];
  NSUInteger tw=gState.pipeline.threadExecutionWidth;if(tw<1)tw=1;NSUInteger avail=gState.pipeline.maxTotalThreadsPerThreadgroup/tw;NSUInteger th=avail<16?avail:16;if(th<1)th=1;
  [enc dispatchThreads:MTLSizeMake(w,h,1) threadsPerThreadgroup:MTLSizeMake(tw,th,1)];[enc endEncoding];[cb commit];return true;
}
}
