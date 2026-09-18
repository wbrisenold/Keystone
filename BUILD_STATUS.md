# Keystone v1.6 Scene Grade build status

This archive is based on commit `7c30c7739de95009c473da5515f0508a1e825ce4` from `wbrisenold/Keystone`.

Validated in the generation environment:

- `ci/source_sanity.py`: PASS
- existing `keystone_model_tests`: PASS
- `src/KeystoneOFX.cpp` C++17 compile: PASS
- `src/ColorGradrBridge.cpp` C++17 compile: PASS
- `src/SceneGrade.cpp` C++17 fallback compile: PASS
- standalone Scene Grade heuristic render/solve test: PASS
- bundled semantic-model SHA-256 checks: PASS
- retired upstream feature name absent from Scene Grade source and README: PASS

The macOS OFX binary is intentionally not prebuilt here. GitHub Actions builds the universal arm64+x86_64 bundle on macOS, fetches the pinned ncnn source revision, compiles the semantic model path, signs the bundle ad-hoc, and emits `KeystoneOFX-macOS-universal.zip`.
