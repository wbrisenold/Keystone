#pragma once
#include <cstdint>

namespace keystone {
struct Params {
  float neutralAmount=1.0f;
  float neutralGainR=1.0f, neutralGainG=1.0f, neutralGainB=1.0f;
  float matchSlopeR=1.0f, matchSlopeG=1.0f, matchSlopeB=1.0f;
  float matchOffsetR=0.0f, matchOffsetG=0.0f, matchOffsetB=0.0f;
  int32_t ndFilter=0;
  int32_t uvFilter=0; float uvCutNm=410.0f; int32_t irFilter=0; float irCutNm=675.0f;
  float wbTemp=0.0f, wbTint=0.0f;
  float exposure=0.0f, blackPoint=0.0f, contrast=1.0f, shadows=0.0f, highlights=0.0f, roll=0.0f;
  int32_t curvePreset=0;
  float density=0.0f, posSat=0.0f, interlayer=0.0f, satSplit=1.0f;
  float splitAmount=0.0f, splitShadowHue=220.0f, splitHighlightHue=40.0f, splitBalance=0.5f, splitSubtractive=0.5f;
  float hiBleach=0.0f, loBleach=0.0f, colorBleach=0.0f, fade=0.0f;
  int32_t lookColor=0;
  float lookAmount=1.0f;
  int32_t creativeWhite=2;
  int skinEnable=0, skinPreset=0, skinShowMask=0;
  float skinSaturate=0.2f, skinColour=0.0f, skinPop=0.0f, skinCenter=0.0f, skinRange=40.0f, skinBrightness=0.5f, skinBrightnessRange=1.0f, skinIntensity=100.0f;
};
struct NeutralResult { float gainR=1.0f,gainG=1.0f,gainB=1.0f; float slopeR=1.0f,slopeG=1.0f,slopeB=1.0f; float offsetR=0.0f,offsetG=0.0f,offsetB=0.0f; float confidence=0.0f; bool valid=false; };
struct LutEntry { float r=0,g=0,b=0,a=0; };
}
