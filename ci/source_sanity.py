#!/usr/bin/env python3
from pathlib import Path
import hashlib,re,sys
root=Path(__file__).resolve().parents[1]
def fail(msg): print('ERROR:',msg,file=sys.stderr);sys.exit(1)
# Original Referent data must remain byte-identical.
lut=root/'resources/Referent_LogC4_to_Rec709.cube'
sha=hashlib.sha256(lut.read_bytes()).hexdigest()
if sha!='19b2feb5ed8cb767d980e9f9b351b6e1823a3990974277fdb4a46d1f709d251c': fail('Referent LUT SHA256 changed: '+sha)

engine=root/'vendor/ColorGradr/colorgradr.ofx.bundle/Contents/MacOS/colorgradr.ofx'
if not engine.is_file(): fail('bundled ColorGradr engine missing')
if engine.stat().st_size != 3759488: fail('unexpected ColorGradr engine size')


model_bin=root/'resources/SceneModel/ade20k.bin'
model_param=root/'resources/SceneModel/ade20k.param'
if hashlib.sha256(model_bin.read_bytes()).hexdigest()!='908a6785debbca3502ce11a08462a00504f4718e86c326042fd6b076acf84790': fail('scene model bin changed')
if hashlib.sha256(model_param.read_bytes()).hexdigest()!='ba8e532e6357899f7f1fd8deeaf75322ce3f063487602ae32338a415bde569da': fail('scene model param changed')
for target in [root/'README.md',root/'src/SceneGrade.cpp',root/'src/SceneGrade.h',root/'src/KeystoneOFX.cpp']:
    if ('magic'+' grade') in target.read_text().lower(): fail('retired upstream feature name found in '+str(target.relative_to(root)))

# Resolve load-safety: the main OFX must not compile or link the semantic inference runtime.
scene=(root/'src/SceneGrade.cpp').read_text()
cmake=(root/'CMakeLists.txt').read_text()
if '#include <net.h>' in scene or 'ncnn::' in scene: fail('ncnn leaked into main SceneGrade translation unit')
if 'target_link_libraries(KeystoneOFX PRIVATE ncnn)' in cmake: fail('main OFX has a hard ncnn dependency')
if 'KeystoneSceneEngine.dylib' not in scene or 'dlopen' not in scene: fail('scene sidecar is not lazy-loaded')
if 'KeystoneSceneSegment' not in (root/'src/SceneEngine.cpp').read_text(): fail('scene sidecar C ABI missing')
for target in [root/'src/SceneEngine.cpp',root/'CMakeLists.txt']:
    if ('magic'+' grade') in target.read_text().lower(): fail('retired upstream feature name found in '+str(target.relative_to(root)))

text=(root/'src/KeystoneOFX.cpp').read_text()
metal=(root/'shaders/KeystoneShared.metalh').read_text()
for token in ['colorgradr_fix','resetNeutral','neutralGainR','kOfxActionInstanceChanged','kOfxParamTypeGroup','kOfxParamPropGroupOpen','colorgradr_bridge::delegate']:
    if token not in text and token not in (root/'src/OpenFXMinimal.h').read_text(): fail('missing '+token)
# Every exposed continuous Keystone control must be defined by the slider helper.
expected=['sceneSeparation','sceneBias','neutralAmount','wbTemp','wbTint','exposure','blackPoint','contrast','shadows','highlights','roll','density','posSat','interlayer','satSplit','hiBleach','loBleach','colorBleach','fade','splitAmount','splitShadowHue','splitHighlightHue','splitBalance','splitSubtractive','lookAmount','uvCutNm','irCutNm','skinSaturate','skinColour','skinPop','skinCenter','skinRange','skinBrightness','skinBrightnessRange','skinIntensity']
for p in expected:
    if f'defineSlider(ps,"{p}"' not in text: fail(f'{p} is not exposed as a slider')
# Analyze once must store hidden persistent gains, not evaluate every render.
render=text[text.index('static OfxStatus render'):]
if 'analyzeNeutralRGBA' in render: fail('render path re-analyzes frames; one-frame lock broken')
if 'colorgradr_bridge::delegate(kOfxImageEffectActionRender' not in render: fail('ColorGradr native render is not delegated before Keystone processing')
# No hybrid LUT identifiers.
for p in root.rglob('*'):
    if p == Path(__file__).resolve(): continue
    if p.is_file() and p.suffix in {'.cpp','.h','.mm','.metal','.metalh','.py','.sh','.md'}:
        try:s=p.read_text()
        except:continue
        if 'TL_REFERENT_HYBRID' in s: fail('hybrid LUT reference found in '+str(p.relative_to(root)))
print('source_sanity: OK')
