#!/usr/bin/env python3
from pathlib import Path
import hashlib,re,sys
root=Path(__file__).resolve().parents[1]
def fail(msg): print('ERROR:',msg,file=sys.stderr);sys.exit(1)
# Original Keystone Output data must remain byte-identical.
lut=root/'resources/Keystone_Output_LogC4_to_Rec709.cube'
sha=hashlib.sha256(lut.read_bytes()).hexdigest()
if sha!='9bd910e505f4f8fdfef67f97b85b7127f80528c4fda99e87ebb25eaa985c6d54': fail('Keystone Output LUT SHA256 changed: '+sha)

engine=root/'vendor/NativeMatch/NativeMatch.ofx.bundle/Contents/MacOS/NativeMatch.ofx'
if not engine.is_file(): fail('bundled NativeMatch engine missing')
if engine.stat().st_size != 3759488: fail('unexpected NativeMatch engine size')

text=(root/'src/KeystoneOFX.cpp').read_text()
metal=(root/'shaders/KeystoneShared.metalh').read_text()
for token in ['nativeParamId("fix")','resetNeutral','neutralGainR','kOfxActionInstanceChanged','kOfxParamTypeGroup','kOfxParamPropGroupOpen','native_match_bridge::delegate']:
    if token not in text and token not in (root/'src/OpenFXMinimal.h').read_text(): fail('missing '+token)
# Every exposed continuous Keystone control must be defined by the slider helper.
expected=['neutralAmount','wbTemp','wbTint','exposure','blackPoint','contrast','shadows','highlights','roll','density','posSat','interlayer','satSplit','hiBleach','loBleach','colorBleach','fade','splitAmount','splitShadowHue','splitHighlightHue','splitBalance','splitSubtractive','lookAmount','uvCutNm','irCutNm','skinSaturate','skinColour','skinPop','skinCenter','skinRange','skinBrightness','skinBrightnessRange','skinIntensity']
for p in expected:
    if f'defineSlider(ps,"{p}"' not in text: fail(f'{p} is not exposed as a slider')
# Analyze once must store hidden persistent gains, not evaluate every render.
render=text[text.index('static OfxStatus render'):]
if 'analyzeNeutralRGBA' in render: fail('render path re-analyzes frames; one-frame lock broken')
if 'native_match_bridge::delegate(kOfxImageEffectActionRender' not in render: fail('NativeMatch native render is not delegated before Keystone processing')
# No hybrid LUT identifiers.
for p in root.rglob('*'):
    if p == Path(__file__).resolve(): continue
    if p.is_file() and p.suffix in {'.cpp','.h','.mm','.metal','.metalh','.py','.sh','.md'}:
        try:s=p.read_text()
        except:continue
        if 'KS_SKIN_output_transform_HYBRID' in s: fail('hybrid LUT reference found in '+str(p.relative_to(root)))
print('source_sanity: OK')
