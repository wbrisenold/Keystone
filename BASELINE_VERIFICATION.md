# Baseline verification

Region Grade was built from the code corresponding to repository commit:

`18ca0ee641997b3aabf709feebff0523d671cfda`

This was verified from repository history and code, not from archive names.

## Commit boundary

- `f6a23674f68a7dcd0dd621d719507f13e6f56ce6` -> initial Keystone v1.5 code.
- `757e46c68d199006b42a6e3d2680101b603b276f` -> README only.
- `18ca0ee641997b3aabf709feebff0523d671cfda` -> README only.
- `1b2844bd8937d39e39d43568b94c52e6e6b27301` -> first later scene-port change; modifies CMake, workflow, KeystoneOFX, KeystoneCoreCPU, Metal bridge/shader params and adds scene/model files.

Therefore the plugin code at `18ca0ee` is byte-identical to the initial v1.5 plugin code.

## Verified Git blob hashes before Region Grade changes

- `CMakeLists.txt`: `cce00efbde6c891e21799e5abb9d9f1855d3243e`
- `src/KeystoneOFX.cpp`: `9f290c5a8bc0062f52dc07fdc3451b0cfd514143`
- `src/KeystoneCoreCPU.h`: `898cfe92cf0bdf5f6dac29c80a0785cbcccea811`
- `src/KeystoneParams.h`: `7f5867a65d3de998bbbe51f2b3d2597cbda0cd03`
- `src/MetalBridge.mm`: `ef6061b3ae3d8ab9c1db6fcb069c291507b42b6b`
- `shaders/KeystoneShared.metalh`: `c77bb6f38cfd04abca9e758237a524255b938e68`
- `scripts/build_macos.sh`: `5597230302843bddf3f9e1e4585f159b627b3869`
- `.github/workflows/build.yml`: `ab0512846a075f64494d55102fead9af3184e233`

Region Grade does not use or include SceneGrade, SceneEngine, ncnn, ADE20K, OneGrade, or a sidecar.
