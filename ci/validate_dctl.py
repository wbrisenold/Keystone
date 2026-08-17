#!/usr/bin/env python3
from pathlib import Path
import hashlib, re, sys

root=Path(__file__).resolve().parents[1]
files=list((root/'dctl').glob('*.dctl'))
if len(files)!=1:
    print(f'[FAIL] expected exactly one DCTL, found {len(files)}')
    sys.exit(1)
p=files[0]
t=p.read_text(encoding='utf-8')
sha=hashlib.sha256(p.read_bytes()).hexdigest()
readme=(root/'README.md').read_text(encoding='utf-8')
validation=(root/'VALIDATION.md').read_text(encoding='utf-8')
workflow=(root/'.github/workflows/validate-release.yml').read_text(encoding='utf-8')
conflict=re.search(r'^(<<<<<<<|=======|>>>>>>>)',t,re.M) is not None

checks={
    'non-empty source':len(t)>500,
    'RC27 filename':p.name=='Keystone-v1.0-RC27.dctl',
    'RC27 header':'Keystone v1.0 RC27 production candidate.' in t,
    'transform entry point':'__DEVICE__ float3 transform(' in t,
    'UI parameters present':'DEFINE_UI_PARAMS(' in t,
    'balanced parentheses':t.count('(')==t.count(')'),
    'balanced braces':t.count('{')==t.count('}'),
    'balanced brackets':t.count('[')==t.count(']'),
    'no merge-conflict markers':not conflict,
    'GPL SPDX present':'SPDX-License-Identifier: GPL-3.0-or-later' in t,
    'permanent Negative Space UI':'Negative Space / Mode' not in t and 'Negative / Mid' in t and 'Negative / Below' in t and 'Negative / Above' in t,
    'hidden FNS plumbing removed':'DEFINE_UI_PARAMS(fns_mid_in' not in t and 'DEFINE_UI_PARAMS(fns_blend_width' not in t,
    'printer grade present':'fns_apply_printer_lights' in t and 'neg=fns_apply_printer_lights' in t,
    'printer not inverted':'/ _fmaxf(gains.' not in t and 'rgb.x/_fmaxf(gains.x' not in t,
    'negative-native tone present':'apply_negative_space_tone' in t and 'apply_primera_encoded_tone' not in t,
    'Mid Chroma removed':'Color / Mid Chroma' not in t and 'midtone_chroma' not in t,
    'core color controls present':all(x in t for x in ['Color / Chroma','Color / Vibrance','Color / Hue']),
    'all ME Desatch controls present':all(x in t for x in ['Sat / Global','Sat / Red','Sat / Green','Sat / Blue','Sat / Cyan','Sat / Magenta','Sat / Yellow']),
    'ME working-transfer stage present':'Exact ME_Desatch working-transfer stage' in t and 'ds_code=apply_me_desatch_exact' in t and 'graded_xyz=native_linear_to_xyz(ds_back,g)' in t,
    'ME removed from final encoded tail':'if(desatch_active)result=apply_me_desatch_exact' not in t,
    'monotonic highlight fix present':'keystone_highlight_gain_monotonic' in t and 'primera_highlight_gain(' not in t,
    'pre-encode safety present':'technical_encoded_negative_guard' in t,
    'post-encode safety present':'technical_final_encoded_guard' in t,
    'scene ceiling present':'technical_scene_ceiling_guard' in t,
    'Color Volume remains removed':'cv_gamut_compression' not in t and 'Volume /' not in t,
    'creative white point remains moved':'creative_whitepoint' not in t,
    'README hash current':sha in readme and p.name in readme,
    'VALIDATION hash current':sha in validation and p.name in validation,
    'release includes scripts':'cp -R dctl scripts' in workflow,
    'release includes third-party notice':'THIRD_PARTY.md' in workflow,
    'behavioral CI wired':'behavioral_validate.py' in workflow,
}

ui_ids=set(m.group(1) for m in re.finditer(r'DEFINE_UI_PARAMS\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,',t))
enum_ids=set()
for m in re.finditer(r'DEFINE_UI_PARAMS\(\s*[A-Za-z_][A-Za-z0-9_]*\s*,.*?DCTLUI_COMBO_BOX\s*,.*?\{([^{}]+)\}\s*,\s*\{[^{}]+\}\s*\)',t,re.S):
    for x in m.group(1).split(','):
        x=x.strip()
        if re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*',x): enum_ids.add(x)
fn_ids=set(m.group(1) for m in re.finditer(r'__DEVICE__\s+(?:float|float2|float3|int|bool)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(',t))
collisions=sorted((ui_ids|enum_ids)&fn_ids)
checks['no Metal UI/function symbol collisions']=not collisions

for name,ok in checks.items(): print(f"[{'PASS' if ok else 'FAIL'}] {name}")
if collisions: print('Collisions:',', '.join(collisions))
if not all(checks.values()): sys.exit(1)
print('SHA-256:',sha)
