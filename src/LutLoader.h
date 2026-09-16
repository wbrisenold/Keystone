#pragma once
#include <string>
#include <vector>
#include "KeystoneParams.h"
namespace keystone {
bool loadCube33(const std::string& path,std::vector<LutEntry>& out,std::string* error=nullptr);
std::string bundledReferentPath();
}
