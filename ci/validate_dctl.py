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

conflict = re.search(r'^(<<<<<<<|=======|>>>>>>>)', t, re.M) is not None

checks={
    'non-empty source':len(t)>500,
    'transform entry point':'__DEVICE__ float3 transform(' in t,
    'UI parameters present':'DEFINE_UI_PARAMS(' in t,
    'balanced parentheses':t.count('(')==t.count(')'),
    'balanced braces':t.count('{')==t.count('}'),
    'balanced brackets':t.count('[')==t.count(']'),
    'no merge-conflict markers':not conflict,
    'SPDX present':'SPDX-License-Identifier:' in t,
}

ui_ids=set(m.group(1) for m in re.finditer(r'DEFINE_UI_PARAMS\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*,',t))
enum_ids=set()
for m in re.finditer(r'DEFINE_UI_PARAMS\(\s*[A-Za-z_][A-Za-z0-9_]*\s*,.*?DCTLUI_COMBO_BOX\s*,.*?\{([^{}]+)\}\s*,\s*\{[^{}]+\}\s*\)',t,re.S):
    for x in m.group(1).split(','):
        x=x.strip()
        if re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*',x):
            enum_ids.add(x)
fn_ids=set(m.group(1) for m in re.finditer(r'__DEVICE__\s+(?:float|float2|float3|int|bool)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(',t))
collisions=sorted((ui_ids|enum_ids)&fn_ids)
checks['no Metal UI/function symbol collisions']=not collisions

for name,ok in checks.items():
    print(f"[{'PASS' if ok else 'FAIL'}] {name}")
if collisions:
    print('Collisions:',', '.join(collisions))
if not all(checks.values()):
    sys.exit(1)
print('SHA-256:',hashlib.sha256(p.read_bytes()).hexdigest())
