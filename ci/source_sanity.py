#!/usr/bin/env python3
from pathlib import Path
import hashlib,re,sys
root=Path(__file__).resolve().parents[1]
def fail(msg): print('ERROR:',msg,file=sys.stderr);sys.exit(1)
lut=root/'resources/Keystone_Output_LogC4_to_Rec709.cube'
sha=hashlib.sha256(lut.read_bytes()).hexdigest()
if sha!='9bd910e505f4f8fdfef67f97b85b7127f80528c4fda99e87ebb25eaa985c6d54': fail('Output LUT SHA256 changed: '+sha)
text=(root/'src/KeystoneOFX.cpp').read_text()
header=(root/'src/OpenFXMinimal.h').read_text()
if 'static void setHostFunc(OfxHost* h){gHost=h;}' not in text: fail('setHost must only store host pointer')
sethost=text[text.index('static void setHostFunc'):text.index('static OfxStatus onLoad')]
if 'fetchSuite' in sethost: fail('setHost illegally calls an OFX suite')
load=text[text.index('static OfxStatus onLoad'):text.index('static OfxStatus onUnload')]
for suite in ['kOfxPropertySuite','kOfxImageEffectSuite','kOfxParameterSuite']:
    if suite not in load: fail('onLoad does not fetch '+suite)
if 'kOfxActionLoad' not in text or 'return onLoad()' not in text: fail('Load action not trapped')
if 'kOfxActionUnload' not in text or 'return onUnload()' not in text: fail('Unload action not trapped')
if 'NativeMatchBridge' in text or 'native_match_bridge' in text: fail('nested OFX bridge still present')
if 'kOfxImageEffectPropMetalRenderSupported' in text: fail('load-safe build must not advertise Metal')
if 'runMetal(' in text: fail('load-safe build must not call Metal')
if 'kOfxImageEffectContextGeneral' in text: fail('load-safe build must be Filter context only')
if 'OfxSetHost' in text: fail('non-mandated global OfxSetHost export still present')
for token in ['analyzeNeutral','resetNeutral','neutralGainR','kOfxActionInstanceChanged']:
    if token not in text: fail('missing '+token)

# Keep external product/plugin branding out of Keystone docs and source.
blocked=['colorgradr','tonelab','spektrafilm','genesis','contour','primera','referent','opendrt','hoya','speak','me_desatch']
for q in root.rglob('*'):
    if not q.is_file() or q.name in {'THIRD_PARTY_NOTICES.md','LICENSE','source_sanity.py'}:
        continue
    if q.suffix.lower() not in {'.cpp','.h','.md','.py','.sh','.yml','.yaml','.txt','.inc','.dctl','.cube','.in'} and q.name!='CMakeLists.txt':
        continue
    try: body=q.read_text(errors='ignore').lower()
    except Exception: continue
    for term in blocked:
        if term in body: fail(f'blocked product name {term} in {q.relative_to(root)}')
print('source_sanity: OK')
