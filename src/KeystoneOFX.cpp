#include "OpenFXMinimal.h"
#include "KeystoneParams.h"
#include "NeutralAnalyzer.h"
#include "KeystoneCPU.h"
#include "LutLoader.h"
#include "MetalBridge.h"
#include "ColorGradrBridge.h"
#include "SceneGrade.h"
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

using namespace keystone;
static OfxHost* gHost=nullptr;static OfxPropertySuiteV1* gProp=nullptr;static OfxImageEffectSuiteV1* gEffect=nullptr;static OfxParameterSuiteV1* gParam=nullptr;

struct InstanceData {
  OfxImageClipHandle source=nullptr,output=nullptr;
  OfxParamHandle analyze=nullptr,resetNeutral=nullptr,matchOnly=nullptr,neutralAmount=nullptr,neutralGainR=nullptr,neutralGainG=nullptr,neutralGainB=nullptr,matchSlopeR=nullptr,matchSlopeG=nullptr,matchSlopeB=nullptr,matchOffsetR=nullptr,matchOffsetG=nullptr,matchOffsetB=nullptr,neutralValid=nullptr,neutralConfidence=nullptr;
  OfxParamHandle sceneAnalyze=nullptr,sceneReset=nullptr,sceneSubject=nullptr,sceneSeparation=nullptr,sceneBias=nullptr,sceneStatus=nullptr,sceneGainTemp=nullptr,sceneOffsetTemp=nullptr,sceneExposure=nullptr,sceneShadows=nullptr,sceneHighlights=nullptr,sceneBase=nullptr,sceneMode=nullptr,sceneResolved=nullptr,sceneValid=nullptr;
  OfxParamHandle ndFilter=nullptr,uvFilter=nullptr,uvCutNm=nullptr,irFilter=nullptr,irCutNm=nullptr,wbTemp=nullptr,wbTint=nullptr,exposure=nullptr,blackPoint=nullptr,contrast=nullptr,shadows=nullptr,highlights=nullptr,roll=nullptr,curvePreset=nullptr;
  OfxParamHandle density=nullptr,posSat=nullptr,interlayer=nullptr,satSplit=nullptr,splitAmount=nullptr,splitShadowHue=nullptr,splitHighlightHue=nullptr,splitBalance=nullptr,splitSubtractive=nullptr;
  OfxParamHandle hiBleach=nullptr,loBleach=nullptr,colorBleach=nullptr,fade=nullptr,lookColor=nullptr,lookAmount=nullptr,creativeWhite=nullptr,skinEnable=nullptr,skinPreset=nullptr,skinSaturate=nullptr,skinColour=nullptr,skinPop=nullptr,skinCenter=nullptr,skinRange=nullptr,skinBrightness=nullptr,skinBrightnessRange=nullptr,skinShowMask=nullptr,skinIntensity=nullptr;
  std::vector<LutEntry> cpuLut;
};

static void setHostFunc(void* h){gHost=static_cast<OfxHost*>(h);if(!gHost||!gHost->fetchSuite)return;gProp=(OfxPropertySuiteV1*)gHost->fetchSuite(gHost->host,kOfxPropertySuite,1);gEffect=(OfxImageEffectSuiteV1*)gHost->fetchSuite(gHost->host,kOfxImageEffectSuite,1);gParam=(OfxParameterSuiteV1*)gHost->fetchSuite(gHost->host,kOfxParameterSuite,1);colorgradr_bridge::setHost(h);}
static void S(OfxPropertySetHandle h,const char*n,int i,const char*v){if(gProp)gProp->propSetString(h,n,i,v);}static void I(OfxPropertySetHandle h,const char*n,int i,int v){if(gProp)gProp->propSetInt(h,n,i,v);}static void D(OfxPropertySetHandle h,const char*n,int i,double v){if(gProp)gProp->propSetDouble(h,n,i,v);}static void P(OfxPropertySetHandle h,const char*n,int i,void*v){if(gProp)gProp->propSetPointer(h,n,i,v);}

static OfxPropertySetHandle defineGroup(OfxParamSetHandle ps,const char* id,const char* label,bool open){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeGroup,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxParamPropScriptName,0,id);I(p,kOfxParamPropGroupOpen,0,open?1:0);I(p,kOfxParamPropAnimates,0,0);return p;}
static OfxPropertySetHandle defineSlider(OfxParamSetHandle ps,const char* id,const char* label,const char* parent,double def,double mn,double mx,double step,int digits,const char* hint){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeDouble,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxPropShortLabel,0,label);S(p,kOfxParamPropScriptName,0,id);if(parent)S(p,kOfxParamPropParent,0,parent);D(p,kOfxParamPropDefault,0,def);D(p,kOfxParamPropMin,0,mn);D(p,kOfxParamPropMax,0,mx);D(p,kOfxParamPropDisplayMin,0,mn);D(p,kOfxParamPropDisplayMax,0,mx);D(p,kOfxParamPropIncrement,0,step);I(p,kOfxParamPropDigits,0,digits);if(hint)S(p,kOfxParamPropHint,0,hint);return p;}
static OfxPropertySetHandle defineChoice(OfxParamSetHandle ps,const char* id,const char* label,const char* parent,int def,const std::vector<const char*>& opts,const char* hint=nullptr){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeChoice,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxPropShortLabel,0,label);S(p,kOfxParamPropScriptName,0,id);if(parent)S(p,kOfxParamPropParent,0,parent);I(p,kOfxParamPropDefault,0,def);for(size_t i=0;i<opts.size();++i)S(p,kOfxParamPropChoiceOption,(int)i,opts[i]);if(hint)S(p,kOfxParamPropHint,0,hint);return p;}
static OfxPropertySetHandle defineButton(OfxParamSetHandle ps,const char* id,const char* label,const char* parent,const char* hint){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypePushButton,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxParamPropScriptName,0,id);if(parent)S(p,kOfxParamPropParent,0,parent);I(p,kOfxParamPropAnimates,0,0);if(hint)S(p,kOfxParamPropHint,0,hint);return p;}
static OfxPropertySetHandle defineString(OfxParamSetHandle ps,const char* id,const char* label,const char* parent,const char* def){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeString,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxPropLabel,0,label);S(p,kOfxParamPropScriptName,0,id);if(parent)S(p,kOfxParamPropParent,0,parent);S(p,kOfxParamPropDefault,0,def?def:"");I(p,kOfxParamPropAnimates,0,0);I(p,kOfxParamPropEnabled,0,0);return p;}
static OfxPropertySetHandle defineHiddenDouble(OfxParamSetHandle ps,const char* id,double def){auto p=defineSlider(ps,id,id,nullptr,def,-16.0,16.0,0.000001,6,nullptr);if(p){I(p,kOfxParamPropSecret,0,1);I(p,kOfxParamPropAnimates,0,0);I(p,kOfxParamPropPersistant,0,1);}return p;}
static OfxPropertySetHandle defineHiddenBool(OfxParamSetHandle ps,const char* id,int def){OfxPropertySetHandle p=nullptr;if(gParam->paramDefine(ps,kOfxParamTypeBoolean,id,&p)!=kOfxStatOK)return nullptr;S(p,kOfxParamPropScriptName,0,id);I(p,kOfxParamPropDefault,0,def);I(p,kOfxParamPropSecret,0,1);I(p,kOfxParamPropAnimates,0,0);I(p,kOfxParamPropPersistant,0,1);return p;}

static OfxStatus describe(OfxImageEffectHandle e){if(!gProp||!gEffect||!gParam)return kOfxStatErrMissingHostFeature;OfxPropertySetHandle p=nullptr;gEffect->getPropertySet(e,&p);S(p,kOfxPropLabel,0,"Keystone v1.6.1");S(p,kOfxPropShortLabel,0,"Keystone v1.6.1");S(p,kOfxPropLongLabel,0,"Keystone v1.6.1");S(p,kOfxPropVersionLabel,0,"1.6.1");S(p,kOfxPropPluginDescription,0,"ARRI AWG4/LogC4 grading pipeline using the original ColorGradr OFX Fix engine before Keystone grading.");S(p,kOfxImageEffectPropSupportedContexts,0,kOfxImageEffectContextFilter);S(p,kOfxImageEffectPropSupportedContexts,1,kOfxImageEffectContextGeneral);S(p,kOfxImageEffectPropSupportedPixelDepths,0,kOfxBitDepthFloat);I(p,kOfxImageEffectPropSupportsTiles,0,0);I(p,kOfxImageEffectPropSupportsMultiResolution,0,1);I(p,kOfxImageEffectPropSupportsMultipleClipDepths,0,0);I(p,kOfxImageEffectPropTemporalClipAccess,0,0);I(p,kOfxImageEffectPropRenderTwiceAlways,0,0);S(p,kOfxImageEffectPropMetalRenderSupported,0,"true");return kOfxStatOK;}

static OfxStatus describeInContext(OfxImageEffectHandle e){
  OfxParamSetHandle ps=nullptr;gEffect->getParamSet(e,&ps);
  defineGroup(ps,"grpFilter","Input / Filters",false);defineGroup(ps,"grpNeutral","Auto Match",true);defineGroup(ps,"grpScene","Scene Grade",true);defineGroup(ps,"grpWB","White Balance",false);defineGroup(ps,"grpTone","Tone",false);defineGroup(ps,"grpColor","Color",false);defineGroup(ps,"grpFilm","Film / Finish",false);defineGroup(ps,"grpSplit","Split Tone",false);defineGroup(ps,"grpLook","Look",false);defineGroup(ps,"grpSkin","Skin",false);
  // ColorGradr owns the actual Fix button and transform state. Re-label its native
  // control and place it in Keystone's Auto Match group; no reconstructed neutral
  // parameters are involved in the correction path.
  OfxParamHandle cgFix=nullptr,cgEnable=nullptr;OfxPropertySetHandle cgp=nullptr;
  if(gParam->paramGetHandle(ps,"colorgradr_fix",&cgFix,&cgp)==kOfxStatOK&&cgp){S(cgp,kOfxPropLabel,0,"Analyze Match");S(cgp,kOfxPropShortLabel,0,"Analyze Match");S(cgp,kOfxParamPropParent,0,"grpNeutral");}
  if(gParam->paramGetHandle(ps,"colorgradr_enable",&cgEnable,&cgp)==kOfxStatOK&&cgp){S(cgp,kOfxPropLabel,0,"Enable Match");S(cgp,kOfxPropShortLabel,0,"Enable Match");S(cgp,kOfxParamPropParent,0,"grpNeutral");}
  const char* cgHide[]={"colorgradr_activate","colorgradr_activate_offline_license","colorgradr_activate_offline_trial","colorgradr_activation_key","colorgradr_contrast_black_tgt","colorgradr_contrast_white_tgt","colorgradr_enable_bw","colorgradr_enable_hue","colorgradr_extend_trial","colorgradr_gamma_adjust","colorgradr_hue_fix_strength","colorgradr_instance_version","colorgradr_license_group","colorgradr_license_label","colorgradr_license_page","colorgradr_license_status","colorgradr_main_group","colorgradr_main_label","colorgradr_main_page","colorgradr_match","colorgradr_mid_tone_adjust","colorgradr_offline_activation_key","colorgradr_offline_license_group","colorgradr_offline_license_label","colorgradr_offline_license_page","colorgradr_postprocesslut","colorgradr_postprocesslut_data","colorgradr_postprocesslut_strength","colorgradr_saturation_adjust","colorgradr_save_1d_lut","colorgradr_save_cdl","colorgradr_save_offline_activate_file","colorgradr_save_offline_deactivate_file","colorgradr_save_offline_trial_file","colorgradr_status","colorgradr_transform_array","colorgradr_transform_merged","colorgradr_transform_nonhue_merged"};
  for(const char* id:cgHide){OfxParamHandle h=nullptr;OfxPropertySetHandle pp=nullptr;if(gParam->paramGetHandle(ps,id,&h,&pp)==kOfxStatOK&&pp)I(pp,kOfxParamPropSecret,0,1);}
  defineButton(ps,"resetNeutral","Disable Match","grpNeutral","Disable ColorGradr Fix without changing its stored transform.");
  defineChoice(ps,"matchOnly","ColorGradr Only","grpNeutral",0,{"Off","On"},"Diagnostic parity mode: On outputs the native ColorGradr render with no Keystone processing after it.");
  defineSlider(ps,"neutralAmount","Legacy Amount","grpNeutral",1.0,0.0,1.0,0.01,2,"Compatibility control; ColorGradr Fix itself remains unmodified at 1.0.");
  defineHiddenDouble(ps,"neutralGainR",1.0);defineHiddenDouble(ps,"neutralGainG",1.0);defineHiddenDouble(ps,"neutralGainB",1.0);defineHiddenDouble(ps,"matchSlopeR",1.0);defineHiddenDouble(ps,"matchSlopeG",1.0);defineHiddenDouble(ps,"matchSlopeB",1.0);defineHiddenDouble(ps,"matchOffsetR",0.0);defineHiddenDouble(ps,"matchOffsetG",0.0);defineHiddenDouble(ps,"matchOffsetB",0.0);defineHiddenDouble(ps,"neutralConfidence",0.0);defineHiddenBool(ps,"neutralValid",0);
  defineButton(ps,"sceneAnalyze","Analyze Scene","grpScene","Analyze the current frame, identify semantic regions, and solve a scene-aware Keystone grade.");
  defineChoice(ps,"sceneSubject","Subject","grpScene",0,{"Auto","Sky","Water","Skin","Foliage","Terrain","Ground","Built","Other"},"Auto uses the primary detected subject. Choose a named region to solve specifically for it.");
  defineSlider(ps,"sceneSeparation","Separation","grpScene",1.0,0.0,2.0,0.01,2,"Scales the scene color-separation move without re-running segmentation.");
  defineSlider(ps,"sceneBias","Bias","grpScene",0.0,-2.0,2.0,0.01,2,"Leans the fitted subject tone targets while preserving the scene decision.");
  defineButton(ps,"sceneReset","Reset Scene Grade","grpScene","Clear the stored scene grade and return the Scene Grade stage to exact identity.");
  defineString(ps,"sceneStatus","Status","grpScene","Not analyzed");
  defineHiddenDouble(ps,"sceneGainTemp",0.0);defineHiddenDouble(ps,"sceneOffsetTemp",0.0);defineHiddenDouble(ps,"sceneExposure",0.0);defineHiddenDouble(ps,"sceneShadows",0.0);defineHiddenDouble(ps,"sceneHighlights",0.0);defineHiddenDouble(ps,"sceneBase",0.0);defineHiddenDouble(ps,"sceneMode",-1.0);defineHiddenDouble(ps,"sceneResolved",-1.0);defineHiddenBool(ps,"sceneValid",0);
  defineChoice(ps,"ndFilter","Hoya ND","grpFilter",0,{"Off","ND2","ND4","ND8","ND16","ND32","ND64","ND100","ND200","ND400","ND500","ND1000"});
  defineChoice(ps,"uvFilter","UV Cut","grpFilter",0,{"Off","On"},"SpektraFilm camera-side UV cutoff behavior projected into Keystone's AWG4 RGB pipeline.");
  defineSlider(ps,"uvCutNm","UV Cut nm","grpFilter",410.0,350.0,500.0,1.0,0,"Cutoff center. SpektraFilm source default is 410 nm.");
  defineChoice(ps,"irFilter","IR Cut","grpFilter",0,{"Off","On"},"SpektraFilm camera-side IR cutoff behavior projected into Keystone's AWG4 RGB pipeline.");
  defineSlider(ps,"irCutNm","IR Cut nm","grpFilter",675.0,600.0,800.0,1.0,0,"Cutoff center. SpektraFilm source default is 675 nm.");
  defineSlider(ps,"wbTemp","Temp","grpWB",0,-100,100,0.1,1,"Warm/cool opponent white balance in linear AWG4.");defineSlider(ps,"wbTint","Tint","grpWB",0,-100,100,0.1,1,"Magenta/green opponent white balance in linear AWG4.");
  defineSlider(ps,"exposure","Exposure","grpTone",0,-6,6,0.01,2,nullptr);defineSlider(ps,"blackPoint","Black Pt","grpTone",0,-.05,.05,.0001,4,nullptr);defineSlider(ps,"contrast","Contrast","grpTone",1,.5,2,.001,3,nullptr);defineSlider(ps,"shadows","Shadows","grpTone",0,-1,1,.01,2,nullptr);defineSlider(ps,"highlights","Highlights","grpTone",0,-1,1,.01,2,nullptr);defineSlider(ps,"roll","Roll","grpTone",0,0,2,.01,2,nullptr);
  defineChoice(ps,"curvePreset","Curve","grpTone",0,{"Off","Speak Neutral","Speak Latitude","Speak Punch","Speak Chrome"});
  defineSlider(ps,"density","Density","grpColor",0,0,1,.001,3,nullptr);defineSlider(ps,"posSat","Pos Sat","grpColor",0,0,1,.001,3,nullptr);defineSlider(ps,"interlayer","Interlayer","grpColor",0,0,3,.01,2,nullptr);defineSlider(ps,"satSplit","Sat Split","grpColor",1,.5,2,.001,3,nullptr);defineSlider(ps,"hiBleach","Hi Bleach","grpFilm",0,0,1,.001,3,nullptr);defineSlider(ps,"loBleach","Lo Bleach","grpFilm",0,0,1,.001,3,nullptr);defineSlider(ps,"colorBleach","Bleach","grpFilm",0,0,80,.1,1,nullptr);defineSlider(ps,"fade","Fade","grpFilm",0,0,100,.1,1,nullptr);
  defineSlider(ps,"splitAmount","Amount","grpSplit",0,0,2,.001,3,nullptr);defineSlider(ps,"splitShadowHue","Shadow Hue","grpSplit",220,0,360,.1,1,nullptr);defineSlider(ps,"splitHighlightHue","Highlight Hue","grpSplit",40,0,360,.1,1,nullptr);defineSlider(ps,"splitBalance","Balance","grpSplit",.5,0,1,.001,3,nullptr);defineSlider(ps,"splitSubtractive","Subtractive","grpSplit",.5,0,1,.001,3,nullptr);
  defineChoice(ps,"lookColor","Color","grpLook",0,{"Off","Warm Cool","Cool Warm","Teal Orange","Sepia","Bleach Cool","Dusk Purple","Cyan Rose","Green Gold","Purple Gold","CHI","Island"});defineSlider(ps,"lookAmount","Amount","grpLook",1,0,1.5,.01,2,nullptr);defineChoice(ps,"creativeWhite","Creative White","grpLook",2,{"D93","D75","D65","D60","D55","D50"});
  defineChoice(ps,"skinEnable","Enable","grpSkin",0,{"Off","On"},"Enable the ToneLab Skin Tones block.");
  defineChoice(ps,"skinPreset","Preset","grpSkin",0,{"Custom","Vibrant","Muted","Cool / Neutralize Red","Warm / Add Warmth","Matte / Reduce Shine"},"ToneLab Skin Tones preset family.");
  defineSlider(ps,"skinSaturate","Saturate Skin","grpSkin",0.2,-1.0,1.0,0.01,2,"ToneLab skin saturation control.");
  defineSlider(ps,"skinColour","Skin Luminance","grpSkin",0.0,-1.0,1.0,0.01,2,"ToneLab skin luminance / density control.");
  defineSlider(ps,"skinPop","Skin Tone","grpSkin",0.0,-60.0,60.0,0.5,1,"ToneLab skin hue rotation in degrees.");
  defineSlider(ps,"skinCenter","Tone Center","grpSkin",0.0,-30.0,30.0,0.5,1,"Skin tone selection center offset.");
  defineSlider(ps,"skinRange","Tone Range","grpSkin",40.0,15.0,65.0,0.5,1,"Skin tone selection range.");
  defineSlider(ps,"skinBrightness","Brightness Center","grpSkin",0.5,0.0,1.0,0.01,2,"Luma selection center.");
  defineSlider(ps,"skinBrightnessRange","Brightness Range","grpSkin",1.0,0.25,1.75,0.01,2,"Luma selection range.");
  defineChoice(ps,"skinShowMask","Show Mask","grpSkin",0,{"Off","On"},"Preview the ToneLab-style skin selection mask.");
  defineSlider(ps,"skinIntensity","Intensity","grpSkin",100.0,0.0,200.0,1.0,0,"Skin tones intensity.");
  return kOfxStatOK;
}

static void getH(OfxParamSetHandle ps,const char* n,OfxParamHandle& h){OfxPropertySetHandle p=nullptr;if(gParam)gParam->paramGetHandle(ps,n,&h,&p);}
static std::mutex gDataMutex;
static std::unordered_map<OfxImageEffectHandle,std::unique_ptr<InstanceData>> gData;
static InstanceData* dataFor(OfxImageEffectHandle e){std::lock_guard<std::mutex> l(gDataMutex);auto it=gData.find(e);return it==gData.end()?nullptr:it->second.get();}
static OfxStatus createInstance(OfxImageEffectHandle e){
  auto d=std::make_unique<InstanceData>();OfxPropertySetHandle dummy=nullptr;
  gEffect->clipGetHandle(e,kOfxImageEffectSimpleSourceClipName,&d->source,&dummy);gEffect->clipGetHandle(e,kOfxImageEffectOutputClipName,&d->output,&dummy);OfxParamSetHandle ps=nullptr;gEffect->getParamSet(e,&ps);
#define H(f,n) getH(ps,n,d->f)
  H(analyze,"colorgradr_fix");H(resetNeutral,"resetNeutral");H(matchOnly,"matchOnly");H(neutralAmount,"neutralAmount");H(neutralGainR,"neutralGainR");H(neutralGainG,"neutralGainG");H(neutralGainB,"neutralGainB");H(matchSlopeR,"matchSlopeR");H(matchSlopeG,"matchSlopeG");H(matchSlopeB,"matchSlopeB");H(matchOffsetR,"matchOffsetR");H(matchOffsetG,"matchOffsetG");H(matchOffsetB,"matchOffsetB");H(neutralValid,"neutralValid");H(neutralConfidence,"neutralConfidence");
  H(sceneAnalyze,"sceneAnalyze");H(sceneReset,"sceneReset");H(sceneSubject,"sceneSubject");H(sceneSeparation,"sceneSeparation");H(sceneBias,"sceneBias");H(sceneStatus,"sceneStatus");H(sceneGainTemp,"sceneGainTemp");H(sceneOffsetTemp,"sceneOffsetTemp");H(sceneExposure,"sceneExposure");H(sceneShadows,"sceneShadows");H(sceneHighlights,"sceneHighlights");H(sceneBase,"sceneBase");H(sceneMode,"sceneMode");H(sceneResolved,"sceneResolved");H(sceneValid,"sceneValid");
  H(ndFilter,"ndFilter");H(uvFilter,"uvFilter");H(uvCutNm,"uvCutNm");H(irFilter,"irFilter");H(irCutNm,"irCutNm");H(wbTemp,"wbTemp");H(wbTint,"wbTint");H(exposure,"exposure");H(blackPoint,"blackPoint");H(contrast,"contrast");H(shadows,"shadows");H(highlights,"highlights");H(roll,"roll");H(curvePreset,"curvePreset");H(density,"density");H(posSat,"posSat");H(interlayer,"interlayer");H(satSplit,"satSplit");H(splitAmount,"splitAmount");H(splitShadowHue,"splitShadowHue");H(splitHighlightHue,"splitHighlightHue");H(splitBalance,"splitBalance");H(splitSubtractive,"splitSubtractive");H(hiBleach,"hiBleach");H(loBleach,"loBleach");H(colorBleach,"colorBleach");H(fade,"fade");H(lookColor,"lookColor");H(lookAmount,"lookAmount");H(creativeWhite,"creativeWhite");H(skinEnable,"skinEnable");H(skinPreset,"skinPreset");H(skinSaturate,"skinSaturate");H(skinColour,"skinColour");H(skinPop,"skinPop");H(skinCenter,"skinCenter");H(skinRange,"skinRange");H(skinBrightness,"skinBrightness");H(skinBrightnessRange,"skinBrightnessRange");H(skinShowMask,"skinShowMask");H(skinIntensity,"skinIntensity");
#undef H
  std::string why;loadCube33(bundledReferentPath(),d->cpuLut,&why);
  {std::lock_guard<std::mutex> l(gDataMutex);gData[e]=std::move(d);}return kOfxStatOK;
}
static OfxStatus destroyInstance(OfxImageEffectHandle e){std::lock_guard<std::mutex> l(gDataMutex);gData.erase(e);return kOfxStatOK;}
static double gd(OfxParamHandle h,double t,double def){double v=def;if(h&&gParam->paramGetValueAtTime(h,t,&v)==kOfxStatOK)return v;return def;}static int gi(OfxParamHandle h,double t,int def){int v=def;if(h&&gParam->paramGetValueAtTime(h,t,&v)==kOfxStatOK)return v;return def;}
static Params fetchParams(InstanceData*d,double t){Params p;p.neutralAmount=(float)gd(d->neutralAmount,t,1);p.neutralGainR=(float)gd(d->neutralGainR,t,1);p.neutralGainG=(float)gd(d->neutralGainG,t,1);p.neutralGainB=(float)gd(d->neutralGainB,t,1);p.matchSlopeR=(float)gd(d->matchSlopeR,t,1);p.matchSlopeG=(float)gd(d->matchSlopeG,t,1);p.matchSlopeB=(float)gd(d->matchSlopeB,t,1);p.matchOffsetR=(float)gd(d->matchOffsetR,t,0);p.matchOffsetG=(float)gd(d->matchOffsetG,t,0);p.matchOffsetB=(float)gd(d->matchOffsetB,t,0);p.sceneGainTemp=(float)gd(d->sceneGainTemp,t,0);p.sceneOffsetTemp=(float)gd(d->sceneOffsetTemp,t,0);p.sceneExposure=(float)gd(d->sceneExposure,t,0);p.sceneShadows=(float)gd(d->sceneShadows,t,0);p.sceneHighlights=(float)gd(d->sceneHighlights,t,0);p.ndFilter=gi(d->ndFilter,t,0);p.uvFilter=gi(d->uvFilter,t,0);p.uvCutNm=(float)gd(d->uvCutNm,t,410);p.irFilter=gi(d->irFilter,t,0);p.irCutNm=(float)gd(d->irCutNm,t,675);p.wbTemp=(float)gd(d->wbTemp,t,0);p.wbTint=(float)gd(d->wbTint,t,0);p.exposure=(float)gd(d->exposure,t,0);p.blackPoint=(float)gd(d->blackPoint,t,0);p.contrast=(float)gd(d->contrast,t,1);p.shadows=(float)gd(d->shadows,t,0);p.highlights=(float)gd(d->highlights,t,0);p.roll=(float)gd(d->roll,t,0);p.curvePreset=gi(d->curvePreset,t,0);p.density=(float)gd(d->density,t,0);p.posSat=(float)gd(d->posSat,t,0);p.interlayer=(float)gd(d->interlayer,t,0);p.satSplit=(float)gd(d->satSplit,t,1);p.splitAmount=(float)gd(d->splitAmount,t,0);p.splitShadowHue=(float)gd(d->splitShadowHue,t,220);p.splitHighlightHue=(float)gd(d->splitHighlightHue,t,40);p.splitBalance=(float)gd(d->splitBalance,t,.5);p.splitSubtractive=(float)gd(d->splitSubtractive,t,.5);p.hiBleach=(float)gd(d->hiBleach,t,0);p.loBleach=(float)gd(d->loBleach,t,0);p.colorBleach=(float)gd(d->colorBleach,t,0);p.fade=(float)gd(d->fade,t,0);p.lookColor=gi(d->lookColor,t,0);p.lookAmount=(float)gd(d->lookAmount,t,1);p.creativeWhite=gi(d->creativeWhite,t,2);p.skinEnable=gi(d->skinEnable,t,0);p.skinPreset=gi(d->skinPreset,t,0);p.skinSaturate=(float)gd(d->skinSaturate,t,.2);p.skinColour=(float)gd(d->skinColour,t,0);p.skinPop=(float)gd(d->skinPop,t,0);p.skinCenter=(float)gd(d->skinCenter,t,0);p.skinRange=(float)gd(d->skinRange,t,40);p.skinBrightness=(float)gd(d->skinBrightness,t,.5);p.skinBrightnessRange=(float)gd(d->skinBrightnessRange,t,1);p.skinShowMask=gi(d->skinShowMask,t,0);p.skinIntensity=(float)gd(d->skinIntensity,t,100);return p;}

static void sceneStatus(InstanceData*d,const std::string& s){if(d&&d->sceneStatus)gParam->paramSetValue(d->sceneStatus,s.c_str());}
static void clearSceneGrade(InstanceData*d){if(!d)return;gParam->paramSetValue(d->sceneGainTemp,0.0);gParam->paramSetValue(d->sceneOffsetTemp,0.0);gParam->paramSetValue(d->sceneExposure,0.0);gParam->paramSetValue(d->sceneShadows,0.0);gParam->paramSetValue(d->sceneHighlights,0.0);gParam->paramSetValue(d->sceneBase,0.0);gParam->paramSetValue(d->sceneMode,-1.0);gParam->paramSetValue(d->sceneResolved,-1.0);gParam->paramSetValue(d->sceneValid,0);sceneStatus(d,"Not analyzed");}
static void rescaleSceneColor(InstanceData*d,double time){if(!d)return;double base=gd(d->sceneBase,time,0.0),sep=gd(d->sceneSeparation,time,1.0),mode=gd(d->sceneMode,time,-1.0);double v=std::max(-1.0,std::min(1.0,base*std::max(0.0,std::min(2.0,sep))));gParam->paramSetValue(d->sceneGainTemp,mode==0.0?v:0.0);gParam->paramSetValue(d->sceneOffsetTemp,mode==1.0?v:0.0);}
static bool runSceneAnalysis(InstanceData*d,OfxPropertySetHandle inArgs){if(!d||!d->source)return false;double time=0.0;gProp->propGetDouble(inArgs,kOfxPropTime,0,&time);OfxPropertySetHandle si=nullptr;if(gEffect->clipGetImage(d->source,time,nullptr,&si)!=kOfxStatOK||!si){sceneStatus(d,"No source frame");return false;}void* sp=nullptr;int srb=0,b[4]={};gProp->propGetPointer(si,kOfxImagePropData,0,&sp);gProp->propGetInt(si,kOfxImagePropRowBytes,0,&srb);gProp->propGetIntN(si,kOfxImagePropBounds,4,b);int w=b[2]-b[0],h=b[3]-b[1],ss=srb/(int)sizeof(float);if(!sp||w<=0||h<=0||ss<w*4){gEffect->clipReleaseImage(si);sceneStatus(d,"Unsupported source image");return false;}if(d->cpuLut.size()!=35937){std::string why;if(!loadCube33(bundledReferentPath(),d->cpuLut,&why)){gEffect->clipReleaseImage(si);sceneStatus(d,"Output LUT unavailable");return false;}}Params p=fetchParams(d,time);p.sceneGainTemp=p.sceneOffsetTemp=p.sceneExposure=p.sceneShadows=p.sceneHighlights=0.0f;int subj=gi(d->sceneSubject,time,0)-1;float sep=(float)gd(d->sceneSeparation,time,1.0),bias=(float)gd(d->sceneBias,time,0.0);SceneGradeResult r;bool ok=analyzeSceneGrade((const float*)sp,w,h,ss,p,d->cpuLut,subj,sep,bias,r);gEffect->clipReleaseImage(si);if(!ok||!r.ok){clearSceneGrade(d);sceneStatus(d,r.note.empty()?"No scene grade found":r.note);return false;}gParam->paramSetValue(d->sceneGainTemp,(double)r.sceneGainTemp);gParam->paramSetValue(d->sceneOffsetTemp,(double)r.sceneOffsetTemp);gParam->paramSetValue(d->sceneExposure,(double)r.sceneExposure);gParam->paramSetValue(d->sceneShadows,(double)r.sceneShadows);gParam->paramSetValue(d->sceneHighlights,(double)r.sceneHighlights);gParam->paramSetValue(d->sceneBase,(double)r.colorBase);gParam->paramSetValue(d->sceneMode,(double)r.colorMode);gParam->paramSetValue(d->sceneResolved,(double)r.subject);gParam->paramSetValue(d->sceneValid,1);sceneStatus(d,r.note);return true;}

static OfxStatus instanceChanged(OfxImageEffectHandle e,OfxPropertySetHandle inArgs){auto*d=dataFor(e);if(!d)return kOfxStatReplyDefault;char* reason=nullptr;char* type=nullptr;char* name=nullptr;if(gProp->propGetString(inArgs,kOfxPropChangeReason,0,&reason)!=kOfxStatOK||!reason||std::strcmp(reason,kOfxChangeUserEdited)!=0)return kOfxStatReplyDefault;if(gProp->propGetString(inArgs,kOfxPropType,0,&type)!=kOfxStatOK||!type||std::strcmp(type,kOfxTypeParameter)!=0)return kOfxStatReplyDefault;if(gProp->propGetString(inArgs,kOfxPropName,0,&name)!=kOfxStatOK||!name)return kOfxStatReplyDefault;
  if(!std::strncmp(name,"colorgradr_",11))return colorgradr_bridge::delegate(kOfxActionInstanceChanged,e,inArgs,nullptr);
  if(!std::strcmp(name,"resetNeutral")){OfxParamSetHandle ps=nullptr;gEffect->getParamSet(e,&ps);OfxParamHandle h=nullptr;OfxPropertySetHandle pp=nullptr;if(ps&&gParam->paramGetHandle(ps,"colorgradr_enable",&h,&pp)==kOfxStatOK&&h)gParam->paramSetValue(h,0);return kOfxStatOK;}
  if(!std::strcmp(name,"sceneReset")){clearSceneGrade(d);return kOfxStatOK;}
  if(!std::strcmp(name,"sceneAnalyze")||!std::strcmp(name,"sceneSubject")||!std::strcmp(name,"sceneBias")){runSceneAnalysis(d,inArgs);return kOfxStatOK;}
  if(!std::strcmp(name,"sceneSeparation")){double t=0;gProp->propGetDouble(inArgs,kOfxPropTime,0,&t);rescaleSceneColor(d,t);return kOfxStatOK;}
  if(!std::strcmp(name,"skinPreset")){
    int sp=0; if(d->skinPreset)gParam->paramGetValue(d->skinPreset,&sp);
    double sat=.2,lum=0,hue=0,center=0,range=40,bc=.5,br=1.0,intensity=100;
    if(sp==1){sat=.55;lum=.04;range=42;intensity=110;}
    else if(sp==2){sat=-.25;lum=-.02;range=44;intensity=100;}
    else if(sp==3){sat=.12;hue=10;center=2;range=40;intensity=100;}
    else if(sp==4){sat=.28;hue=-8;center=-2;range=42;intensity=100;}
    else if(sp==5){sat=.05;lum=-.16;range=38;bc=.60;br=.70;intensity=100;}
    if(sp>0){gParam->paramSetValue(d->skinEnable,1);gParam->paramSetValue(d->skinSaturate,sat);gParam->paramSetValue(d->skinColour,lum);gParam->paramSetValue(d->skinPop,hue);gParam->paramSetValue(d->skinCenter,center);gParam->paramSetValue(d->skinRange,range);gParam->paramSetValue(d->skinBrightness,bc);gParam->paramSetValue(d->skinBrightnessRange,br);gParam->paramSetValue(d->skinIntensity,intensity);}return kOfxStatOK;
  }
  return kOfxStatReplyDefault;
}

static OfxStatus render(OfxImageEffectHandle e,OfxPropertySetHandle inArgs){auto*d=dataFor(e);if(!d||!d->output)return kOfxStatFailed;
  // First render with the original ColorGradr OFX. This is the actual plugin's
  // Fix engine and transform evaluation, not a Keystone reconstruction.
  OfxStatus cg=colorgradr_bridge::delegate(kOfxImageEffectActionRender,e,inArgs,nullptr);if(cg!=kOfxStatOK&&cg!=kOfxStatReplyDefault)return cg;
  double time=0;gProp->propGetDouble(inArgs,kOfxPropTime,0,&time);OfxPropertySetHandle di=nullptr;if(gEffect->clipGetImage(d->output,time,nullptr,&di)!=kOfxStatOK||!di)return kOfxStatFailed;
  void* dp=nullptr;int drb=0,b[4]={};gProp->propGetPointer(di,kOfxImagePropData,0,&dp);gProp->propGetInt(di,kOfxImagePropRowBytes,0,&drb);gProp->propGetIntN(di,kOfxImagePropBounds,4,b);int w=b[2]-b[0],h=b[3]-b[1],ds=drb/(int)sizeof(float);if(!dp||w<=0||h<=0){gEffect->clipReleaseImage(di);return kOfxStatFailed;}
  if(gi(d->matchOnly,time,0)!=0){gEffect->clipReleaseImage(di);return kOfxStatOK;}
  Params p=fetchParams(d,time);p.neutralAmount=0.0f;p.neutralGainR=p.neutralGainG=p.neutralGainB=1.0f;p.matchSlopeR=p.matchSlopeG=p.matchSlopeB=1.0f;p.matchOffsetR=p.matchOffsetG=p.matchOffsetB=0.0f;
  if(d->cpuLut.size()!=35937){std::string why;if(!loadCube33(bundledReferentPath(),d->cpuLut,&why)){gEffect->clipReleaseImage(di);return kOfxStatFailed;}}
  // Post-process ColorGradr's output in place through Keystone. With Keystone
  // controls bypassed, the buffer entering this stage is exactly ColorGradr's output.
  processRGBA((const float*)dp,(float*)dp,w,h,ds,ds,p,d->cpuLut);gEffect->clipReleaseImage(di);return kOfxStatOK;
}

static OfxStatus rod(OfxImageEffectHandle e,OfxPropertySetHandle inArgs,OfxPropertySetHandle outArgs){auto*d=dataFor(e);if(!d||!d->source)return kOfxStatReplyDefault;double t=0;gProp->propGetDouble(inArgs,kOfxPropTime,0,&t);OfxRectD r{};if(gEffect->clipGetRegionOfDefinition(d->source,t,&r)==kOfxStatOK){double v[4]={r.x1,r.y1,r.x2,r.y2};gProp->propSetDoubleN(outArgs,kOfxImageEffectPropRegionOfDefinition,4,v);return kOfxStatOK;}return kOfxStatReplyDefault;}
static OfxStatus mainEntry(const char*a,const void*h,OfxPropertySetHandle in,OfxPropertySetHandle out){auto e=(OfxImageEffectHandle)h;
  if(!std::strcmp(a,kOfxActionLoad)||!std::strcmp(a,kOfxActionUnload))return colorgradr_bridge::delegate(a,h,in,out);
  if(!std::strcmp(a,kOfxActionDescribe)){OfxStatus cs=colorgradr_bridge::delegate(a,h,in,out);if(cs!=kOfxStatOK&&cs!=kOfxStatReplyDefault)return cs;return describe(e);}
  if(!std::strcmp(a,kOfxImageEffectActionDescribeInContext)){OfxStatus cs=colorgradr_bridge::delegate(a,h,in,out);if(cs!=kOfxStatOK&&cs!=kOfxStatReplyDefault)return cs;return describeInContext(e);}
  if(!std::strcmp(a,kOfxActionCreateInstance)){OfxStatus cs=colorgradr_bridge::delegate(a,h,in,out);if(cs!=kOfxStatOK&&cs!=kOfxStatReplyDefault)return cs;return createInstance(e);}
  if(!std::strcmp(a,kOfxActionDestroyInstance)){OfxStatus ks=destroyInstance(e);OfxStatus cs=colorgradr_bridge::delegate(a,h,in,out);return cs==kOfxStatOK?ks:cs;}
  if(!std::strcmp(a,kOfxActionInstanceChanged))return instanceChanged(e,in);
  if(!std::strcmp(a,kOfxImageEffectActionRender))return render(e,in);
  if(!std::strcmp(a,kOfxImageEffectActionGetRegionOfDefinition)){OfxStatus cs=colorgradr_bridge::delegate(a,h,in,out);if(cs==kOfxStatOK)return cs;return rod(e,in,out);}
  return colorgradr_bridge::delegate(a,h,in,out);
}
static OfxPlugin gPlugin={kOfxImageEffectPluginApi,1,"com.luma.keystoneofx",1,6,setHostFunc,mainEntry};
extern "C" { OfxExport int OfxGetNumberOfPlugins(){return 1;} OfxExport OfxPlugin* OfxGetPlugin(int n){return n==0?&gPlugin:nullptr;} OfxExport void OfxSetHost(void*h){setHostFunc(h);} }
