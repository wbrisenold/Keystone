#include "../src/OpenFXMinimal.h"
#include <dlfcn.h>
#include <cstdio>

static int gFetches=0;
static int dummySuite=1;
static void* fetchSuite(OfxPropertySetHandle,const char*,int){ ++gFetches; return &dummySuite; }

int main(int argc,char**argv){
  if(argc!=2){std::fprintf(stderr,"usage: %s plugin\n",argv[0]);return 2;}
  void* lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
  if(!lib){std::fprintf(stderr,"dlopen: %s\n",dlerror());return 3;}
  using CountFn=int(*)(); using GetFn=OfxPlugin*(*)(int); using SetFn=void(*)(void*);
  auto count=(CountFn)dlsym(lib,"OfxGetNumberOfPlugins");
  auto get=(GetFn)dlsym(lib,"OfxGetPlugin");
  auto set=(SetFn)dlsym(lib,"OfxSetHost");
  if(!count||!get||!set||count()!=1){std::fprintf(stderr,"OFX exports invalid\n");return 4;}
  OfxPlugin* p=get(0); if(!p||!p->setHost||!p->mainEntry){return 5;}
  OfxHost host{nullptr,fetchSuite};
  gFetches=0; p->setHost(&host);
  if(gFetches!=3){std::fprintf(stderr,"setHost fetched %d suites, expected 3\n",gFetches);return 6;}
  dlclose(lib);
  std::puts("ofx_loader_smoke: OK");
  return 0;
}
