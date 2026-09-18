#pragma once
#include "OpenFXMinimal.h"

namespace native_match_bridge {
bool available();
const char* lastError();
void setHost(void* host);
OfxStatus delegate(const char* action,const void* handle,OfxPropertySetHandle inArgs,OfxPropertySetHandle outArgs);
}
