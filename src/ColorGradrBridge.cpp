#include "ColorGradrBridge.h"
#include <string>
#include <mutex>
#ifdef __APPLE__
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#endif

namespace colorgradr_bridge {
namespace {
std::mutex gMutex;
std::string gError;
OfxPlugin* gPlugin=nullptr;
#ifdef __APPLE__
void* gLib=nullptr;
using GetCountFn=int(*)();
using GetPluginFn=OfxPlugin*(*)(int);

static std::string dirname(std::string p){auto n=p.find_last_of('/');return n==std::string::npos?std::string():p.substr(0,n);}
static bool tryLoad(const std::string& path){
  gLib=dlopen(path.c_str(),RTLD_NOW|RTLD_LOCAL);
  if(!gLib){const char* e=dlerror();gError=e?e:"dlopen failed";return false;}
  auto count=(GetCountFn)dlsym(gLib,"OfxGetNumberOfPlugins");
  auto get=(GetPluginFn)dlsym(gLib,"OfxGetPlugin");
  if(!count||!get||count()<1){gError="ColorGradr OFX entry points missing";dlclose(gLib);gLib=nullptr;return false;}
  gPlugin=get(0);
  if(!gPlugin||!gPlugin->mainEntry||!gPlugin->setHost){gError="ColorGradr plugin descriptor invalid";dlclose(gLib);gLib=nullptr;gPlugin=nullptr;return false;}
  return true;
}
static void ensureLoaded(){
  if(gPlugin||gLib)return;
  Dl_info info{};
  if(dladdr((void*)&ensureLoaded,&info)&&info.dli_fname){
    std::string macos=dirname(info.dli_fname);
    std::string bundle=dirname(macos);
    std::string embedded=bundle+"/Resources/ColorGradr/colorgradr.ofx.bundle/Contents/MacOS/colorgradr.ofx";
    if(tryLoad(embedded))return;
  }
  const char* installed="/Library/OFX/Plugins/colorgradr.ofx.bundle/Contents/MacOS/colorgradr.ofx";
  if(tryLoad(installed))return;
  gError="Unable to load bundled or installed ColorGradr OFX";
}
#else
static void ensureLoaded(){gError="ColorGradr bridge only loads on macOS";}
#endif
}

bool available(){std::lock_guard<std::mutex> l(gMutex);ensureLoaded();return gPlugin!=nullptr;}
const char* lastError(){std::lock_guard<std::mutex> l(gMutex);return gError.c_str();}
void setHost(void* host){std::lock_guard<std::mutex> l(gMutex);ensureLoaded();if(gPlugin&&gPlugin->setHost)gPlugin->setHost(host);}
OfxStatus delegate(const char* action,const void* handle,OfxPropertySetHandle inArgs,OfxPropertySetHandle outArgs){
  std::lock_guard<std::mutex> l(gMutex);ensureLoaded();if(!gPlugin||!gPlugin->mainEntry)return kOfxStatFailed;
  return gPlugin->mainEntry(action,handle,inArgs,outArgs);
}
}
