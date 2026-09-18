#include "OpenFXMinimal.h"
#include "KeystoneParams.h"
#include "NeutralAnalyzer.h"
#include "KeystoneCPU.h"
#include "LutLoader.h"
#include <cstring>
#include <memory>
#include <string>
#include <vector>

using namespace keystone;
static OfxHost* gHost=nullptr;static OfxPropertySuiteV1* gProp=nullptr;static OfxImageEffectSuiteV1* gEffect=nullptr;static OfxParameterSuiteV1* gParam=nullptr;

struct InstanceData {
  OfxImageClipHandle source=nullptr,output=nullptr;
  OfxParamHandle analyze=nullptr,resetNeutral=nullptr,neutralAmount=nullptr,neutralGainR=nullptr,neutralGainG=nullptr,neutralGainB=nullptr,matchSlopeR=nullptr,matchSlopeG=nullptr,matchSlopeB=nullptr,matchOffsetR=nullptr,matchOffsetG=nullptr,matchOffsetB=nullptr,neutralValid=nullptr,neutralConfidence=nullptr;
  OfxParamHandle ndFilter=nullptr,uvFilter=nullptr,uvCutNm=nullptr,irFilter=nullptr,irCutNm=nullptr,wbTemp=nullptr,wbTint=nullptr,exposure=nullptr,blackPoint=nullptr,contrast=nullptr,shadows=nullptr,highlights=nullptr,roll=nullptr,curvePreset=nullptr;
  OfxParamHandle density=nullptr,posSat=nullptr,interlayer=nullptr,satSplit=nullptr,splitAmount=nullptr,splitShadowHue=nullptr,splitHighlightHue=nullptr,splitBalance=nullptr,splitSubtractive=nullptr;
  OfxParamHandle regionEnable=nullptr,regionTarget=nullptr,regionExposure=nullptr,regionContrast=nullptr,regionSaturation=nullptr,regionTemp=nullptr,regionAmount=nullptr,regionFeather=nullptr,regionShowMask=nullptr;
  OfxParamHandle hiBleach=nullptr,loBleach=nullptr,colorBleach=nullptr,fade=nullptr,lookColor=nullptr,lookAmount=nullptr,creativeWhite=nullptr,skinEnable=nullptr,skinPreset=nullptr,skinSaturate=nullptr,skinColour=nullptr,skinPop=nullptr,skinCenter=nullptr,skinRange=nullptr,skinBrightness=nullptr,skinBrightnessRange=nullptr,skinShowMask=nullptr,skinIntensity=nullptr;
  std::vector<LutEntry> cpuLut;
};

static void setHostFunc(OfxHost* h){gHost=h;}

static OfxStatus onLoad(){
  if(!gHost||!gHost->fetchSuite)return kOfxStatErrMissingHostFeature;
  gProp=(OfxPropertySuiteV1*)gHost->fetchSuite(gHost->host,kOfxPropertySuite,1);
  gEffect=(OfxImageEffectSuiteV1*)gHost->fetchSuite(gHost->host,kOfxImageEffectSuite,1);
  gParam=(OfxParameterSuiteV1*)gHost->fetchSuite(gHost->host,kOfxParameterSuite,1);
  return (gProp&&gEffect&&gParam)?kOfxStatOK:kOfxStatErrMissingHostFeature;
}
static OfxStatus onUnload(){gProp=nullptr;gEffect=nullptr;gParam=nullptr;return kOfxStatOK;}
static void S(OfxPropertySetHandle h,const char*n,int i,const char*v){if(gProp)gProp->propSetString(h,n,i,v);}static void I(OfxPropertySetHandle h,const char*n,int i,int v){if(gProp)gProp->propSetInt(h,n,i,v);}static void D(OfxPropertySetHandle h,const char*n,int i,double v){if(gProp)gProp->propSetDouble(h,n,i,v);}static void P(OfxPropertySetHandle h,const char*n,int i,void*v){if(gProp)gProp->propSetPointer(h,n,i,v);}

static OfxPropertySetHandle defineGroup(OfxParamSetHandle ps,const char* id,const char* label,bool open){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeGroup,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxParamPropScriptName,0,id);I(p,kOfxParamPropGroupOpen,0,open?1:0);I(p,kOfxParamPropAnimates,0,0);return p;}
static OfxPropertySetHandle defineSlider(OfxParamSetHandle ps,const char* id,const char* label,const char* parent,double def,double mn,double mx,double step,int digits,const char* hint){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeDouble,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxPropShortLabel,0,label);S(p,kOfxParamPropScriptName,0,id);if(parent)S(p,kOfxParamPropParent,0,parent);D(p,kOfxParamPropDefault,0,def);D(p,kOfxParamPropMin,0,mn);D(p,kOfxParamPropMax,0,mx);D(p,kOfxParamPropDisplayMin,0,mn);D(p,kOfxParamPropDisplayMax,0,mx);D(p,kOfxParamPropIncrement,0,step);I(p,kOfxParamPropDigits,0,digits);if(hint)S(p,kOfxParamPropHint,0,hint);return p;}
static OfxPropertySetHandle defineChoice(OfxParamSetHandle ps,const char* id,const char* label,const char* parent,int def,const std::vector<const char*>& opts,const char* hint=nullptr){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeChoice,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxPropShortLabel,0,label);S(p,kOfxParamPropScriptName,0,id);if(parent)S(p,kOfxParamPropParent,0,parent);I(p,kOfxParamPropDefault,0,def);for(size_t i=0;i<opts.size();++i)S(p,kOfxParamPropChoiceOption,(int)i,opts[i]);if(hint)S(p,kOfxParamPropHint,0,hint);return p;}
static OfxPropertySetHandle defineButton(OfxParamSetHandle ps,const char* id,const char* label,const char* parent,const char* hint){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypePushButton,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxParamPropScriptName,0,id);if(parent)S(p,kOfxParamPropParent,0,parent);I(p,kOfxParamPropAnimates,0,0);if(hint)S(p,kOfxParamPropHint,0,hint);return p;}
static OfxPropertySetHandle defineHiddenDouble(OfxParamSetHandle ps,const char* id,double def){auto p=defineSlider(ps,id,id,nullptr,def,-16.0,16.0,0.000001,6,nullptr);if(p){I(p,kOfxParamPropSecret,0,1);I(p,kOfxParamPropAnimates,0,0);I(p,kOfxParamPropPersistant,0,1);}return p;}
static OfxPropertySetHandle defineHiddenBool(OfxParamSetHandle ps,const char* id,int def){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeBoolean,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxParamPropScriptName,0,id);I(p,kOfxParamPropDefault,0,def);I(p,kOfxParamPropSecret,0,1);I(p,kOfxParamPropAnimates,0,0);I(p,kOfxParamPropPersistant,0,1);return p;}

static OfxStatus describe(OfxImageEffectHandle e){if(!gProp||!gEffect||!gParam)return kOfxStatErrMissingHostFeature;OfxPropertySetHandle p=nullptr;gEffect->getPropertySet(e,&p);S(p,kOfxPropLabel,0,"Keystone v1.6 Region Grade");S(p,kOfxPropShortLabel,0,"Keystone v1.6 Region Grade");S(p,kOfxPropLongLabel,0,"Keystone v1.6 Region Grade");S(p,kOfxPropVersionLabel,0,"1.6");S(p,kOfxPropPluginDescription,0,"ARRI AWG4/LogC4 grading pipeline with one-frame Auto Match, Region Grade, and Keystone output transform.");S(p,kOfxImageEffectPropSupportedContexts,0,kOfxImageEffectContextFilter);S(p,kOfxImageEffectPropSupportedPixelDepths,0,kOfxBitDepthFloat);I(p,kOfxImageEffectPropSupportsTiles,0,0);I(p,kOfxImageEffectPropSupportsMultiResolution,0,1);I(p,kOfxImageEffectPropSupportsMultipleClipDepths,0,0);I(p,kOfxImageEffectPropTemporalClipAccess,0,0);I(p,kOfxImageEffectPropRenderTwiceAlways,0,0);return kOfxStatOK;}

static OfxStatus describeInContext(OfxImageEffectHandle e){
  OfxPropertySetHandle p=nullptr;gEffect->clipDefine(e,kOfxImageEffectSimpleSourceClipName,&p);S(p,kOfxImageEffectPropSupportedPixelDepths,0,kOfxBitDepthFloat);S(p,kOfxImageEffectPropComponents,0,kOfxImageComponentRGBA);S(p,kOfxImageClipPropFieldOrder,0,kOfxImageFieldNone);
  gEffect->clipDefine(e,kOfxImageEffectOutputClipName,&p);S(p,kOfxImageEffectPropSupportedPixelDepths,0,kOfxBitDepthFloat);S(p,kOfxImageEffectPropComponents,0,kOfxImageComponentRGBA);S(p,kOfxImageClipPropFieldOrder,0,kOfxImageFieldNone);
  OfxParamSetHandle ps=nullptr;gEffect->getParamSet(e,&ps);
  defineGroup(ps,"grpFilter","Input / Filters",false);defineGroup(ps,"grpNeutral","Auto Match",true);defineGroup(ps,"grpWB","White Balance",false);defineGroup(ps,"grpTone","Tone",false);defineGroup(ps,"grpColor","Color",false);defineGroup(ps,"grpRegion","Region Grade",true);defineGroup(ps,"grpFilm","Film / Finish",false);defineGroup(ps,"grpSplit","Split Tone",false);defineGroup(ps,"grpLook","Look",false);defineGroup(ps,"grpSkin","Skin",false);
  defineButton(ps,"analyzeNeutral","Analyze Match","grpNeutral","Analyze the current frame once and store a persistent neutral correction for this plugin instance.");
  defineButton(ps,"resetNeutral","Reset Neutral","grpNeutral","Clear the stored analysis and return the neutral correction to identity.");
  defineSlider(ps,"neutralAmount","Amount","grpNeutral",1.0,0.0,1.0,0.01,2,"Strength of the stored one-frame neutral correction. 0 is exact bypass.");
  defineHiddenDouble(ps,"neutralGainR",1.0);defineHiddenDouble(ps,"neutralGainG",1.0);defineHiddenDouble(ps,"neutralGainB",1.0);defineHiddenDouble(ps,"matchSlopeR",1.0);defineHiddenDouble(ps,"matchSlopeG",1.0);defineHiddenDouble(ps,"matchSlopeB",1.0);defineHiddenDouble(ps,"matchOffsetR",0.0);defineHiddenDouble(ps,"matchOffsetG",0.0);defineHiddenDouble(ps,"matchOffsetB",0.0);defineHiddenDouble(ps,"neutralConfidence",0.0);defineHiddenBool(ps,"neutralValid",0);
  defineChoice(ps,"ndFilter","ND","grpFilter",0,{"Off","ND2","ND4","ND8","ND16","ND32","ND64","ND100","ND200","ND400","ND500","ND1000"});
  defineChoice(ps,"uvFilter","UV Cut","grpFilter",0,{"Off","On"},"Keystone camera-side UV cutoff behavior projected into Keystone's AWG4 RGB pipeline.");
  defineSlider(ps,"uvCutNm","UV Cut nm","grpFilter",410.0,350.0,500.0,1.0,0,"Cutoff center. Keystone source default is 410 nm.");
  defineChoice(ps,"irFilter","IR Cut","grpFilter",0,{"Off","On"},"Keystone camera-side IR cutoff behavior projected into Keystone's AWG4 RGB pipeline.");
  defineSlider(ps,"irCutNm","IR Cut nm","grpFilter",675.0,600.0,800.0,1.0,0,"Cutoff center. Keystone source default is 675 nm.");
  defineSlider(ps,"wbTemp","Temp","grpWB",0,-100,100,0.1,1,"Warm/cool opponent white balance in linear AWG4.");defineSlider(ps,"wbTint","Tint","grpWB",0,-100,100,0.1,1,"Magenta/green opponent white balance in linear AWG4.");
  defineSlider(ps,"exposure","Exposure","grpTone",0,-6,6,0.01,2,nullptr);defineSlider(ps,"blackPoint","Black Pt","grpTone",0,-.05,.05,.0001,4,nullptr);defineSlider(ps,"contrast","Contrast","grpTone",1,.5,2,.001,3,nullptr);defineSlider(ps,"shadows","Shadows","grpTone",0,-1,1,.01,2,nullptr);defineSlider(ps,"highlights","Highlights","grpTone",0,-1,1,.01,2,nullptr);defineSlider(ps,"roll","Roll","grpTone",0,0,2,.01,2,nullptr);
  defineChoice(ps,"curvePreset","Curve","grpTone",0,{"Off","Neutral","Latitude","Punch","Chrome"});
  defineSlider(ps,"density","Density","grpColor",0,0,1,.001,3,nullptr);defineSlider(ps,"posSat","Pos Sat","grpColor",0,0,1,.001,3,nullptr);defineSlider(ps,"interlayer","Interlayer","grpColor",0,0,3,.01,2,nullptr);defineSlider(ps,"satSplit","Sat Split","grpColor",1,.5,2,.001,3,nullptr);
  defineChoice(ps,"regionEnable","Enable","grpRegion",0,{"Off","On"},"Enable Keystone-native selective region grading.");
  defineChoice(ps,"regionTarget","Region","grpRegion",0,{"Subject","Background","Sky","Foliage / Trees","Water","Ground","Terrain","Built Environment"},"Choose the region to grade.");
  defineSlider(ps,"regionExposure","Exposure","grpRegion",0,-4,4,.01,2,"Darken or brighten only the selected region.");
  defineSlider(ps,"regionContrast","Contrast","grpRegion",1,.5,2,.001,3,"Local luminance contrast for the selected region.");
  defineSlider(ps,"regionSaturation","Saturation","grpRegion",1,0,2,.001,3,"Local saturation for the selected region.");
  defineSlider(ps,"regionTemp","Temperature","grpRegion",0,-100,100,.1,1,"Warm or cool only the selected region.");
  defineSlider(ps,"regionAmount","Amount","grpRegion",1,0,1,.001,3,"Mix amount for the local grade.");
  defineSlider(ps,"regionFeather","Feather","grpRegion",.25,0,1,.001,3,"Softens the region selection and spatial boundaries.");
  defineChoice(ps,"regionShowMask","Show Mask","grpRegion",0,{"Off","On"},"Preview the selected region mask.");
  defineSlider(ps,"hiBleach","Hi Bleach","grpFilm",0,0,1,.001,3,nullptr);defineSlider(ps,"loBleach","Lo Bleach","grpFilm",0,0,1,.001,3,nullptr);defineSlider(ps,"colorBleach","Bleach","grpFilm",0,0,80,.1,1,nullptr);defineSlider(ps,"fade","Fade","grpFilm",0,0,100,.1,1,nullptr);
  defineSlider(ps,"splitAmount","Amount","grpSplit",0,0,2,.001,3,nullptr);defineSlider(ps,"splitShadowHue","Shadow Hue","grpSplit",220,0,360,.1,1,nullptr);defineSlider(ps,"splitHighlightHue","Highlight Hue","grpSplit",40,0,360,.1,1,nullptr);defineSlider(ps,"splitBalance","Balance","grpSplit",.5,0,1,.001,3,nullptr);defineSlider(ps,"splitSubtractive","Subtractive","grpSplit",.5,0,1,.001,3,nullptr);
  defineChoice(ps,"lookColor","Color","grpLook",0,{"Off","Warm Cool","Cool Warm","Teal Orange","Sepia","Bleach Cool","Dusk Purple","Cyan Rose","Green Gold","Purple Gold","CHI","Island"});defineSlider(ps,"lookAmount","Amount","grpLook",1,0,1.5,.01,2,nullptr);defineChoice(ps,"creativeWhite","Creative White","grpLook",2,{"D93","D75","D65","D60","D55","D50"});
  defineChoice(ps,"skinEnable","Enable","grpSkin",0,{"Off","On"},"Enable the Keystone Skin Tones block.");
  defineChoice(ps,"skinPreset","Preset","grpSkin",0,{"Custom","Vibrant","Muted","Cool / Neutralize Red","Warm / Add Warmth","Matte / Reduce Shine"},"Keystone Skin Tones preset family.");
  defineSlider(ps,"skinSaturate","Saturate Skin","grpSkin",0.2,-1.0,1.0,0.01,2,"Keystone skin saturation control.");
  defineSlider(ps,"skinColour","Skin Luminance","grpSkin",0.0,-1.0,1.0,0.01,2,"Keystone skin luminance / density control.");
  defineSlider(ps,"skinPop","Skin Tone","grpSkin",0.0,-60.0,60.0,0.5,1,"Keystone skin hue rotation in degrees.");
  defineSlider(ps,"skinCenter","Tone Center","grpSkin",0.0,-30.0,30.0,0.5,1,"Skin tone selection center offset.");
  defineSlider(ps,"skinRange","Tone Range","grpSkin",40.0,15.0,65.0,0.5,1,"Skin tone selection range.");
  defineSlider(ps,"skinBrightness","Brightness Center","grpSkin",0.5,0.0,1.0,0.01,2,"Luma selection center.");
  defineSlider(ps,"skinBrightnessRange","Brightness Range","grpSkin",1.0,0.25,1.75,0.01,2,"Luma selection range.");
  defineChoice(ps,"skinShowMask","Show Mask","grpSkin",0,{"Off","On"},"Preview the Keystone-style skin selection mask.");
  defineSlider(ps,"skinIntensity","Intensity","grpSkin",100.0,0.0,200.0,1.0,0,"Skin tones intensity.");
  return kOfxStatOK;
}

static InstanceData* dataFor(OfxImageEffectHandle e){OfxPropertySetHandle p=nullptr;if(!gEffect||gEffect->getPropertySet(e,&p)!=kOfxStatOK)return nullptr;void* v=nullptr;if(gProp->propGetPointer(p,kOfxPropInstanceData,0,&v)!=kOfxStatOK)return nullptr;return (InstanceData*)v;}
static void getH(OfxParamSetHandle ps,const char* id,OfxParamHandle& h){OfxPropertySetHandle d=nullptr;gParam->paramGetHandle(ps,id,&h,&d);}
static OfxStatus createInstance(OfxImageEffectHandle e){auto*d=new InstanceData;OfxPropertySetHandle props=nullptr;gEffect->getPropertySet(e,&props);P(props,kOfxPropInstanceData,0,d);OfxPropertySetHandle dummy=nullptr;gEffect->clipGetHandle(e,kOfxImageEffectSimpleSourceClipName,&d->source,&dummy);gEffect->clipGetHandle(e,kOfxImageEffectOutputClipName,&d->output,&dummy);OfxParamSetHandle ps=nullptr;gEffect->getParamSet(e,&ps);
getH(ps,"analyzeNeutral",d->analyze);getH(ps,"resetNeutral",d->resetNeutral);getH(ps,"neutralAmount",d->neutralAmount);getH(ps,"neutralGainR",d->neutralGainR);getH(ps,"neutralGainG",d->neutralGainG);getH(ps,"neutralGainB",d->neutralGainB);getH(ps,"matchSlopeR",d->matchSlopeR);getH(ps,"matchSlopeG",d->matchSlopeG);getH(ps,"matchSlopeB",d->matchSlopeB);getH(ps,"matchOffsetR",d->matchOffsetR);getH(ps,"matchOffsetG",d->matchOffsetG);getH(ps,"matchOffsetB",d->matchOffsetB);getH(ps,"neutralValid",d->neutralValid);getH(ps,"neutralConfidence",d->neutralConfidence);
#define H(field,id) getH(ps,id,d->field)
  H(ndFilter,"ndFilter");H(uvFilter,"uvFilter");H(uvCutNm,"uvCutNm");H(irFilter,"irFilter");H(irCutNm,"irCutNm");H(wbTemp,"wbTemp");H(wbTint,"wbTint");H(exposure,"exposure");H(blackPoint,"blackPoint");H(contrast,"contrast");H(shadows,"shadows");H(highlights,"highlights");H(roll,"roll");H(curvePreset,"curvePreset");H(density,"density");H(posSat,"posSat");H(interlayer,"interlayer");H(satSplit,"satSplit");H(regionEnable,"regionEnable");H(regionTarget,"regionTarget");H(regionExposure,"regionExposure");H(regionContrast,"regionContrast");H(regionSaturation,"regionSaturation");H(regionTemp,"regionTemp");H(regionAmount,"regionAmount");H(regionFeather,"regionFeather");H(regionShowMask,"regionShowMask");H(splitAmount,"splitAmount");H(splitShadowHue,"splitShadowHue");H(splitHighlightHue,"splitHighlightHue");H(splitBalance,"splitBalance");H(splitSubtractive,"splitSubtractive");H(hiBleach,"hiBleach");H(loBleach,"loBleach");H(colorBleach,"colorBleach");H(fade,"fade");H(lookColor,"lookColor");H(lookAmount,"lookAmount");H(creativeWhite,"creativeWhite");H(skinEnable,"skinEnable");H(skinPreset,"skinPreset");H(skinSaturate,"skinSaturate");H(skinColour,"skinColour");H(skinPop,"skinPop");H(skinCenter,"skinCenter");H(skinRange,"skinRange");H(skinBrightness,"skinBrightness");H(skinBrightnessRange,"skinBrightnessRange");H(skinShowMask,"skinShowMask");H(skinIntensity,"skinIntensity");
#undef H
  std::string why;loadCube33(bundledOutputTransformPath(),d->cpuLut,&why);return kOfxStatOK;}
static OfxStatus destroyInstance(OfxImageEffectHandle e){auto*d=dataFor(e);delete d;OfxPropertySetHandle p=nullptr;gEffect->getPropertySet(e,&p);P(p,kOfxPropInstanceData,0,nullptr);return kOfxStatOK;}
static double gd(OfxParamHandle h,double t,double def){double v=def;if(h&&gParam->paramGetValueAtTime(h,t,&v)==kOfxStatOK)return v;return def;}static int gi(OfxParamHandle h,double t,int def){int v=def;if(h&&gParam->paramGetValueAtTime(h,t,&v)==kOfxStatOK)return v;return def;}
static Params fetchParams(InstanceData*d,double t){Params p;p.neutralAmount=(float)gd(d->neutralAmount,t,1);p.neutralGainR=(float)gd(d->neutralGainR,t,1);p.neutralGainG=(float)gd(d->neutralGainG,t,1);p.neutralGainB=(float)gd(d->neutralGainB,t,1);p.matchSlopeR=(float)gd(d->matchSlopeR,t,1);p.matchSlopeG=(float)gd(d->matchSlopeG,t,1);p.matchSlopeB=(float)gd(d->matchSlopeB,t,1);p.matchOffsetR=(float)gd(d->matchOffsetR,t,0);p.matchOffsetG=(float)gd(d->matchOffsetG,t,0);p.matchOffsetB=(float)gd(d->matchOffsetB,t,0);p.ndFilter=gi(d->ndFilter,t,0);p.uvFilter=gi(d->uvFilter,t,0);p.uvCutNm=(float)gd(d->uvCutNm,t,410);p.irFilter=gi(d->irFilter,t,0);p.irCutNm=(float)gd(d->irCutNm,t,675);p.wbTemp=(float)gd(d->wbTemp,t,0);p.wbTint=(float)gd(d->wbTint,t,0);p.exposure=(float)gd(d->exposure,t,0);p.blackPoint=(float)gd(d->blackPoint,t,0);p.contrast=(float)gd(d->contrast,t,1);p.shadows=(float)gd(d->shadows,t,0);p.highlights=(float)gd(d->highlights,t,0);p.roll=(float)gd(d->roll,t,0);p.curvePreset=gi(d->curvePreset,t,0);p.density=(float)gd(d->density,t,0);p.posSat=(float)gd(d->posSat,t,0);p.interlayer=(float)gd(d->interlayer,t,0);p.satSplit=(float)gd(d->satSplit,t,1);p.regionEnable=gi(d->regionEnable,t,0);p.regionTarget=gi(d->regionTarget,t,0);p.regionExposure=(float)gd(d->regionExposure,t,0);p.regionContrast=(float)gd(d->regionContrast,t,1);p.regionSaturation=(float)gd(d->regionSaturation,t,1);p.regionTemp=(float)gd(d->regionTemp,t,0);p.regionAmount=(float)gd(d->regionAmount,t,1);p.regionFeather=(float)gd(d->regionFeather,t,.25);p.regionShowMask=gi(d->regionShowMask,t,0);p.splitAmount=(float)gd(d->splitAmount,t,0);p.splitShadowHue=(float)gd(d->splitShadowHue,t,220);p.splitHighlightHue=(float)gd(d->splitHighlightHue,t,40);p.splitBalance=(float)gd(d->splitBalance,t,.5);p.splitSubtractive=(float)gd(d->splitSubtractive,t,.5);p.hiBleach=(float)gd(d->hiBleach,t,0);p.loBleach=(float)gd(d->loBleach,t,0);p.colorBleach=(float)gd(d->colorBleach,t,0);p.fade=(float)gd(d->fade,t,0);p.lookColor=gi(d->lookColor,t,0);p.lookAmount=(float)gd(d->lookAmount,t,1);p.creativeWhite=gi(d->creativeWhite,t,2);p.skinEnable=gi(d->skinEnable,t,0);p.skinPreset=gi(d->skinPreset,t,0);p.skinSaturate=(float)gd(d->skinSaturate,t,.2);p.skinColour=(float)gd(d->skinColour,t,0);p.skinPop=(float)gd(d->skinPop,t,0);p.skinCenter=(float)gd(d->skinCenter,t,0);p.skinRange=(float)gd(d->skinRange,t,40);p.skinBrightness=(float)gd(d->skinBrightness,t,.5);p.skinBrightnessRange=(float)gd(d->skinBrightnessRange,t,1);p.skinShowMask=gi(d->skinShowMask,t,0);p.skinIntensity=(float)gd(d->skinIntensity,t,100);return p;}

static OfxStatus instanceChanged(OfxImageEffectHandle e,OfxPropertySetHandle inArgs){auto*d=dataFor(e);if(!d)return kOfxStatReplyDefault;char* reason=nullptr;char* type=nullptr;char* name=nullptr;if(gProp->propGetString(inArgs,kOfxPropChangeReason,0,&reason)!=kOfxStatOK||!reason||std::strcmp(reason,kOfxChangeUserEdited)!=0)return kOfxStatReplyDefault;if(gProp->propGetString(inArgs,kOfxPropType,0,&type)!=kOfxStatOK||!type||std::strcmp(type,kOfxTypeParameter)!=0)return kOfxStatReplyDefault;if(gProp->propGetString(inArgs,kOfxPropName,0,&name)!=kOfxStatOK||!name)return kOfxStatReplyDefault;
  if(!std::strcmp(name,"resetNeutral")){gParam->paramSetValue(d->neutralGainR,1.0);gParam->paramSetValue(d->neutralGainG,1.0);gParam->paramSetValue(d->neutralGainB,1.0);gParam->paramSetValue(d->matchSlopeR,1.0);gParam->paramSetValue(d->matchSlopeG,1.0);gParam->paramSetValue(d->matchSlopeB,1.0);gParam->paramSetValue(d->matchOffsetR,0.0);gParam->paramSetValue(d->matchOffsetG,0.0);gParam->paramSetValue(d->matchOffsetB,0.0);gParam->paramSetValue(d->neutralConfidence,0.0);gParam->paramSetValue(d->neutralValid,0);return kOfxStatOK;}
  if(!std::strcmp(name,"skinPreset")){
    int sp=0; if(d->skinPreset)gParam->paramGetValue(d->skinPreset,&sp);
    double sat=.2,lum=0,hue=0,center=0,range=40,bc=.5,br=1.0,intensity=100;
    if(sp==1){sat=.55;lum=.04;range=42;intensity=110;}              // Vibrant
    else if(sp==2){sat=-.25;lum=-.02;range=44;intensity=100;}       // Muted
    else if(sp==3){sat=.12;hue=10;center=2;range=40;intensity=100;} // Cool / Neutralize Red
    else if(sp==4){sat=.28;hue=-8;center=-2;range=42;intensity=100;}// Warm / Add Warmth
    else if(sp==5){sat=.05;lum=-.16;range=38;bc=.60;br=.70;intensity=100;} // Matte / Reduce Shine
    if(sp>0){gParam->paramSetValue(d->skinEnable,1);gParam->paramSetValue(d->skinSaturate,sat);gParam->paramSetValue(d->skinColour,lum);gParam->paramSetValue(d->skinPop,hue);gParam->paramSetValue(d->skinCenter,center);gParam->paramSetValue(d->skinRange,range);gParam->paramSetValue(d->skinBrightness,bc);gParam->paramSetValue(d->skinBrightnessRange,br);gParam->paramSetValue(d->skinIntensity,intensity);}return kOfxStatOK;
  }
  if(std::strcmp(name,"analyzeNeutral"))return kOfxStatReplyDefault;
  double time=0.0;gProp->propGetDouble(inArgs,kOfxPropTime,0,&time);OfxPropertySetHandle img=nullptr;if(gEffect->clipGetImage(d->source,time,nullptr,&img)!=kOfxStatOK||!img)return kOfxStatFailed;void* raw=nullptr;int rb=0,b[4]={0,0,0,0};gProp->propGetPointer(img,kOfxImagePropData,0,&raw);gProp->propGetInt(img,kOfxImagePropRowBytes,0,&rb);gProp->propGetIntN(img,kOfxImagePropBounds,4,b);int w=b[2]-b[0],h=b[3]-b[1];Params p=fetchParams(d,time);p.neutralAmount=0;p.neutralGainR=p.neutralGainG=p.neutralGainB=1;NeutralResult r;if(raw&&rb!=0)r=analyzeNeutralRGBA((const float*)raw,w,h,rb/(int)sizeof(float),p);gEffect->clipReleaseImage(img);if(!r.valid)return kOfxStatFailed;gParam->paramSetValue(d->neutralGainR,(double)r.gainR);gParam->paramSetValue(d->neutralGainG,(double)r.gainG);gParam->paramSetValue(d->neutralGainB,(double)r.gainB);gParam->paramSetValue(d->matchSlopeR,(double)r.slopeR);gParam->paramSetValue(d->matchSlopeG,(double)r.slopeG);gParam->paramSetValue(d->matchSlopeB,(double)r.slopeB);gParam->paramSetValue(d->matchOffsetR,(double)r.offsetR);gParam->paramSetValue(d->matchOffsetG,(double)r.offsetG);gParam->paramSetValue(d->matchOffsetB,(double)r.offsetB);gParam->paramSetValue(d->neutralConfidence,(double)r.confidence);gParam->paramSetValue(d->neutralValid,1);return kOfxStatOK;}

static OfxStatus render(OfxImageEffectHandle e,OfxPropertySetHandle inArgs){auto*d=dataFor(e);if(!d||!d->source||!d->output)return kOfxStatFailed;double time=0;gProp->propGetDouble(inArgs,kOfxPropTime,0,&time);OfxPropertySetHandle si=nullptr,di=nullptr;auto s1=gEffect->clipGetImage(d->source,time,nullptr,&si),s2=gEffect->clipGetImage(d->output,time,nullptr,&di);if(s1!=kOfxStatOK||s2!=kOfxStatOK||!si||!di){if(si)gEffect->clipReleaseImage(si);if(di)gEffect->clipReleaseImage(di);return kOfxStatFailed;}void* sp=nullptr,*dp=nullptr;int srb=0,drb=0,b[4]={};gProp->propGetPointer(si,kOfxImagePropData,0,&sp);gProp->propGetPointer(di,kOfxImagePropData,0,&dp);gProp->propGetInt(si,kOfxImagePropRowBytes,0,&srb);gProp->propGetInt(di,kOfxImagePropRowBytes,0,&drb);gProp->propGetIntN(di,kOfxImagePropBounds,4,b);int w=b[2]-b[0],h=b[3]-b[1],ss=srb/(int)sizeof(float),ds=drb/(int)sizeof(float);Params p=fetchParams(d,time);if(d->cpuLut.size()!=35937){std::string why;if(!loadCube33(bundledOutputTransformPath(),d->cpuLut,&why)){gEffect->clipReleaseImage(si);gEffect->clipReleaseImage(di);return kOfxStatFailed;}}processRGBA((const float*)sp,(float*)dp,w,h,ss,ds,p,d->cpuLut);gEffect->clipReleaseImage(si);gEffect->clipReleaseImage(di);return kOfxStatOK;}
static OfxStatus rod(OfxImageEffectHandle e,OfxPropertySetHandle inArgs,OfxPropertySetHandle outArgs){auto*d=dataFor(e);if(!d||!d->source)return kOfxStatReplyDefault;double t=0;gProp->propGetDouble(inArgs,kOfxPropTime,0,&t);OfxRectD r{};if(gEffect->clipGetRegionOfDefinition(d->source,t,&r)==kOfxStatOK){double v[4]={r.x1,r.y1,r.x2,r.y2};gProp->propSetDoubleN(outArgs,kOfxImageEffectPropRegionOfDefinition,4,v);return kOfxStatOK;}return kOfxStatReplyDefault;}
static OfxStatus mainEntry(const char*a,const void*h,OfxPropertySetHandle in,OfxPropertySetHandle out){
  auto e=(OfxImageEffectHandle)h;
  if(!a)return kOfxStatErrUnknown;
  if(!std::strcmp(a,kOfxActionLoad))return onLoad();
  if(!std::strcmp(a,kOfxActionUnload))return onUnload();
  if(!std::strcmp(a,kOfxActionDescribe))return describe(e);
  if(!std::strcmp(a,kOfxImageEffectActionDescribeInContext))return describeInContext(e);
  if(!std::strcmp(a,kOfxActionCreateInstance))return createInstance(e);
  if(!std::strcmp(a,kOfxActionDestroyInstance))return destroyInstance(e);
  if(!std::strcmp(a,kOfxActionInstanceChanged))return instanceChanged(e,in);
  if(!std::strcmp(a,kOfxImageEffectActionRender))return render(e,in);
  if(!std::strcmp(a,kOfxImageEffectActionGetRegionOfDefinition))return rod(e,in,out);
  return kOfxStatReplyDefault;
}
static OfxPlugin gPlugin={kOfxImageEffectPluginApi,1,"com.luma.keystoneofx",1,6,setHostFunc,mainEntry};
extern "C" {
  __attribute__((visibility("default"))) int OfxGetNumberOfPlugins(){return 1;}
  __attribute__((visibility("default"))) OfxPlugin* OfxGetPlugin(int n){return n==0?&gPlugin:nullptr;}
}
