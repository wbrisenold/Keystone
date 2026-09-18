#include "LutLoader.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#ifdef __APPLE__
#include <dlfcn.h>
#endif
namespace keystone {
bool loadCube33(const std::string& path,std::vector<LutEntry>& out,std::string* error){
  std::ifstream f(path); if(!f){if(error)*error="cannot open LUT: "+path;return false;}
  std::vector<LutEntry> tmp; tmp.reserve(35937); std::string line; int size=0;
  while(std::getline(f,line)){
    if(line.empty()||line[0]=='#')continue;
    std::istringstream ss(line); std::string first; ss>>first; if(!ss)continue;
    if(first=="TITLE"||first=="DOMAIN_MIN"||first=="DOMAIN_MAX"||first=="LUT_1D_SIZE")continue;
    if(first=="LUT_3D_SIZE"){ss>>size;continue;}
    char* end=nullptr; float r=std::strtof(first.c_str(),&end); if(end==first.c_str()||*end!='\0')continue;
    float g,b; if(!(ss>>g>>b))continue; tmp.push_back({r,g,b,0.0f});
  }
  if(size!=33||tmp.size()!=35937){if(error){std::ostringstream s;s<<"expected 33^3 LUT (35937 nodes), got size="<<size<<" nodes="<<tmp.size();*error=s.str();}return false;}
  out.swap(tmp); return true;
}
std::string bundledOutputTransformPath(){
  if(const char* e=std::getenv("KEYSTONE_OUTPUT_LUT"))return e;
#ifdef __APPLE__
  Dl_info info{}; if(dladdr((const void*)&bundledOutputTransformPath,&info)&&info.dli_fname){
    std::string p=info.dli_fname; auto slash=p.find_last_of('/'); if(slash!=std::string::npos){
      p=p.substr(0,slash); // Contents/MacOS
      slash=p.find_last_of('/'); if(slash!=std::string::npos) p=p.substr(0,slash)+"/Resources/Keystone_Output_LogC4_to_Rec709.cube";
      return p;
    }
  }
#endif
  return "resources/Keystone_Output_LogC4_to_Rec709.cube";
}
}
