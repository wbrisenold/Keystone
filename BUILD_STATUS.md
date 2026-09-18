# Keystone v1.6.1 Scene Grade load-safe build status

This archive is based on commit `7c30c7739de95009c473da5515f0508a1e825ce4` from `wbrisenold/Keystone`, plus the Scene Grade integration and load-safety fix.

Validated in the generation environment:

- `ci/source_sanity.py`: PASS
- existing `keystone_model_tests`: PASS
- full non-macOS CMake configure/build: PASS
- OFX `dlopen` + exported-entry-point + host-suite handshake smoke test: PASS
- `src/KeystoneOFX.cpp` C++17 compile: PASS
- `src/ColorGradrBridge.cpp` C++17 compile: PASS
- `src/SceneGrade.cpp` C++17 compile without ncnn: PASS
- main Scene Grade translation unit contains no ncnn symbols/includes: PASS
- bundled semantic-model SHA-256 checks: PASS
- retired upstream feature name absent from Scene Grade source and README: PASS

Load-safety architecture:

- `KeystoneOFX.ofx` does not link ncnn or the scene sidecar.
- `KeystoneSceneEngine.dylib` is built separately on macOS and copied into `Contents/Resources`.
- The sidecar is opened only when **Analyze Scene** is pressed.
- Missing/failed sidecar falls back to Keystone's deterministic region heuristic rather than preventing plugin load.
- macOS CI verifies the main OFX and sidecar are both universal arm64+x86_64, verifies the sidecar export, checks runtime dependencies, and signs both.

The macOS OFX binary is not prebuilt in this source archive. GitHub Actions builds and packages `KeystoneOFX-macOS-universal.zip`.
