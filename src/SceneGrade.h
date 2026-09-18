#pragma once
#include "KeystoneParams.h"
#include <string>
#include <vector>

namespace keystone {

enum SceneRegion : int {
  SceneSky=0, SceneWater, SceneSkin, SceneFoliage, SceneTerrain, SceneGround, SceneBuilt, SceneOther, SceneRegionCount
};

struct SceneGradeResult {
  bool ok=false;
  bool modelReady=false;
  int subject=-1;
  int option=0;
  int options=0;
  int colorMode=-1; // 0 multiplicative temperature, 1 additive temperature
  int colorSign=0;
  unsigned regionMask=0;
  float subjectCover=0.0f;
  float subjectL=0.0f, surroundL=0.0f;
  float subjectB=0.0f, surroundB=0.0f;
  float colorBase=0.0f;
  float sceneGainTemp=0.0f;
  float sceneOffsetTemp=0.0f;
  float sceneExposure=0.0f;
  float sceneShadows=0.0f;
  float sceneHighlights=0.0f;
  float confidence=0.0f;
  std::string note;
};

const char* sceneRegionName(int region);
bool analyzeSceneGrade(const float* rgba,int width,int height,int strideFloats,
                       const Params& current,const std::vector<LutEntry>& displayLut,
                       int requestedRegion,float separation,float bias,
                       SceneGradeResult& out);

} // namespace keystone
