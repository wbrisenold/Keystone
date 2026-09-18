# Region Grade validation

Base: `Keystone-v1.6.0-LoadSafe-GitHub-Ready.zip`

The Region Grade branch changes only:
- `CMakeLists.txt` (adds the Region Grade test target)
- `src/KeystoneParams.h`
- `src/KeystoneCoreCPU.h`
- `src/KeystoneCPU.cpp`
- `src/KeystoneOFX.cpp`
- `tests/region_grade_tests.cpp`
- documentation

No new dynamic library, model, sidecar, nested OFX, Metal framework, or inference runtime is added.

Validated in the generation environment:
- `keystone_model_tests`: PASS
- `region_grade_tests`: PASS
- `ofx_loader_smoke`: PASS
- `ci/source_sanity.py`: PASS
- CMake Release build on Linux: PASS

The final macOS/Resolve runtime gate still requires the universal GitHub Actions artifact to be loaded in DaVinci Resolve.
