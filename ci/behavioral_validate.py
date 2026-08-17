#!/usr/bin/env python3
from pathlib import Path
import re, shutil, subprocess, tempfile, sys

root=Path(__file__).resolve().parents[1]
dctls=list((root/'dctl').glob('*.dctl'))
if len(dctls)!=1:
    raise SystemExit(f'[FAIL] expected exactly one DCTL, found {len(dctls)}')
src=dctls[0].read_text(encoding='utf-8')
needle=r'__DEVICE__ float finite_or_zero\(float x\)\{return\(x==x&&_fabs\(x\)<3\.402823e38f\)\?x:0\.0f;\}'
raw,count=re.subn(needle,'__DEVICE__ float finite_or_zero(float x){return x;}',src,count=1)
if count!=1:
    raise SystemExit('[FAIL] could not disable final finite guard for upstream stress test')
compiler=shutil.which('g++') or shutil.which('clang++')
if not compiler:
    raise SystemExit('[FAIL] C++ compiler not found')
with tempfile.TemporaryDirectory(prefix='keystone-ci-') as td:
    td=Path(td)
    (td/'Keystone_under_test.dctl').write_text(raw,encoding='utf-8')
    exe=td/'behavioral_harness'
    cmd=[compiler,'-std=c++17','-O2',str(root/'ci/behavioral_harness.cpp'),'-I',str(td),'-o',str(exe)]
    print('[RUN]',' '.join(cmd))
    subprocess.run(cmd,check=True)
    subprocess.run([str(exe)],check=True)
print('[PASS] behavioral validation')
