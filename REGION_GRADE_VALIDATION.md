# Region Grade validation

## Files changed from verified pre-port plugin code

Runtime/build changes are limited to:

- `.github/workflows/build.yml` — runs the new test in CI.
- `CMakeLists.txt` — adds the Region Grade test target only.
- `src/KeystoneParams.h` — Region Grade parameters.
- `src/KeystoneCoreCPU.h` — local grade math and region-mask mix point.
- `src/KeystoneCPU.cpp` — region-mask analysis and per-pixel mask evaluation.
- `src/KeystoneOFX.cpp` — Region Grade UI/parameter handles.
- `tests/region_grade_tests.cpp` — synthetic mask and local exposure tests.
- documentation.

No changes were made to:

- `src/ColorGradrBridge.cpp/.h`
- `src/MetalBridge.mm/.h`
- `shaders/KeystoneKernels.metal`
- `shaders/KeystoneShared.metalh`
- `src/Info.plist.in`
- `scripts/build_macos.sh`
- embedded ColorGradr bundle/resources
- generated Keystone grading math/constants
- Referent LUT

## Validation performed

- Existing `keystone_model_tests`: PASS
- `region_grade_tests`: PASS
  - Subject center vs edge
  - Foliage vs subject
  - Sky vs lower frame
  - Background inverse of subject
  - +1 stop selected-region exposure
  - zero-mask exact bypass
  - -1 stop selected-region exposure
- `ci/source_sanity.py`: PASS
- `KeystoneOFX.cpp` C++17 host-source compile: PASS
- `ColorGradrBridge.cpp` C++17 compile: PASS
- `KeystoneCPU.cpp` C++17 compile: PASS
- Search for SceneGrade / SceneEngine / ncnn / ADE20K / OneGrade leftovers: none

The remaining deployment gate is the normal macOS universal GitHub Actions build and Resolve runtime test.
