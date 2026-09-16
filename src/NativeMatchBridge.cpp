#include "NativeMatchBridge.h"
#include <string>
#include <mutex>
#include <cstdio>
#ifdef __APPLE__
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace native_match_bridge {
namespace {
std::mutex gMutex;
std::string gError;
OfxPlugin* gPlugin=nullptr;
#ifdef __APPLE__
void* gLib=nullptr;
using GetCountFn=int(*)();
using GetPluginFn=OfxPlugin*(*)(int);

static std::string dirname(std::string p){auto n=p.find_last_of('/');return n==std::string::npos?std::string():p.substr(0,n);}
static void logLine(const std::string& s){if(FILE* f=std::fopen("/tmp/KeystoneOFX-loader.log","a")){std::fprintf(f,"%s\n",s.c_str());std::fclose(f);}}
static bool tryLoad(const std::string& path){
  gLib=dlopen(path.c_str(),RTLD_LAZY|RTLD_LOCAL);
  if(!gLib){const char* e=dlerror();gError=e?e:"dlopen failed";logLine("NativeMatch dlopen failed: "+gError);return false;}
  auto count=(GetCountFn)dlsym(gLib,"OfxGetNumberOfPlugins");
  auto get=(GetPluginFn)dlsym(gLib,"OfxGetPlugin");
  if(!count||!get||count()<1){gError="Native Match OFX entry points missing";logLine(gError);dlclose(gLib);gLib=nullptr;return false;}
  gPlugin=get(0);
  if(!gPlugin||!gPlugin->mainEntry||!gPlugin->setHost){gError="Native Match plugin descriptor invalid";logLine(gError);dlclose(gLib);gLib=nullptr;gPlugin=nullptr;return false;}
  logLine("NativeMatch engine loaded");
  return true;
}
static void ensureLoaded(){
  if(gPlugin||gLib)return;
  Dl_info info{};
  if(!(dladdr((void*)&ensureLoaded,&info)&&info.dli_fname)){gError="Unable to resolve Keystone bundle path";logLine(gError);return;}
  std::string macos=dirname(info.dli_fname);
  std::string contents=dirname(macos);
  std::string dir=contents+"/Resources/NativeMatch/NativeMatch.ofx.bundle/Contents/MacOS";
  DIR* d=opendir(dir.c_str());
  if(!d){gError="Embedded Native Match MacOS directory missing";logLine(gError+": "+dir);return;}
  while(dirent* ent=readdir(d)){
    if(!ent->d_name||ent->d_name[0]=='.')continue;
    std::string candidate=dir+"/"+ent->d_name;
    struct stat st{}; if(stat(candidate.c_str(),&st)!=0||!S_ISREG(st.st_mode))continue;
    if(tryLoad(candidate)){closedir(d);return;}
  }
  closedir(d);
  if(gError.empty())gError="No loadable embedded Native Match executable found";
  logLine(gError);
}
#else
static void ensureLoaded(){gError="Native Match bridge only loads on macOS";}
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
