#include <net.h>
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include <mutex>

namespace {
enum Region : unsigned char { Sky=0, Water, Skin, Foliage, Terrain, Ground, Built, Other };

static const unsigned char kAdeToRegion[150] = {
 Built,Built,Sky,Ground,Foliage,Ground, Ground,Built,Built,Foliage,Other,Ground,
 Skin,Terrain,Built,Built,Terrain,Foliage, Built,Built,Built,Water,Other,Built,
 Built,Built,Water,Other,Other,Foliage, Other,Built,Built,Other,Terrain,Built,
 Other,Other,Built,Built,Other,Built, Built,Built,Other,Other,Terrain,Other,
 Built,Other,Other,Other,Terrain,Ground, Ground,Other,Water,Other,Built,Ground,
 Water,Ground,Built,Other,Built,Other, Foliage,Other,Terrain,Other,Other,Other,
 Foliage,Other,Other,Built,Other,Other, Other,Other,Built,Other,Other,Built,
 Built,Other,Other,Built,Other,Other, Other,Terrain,Other,Built,Terrain,Other,
 Other,Other,Other,Other,Other,Other, Other,Other,Other,Other,Other,Other,
 Other,Water,Other,Other,Other,Water, Other,Other,Other,Other,Other,Other,
 Other,Other,Other,Other,Other,Other, Other,Other,Water,Other,Built,Other,
 Other,Other,Other,Other,Other,Other, Other,Other,Ground,Built,Other,Other,
 Other,Other,Other,Other,Other,Other
};

class Segmenter {
public:
  bool load(const std::string& paramPath,const std::string& binPath){
    ready_=false; net_.clear();
    net_.opt.use_vulkan_compute=false;
    net_.opt.num_threads=1;
    net_.opt.lightmode=true;
    if(net_.load_param(paramPath.c_str())!=0)return false;
    if(net_.load_model(binPath.c_str())!=0)return false;
    ready_=true; return true;
  }
  bool run(const unsigned char* rgb,int w,int h,std::vector<unsigned char>& regions,int& ow,int& oh){
    if(!ready_||!rgb||w<=0||h<=0)return false;
    const float mean[3]={0.485f*255.f,0.456f*255.f,0.406f*255.f};
    const float norm[3]={1.f/(0.229f*255.f),1.f/(0.224f*255.f),1.f/(0.225f*255.f)};
    ncnn::Mat in=ncnn::Mat::from_pixels_resize(rgb,ncnn::Mat::PIXEL_RGB,w,h,512,512);
    in.substract_mean_normalize(mean,norm);
    ncnn::Extractor ex=net_.create_extractor();
    if(ex.input("in0",in)!=0)return false;
    ncnn::Mat out;
    if(ex.extract("out0",out)!=0||out.w<=0||out.h<=0||out.c<2)return false;
    const int cap=256;
    const int step=std::max(1,std::max(out.w,out.h)/cap);
    ow=(out.w+step-1)/step; oh=(out.h+step-1)/step;
    regions.assign((size_t)ow*(size_t)oh,(unsigned char)Other);
    const int classes=std::min(out.c,150);
    for(int y=0;y<oh;++y){
      const int sy=std::min(out.h-1,y*step);
      for(int x=0;x<ow;++x){
        const int sx=std::min(out.w-1,x*step);
        int best=0; float bv=-1e30f;
        for(int c=0;c<classes;++c){
          const float v=out.channel(c).row(sy)[sx];
          if(v>bv){bv=v;best=c;}
        }
        regions[(size_t)y*(size_t)ow+(size_t)x]=kAdeToRegion[best];
      }
    }
    return true;
  }
private:
  ncnn::Net net_;
  bool ready_=false;
};
}

#if defined(_WIN32)
#define KS_EXPORT __declspec(dllexport)
#else
#define KS_EXPORT __attribute__((visibility("default")))
#endif

extern "C" KS_EXPORT int KeystoneSceneSegment(const unsigned char* rgb,int w,int h,
                                               const char* modelDir,
                                               unsigned char* outRegions,int outCapacity,
                                               int* outW,int* outH){
  if(!rgb||!modelDir||!outRegions||outCapacity<=0||!outW||!outH)return 0;
  static Segmenter seg;
  static std::mutex mu;
  static std::string loadedDir;
  static bool ready=false;
  const std::string dir(modelDir);
  std::lock_guard<std::mutex> lock(mu);
  if(!ready||loadedDir!=dir){
    ready=seg.load(dir+"/ade20k.param",dir+"/ade20k.bin");
    loadedDir=dir;
  }
  if(!ready)return 0;
  std::vector<unsigned char> regions; int rw=0,rh=0;
  if(!seg.run(rgb,w,h,regions,rw,rh))return 0;
  const size_t need=(size_t)rw*(size_t)rh;
  if(need==0||need>(size_t)outCapacity||regions.size()!=need)return 0;
  std::copy(regions.begin(),regions.end(),outRegions);
  *outW=rw; *outH=rh; return 1;
}
