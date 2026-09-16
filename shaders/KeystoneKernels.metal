#include "KeystoneShared.metalh"
kernel void keystoneKernel(device const float4* src [[buffer(0)]],device float4* dst [[buffer(1)]],constant KeystoneMetalParams& p [[buffer(2)]],device const float4* lut [[buffer(3)]],uint2 gid [[thread_position_in_grid]]){
  if(gid.x>=(uint)p.width||gid.y>=(uint)p.height)return;
  uint si=gid.y*(uint)(p.srcStrideFloats/4)+gid.x;
  uint di=gid.y*(uint)(p.dstStrideFloats/4)+gid.x;
  float4 in=src[si];float3 out=ksProcess(in.rgb,p,lut);dst[di]=float4(out,in.a);
}
