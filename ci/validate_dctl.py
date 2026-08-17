#!/usr/bin/env python3
from pathlib import Path
import hashlib,re,sys
root=Path(__file__).resolve().parents[1]
files=list((root/'dctl').glob('*.dctl'))
if len(files)!=1:
    print(f'[FAIL] expected exactly one DCTL, found {len(files)}');sys.exit(1)
p=files[0];t=p.read_text();sha=hashlib.sha256(p.read_bytes()).hexdigest()
readme=(root/'README.md').read_text();validation=(root/'VALIDATION.md').read_text();workflow=(root/'.github/workflows/validate-release.yml').read_text()
checks={
 'non-empty source':len(t)>500,
 'RC28 filename':p.name=='Keystone-v1.0-RC28.dctl',
 'RC28 header':'Keystone v1.0 RC28 production candidate.' in t,
 'transform entry point':'__DEVICE__ float3 transform(' in t,
 'balanced parentheses':t.count('(')==t.count(')'),
 'balanced braces':t.count('{')==t.count('}'),
 'balanced brackets':t.count('[')==t.count(']'),
 'no merge markers':re.search(r'^(<<<<<<<|=======|>>>>>>>)',t,re.M) is None,
 'GPL SPDX':'SPDX-License-Identifier: GPL-3.0-or-later' in t,
 'manual negative response controls removed':all(x not in t for x in ['Negative / Mid','Negative / Below','Negative / Above','fns_mid_out','fns_contrast_below','fns_contrast_above']),
 'manual Pivot removed':'Tone / Pivot' not in t and 'p_pivot_offset' not in t,
 'Balance RGB removed':all(x not in t for x in ['Balance / Red','Balance / Green','Balance / Blue','balance_red','balance_green','balance_blue']),
 'fixed internal negative response':all(x in t for x in ['KEYSTONE_FNS_MID_IN','KEYSTONE_FNS_MID_OUT','KEYSTONE_FNS_BELOW','KEYSTONE_FNS_ABOVE','KEYSTONE_FNS_BLEND']),
 'live printer lights':all(x in t for x in ['Negative / Printer R','Negative / Printer G','Negative / Printer B','fns_apply_printer_lights']),
 'true scene Exposure':'Exposure is a true scene-linear stop move' in t,
 'auto colorspace pivot':'auto_scene_midgray' in t and 'auto_mid=auto_scene_midgray(ti_tf,c_logc3ei)' in t,
 'auto tone zones':all(x in t for x in ['shadow_zone_mask','highlight_zone_mask','apply_common_stop_gain']),
 'monotonic luma rolloff':'apply_auto_rolloff_xyz' in t,
 'gamut-aware Chroma/Hue':all(x in t for x in ['apply_safe_chroma_hue','enforce_native_gamut_oklab']),
 'Hue Uniformity label':'Skin / Hue Uniformity' in t and 'Skin / Evenness' not in t,
 'all ME Desatch controls':all(x in t for x in ['Sat / Global','Sat / Red','Sat / Green','Sat / Blue','Sat / Cyan','Sat / Magenta','Sat / Yellow']),
 'ME working transfer':'Exact ME_Desatch working-transfer stage' in t and 'ds_code=apply_me_desatch_exact' in t,
 'White Clean present':'Output / White Clean' in t and 'white_gamut_clean' in t,
 'Black Clean present':'Output / Black Clean' in t and 'black_gamut_clean' in t,
 'manual Output Negatives removed':'Output / Negatives' not in t and 'native_negative_compress' not in t,
 'manual Skin Protect removed':'Output / Skin Protect' not in t and 'output_skin_protect' not in t,
 'automatic skin cleanup protection':'output_skin_keep=1.0f-clampf(output_skin_mask' in t,
 'pre-encode safety':'technical_encoded_negative_guard' in t,
 'post-encode safety':'technical_final_encoded_guard' in t,
 'scene ceiling':'technical_scene_ceiling_guard' in t,
 'Color Volume remains removed':'Volume /' not in t and 'cv_gamut_compression' not in t,
 'creative whitepoint remains moved':'creative_whitepoint' not in t,
 'README hash current':sha in readme and p.name in readme,
 'VALIDATION hash current':sha in validation and p.name in validation,
 'release includes scripts':'cp -R dctl scripts' in workflow,
 'release includes third-party':'THIRD_PARTY.md' in workflow,
 'behavioral CI wired':'behavioral_validate.py' in workflow,
}
ui_ids=set(m.group(1) for m in re.finditer(r'DEFINE_UI_PARAMS\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,',t))
enum_ids=set()
for m in re.finditer(r'DEFINE_UI_PARAMS\(\s*[A-Za-z_][A-Za-z0-9_]*\s*,.*?DCTLUI_COMBO_BOX\s*,.*?\{([^{}]+)\}\s*,\s*\{[^{}]+\}\s*\)',t,re.S):
    for x in m.group(1).split(','):
        x=x.strip()
        if re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*',x):enum_ids.add(x)
fn_ids=set(m.group(1) for m in re.finditer(r'__DEVICE__\s+(?:float|float2|float3|int|bool)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(',t))
collisions=sorted((ui_ids|enum_ids)&fn_ids);checks['no Metal UI/function symbol collisions']=not collisions
for k,v in checks.items():print(f"[{'PASS' if v else 'FAIL'}] {k}")
if collisions:print('Collisions:',', '.join(collisions))
if not all(checks.values()):sys.exit(1)
print('SHA-256:',sha)
