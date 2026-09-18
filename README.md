# KeystoneOFX — Region Grade build

This branch starts from the last known-good pre-port Keystone v1.6.0 LoadSafe source. The original load-safe architecture is preserved: one Keystone OFX binary, CPU rendering, no nested plug-ins, no inference runtime, no sidecar, and no new load-time dependency.

## Region Grade

Region Grade adds local controls without replacing Keystone's grading pipeline.

Regions:
- Subject
- Background
- Sky
- Foliage / Trees
- Water
- Ground
- Terrain
- Built Environment

Controls:
- Exposure
- Contrast
- Saturation
- Temperature
- Amount
- Feather
- Show Mask

The local grade is applied in AWG4 scene-linear after Auto Match and the existing global tone/color preparation, before Keystone's split-tone/look/finish stages.

### How the masks work

This build intentionally does not add a neural-network runtime. Region masks are generated inside Keystone from image math already available to the plugin:

- Subject combines Keystone skin evidence with a soft spatial/body expansion around detected skin.
- Background is the inverse of the subject mask.
- Sky uses upper-frame position, blue/cyan chroma, brightness, and a bright-neutral cloud allowance.
- Foliage / Trees uses green chroma evidence.
- Water uses cyan/blue chroma separated from likely sky by frame position.
- Terrain uses brown/earth chroma and lower-frame evidence.
- Ground uses lower-frame evidence while excluding stronger foliage, water, and subject evidence.
- Built Environment is the remaining low-chroma/background structure after stronger semantic-style regions are removed.

These are heuristic masks, not object-recognition AI. **Show Mask** is included so the selection can be judged before grading.

## Auto Match

Press **Analyze Match** on a representative frame. Keystone analyzes that frame once and stores the correction in the effect instance. **Reset Neutral** clears it.

## Build

The repository includes `.github/workflows/build.yml`. Push the files with `.github` at the repository root. GitHub Actions builds a universal `arm64 + x86_64` macOS OFX and uploads `KeystoneOFX-macOS-universal.zip`.

Local validation includes:
- existing Keystone model tests
- Region Grade synthetic mask tests
- OFX dynamic-loader lifecycle smoke test
- source sanity checks

## Install

Copy `KeystoneOFX.ofx.bundle` to `/Library/OFX/Plugins/`, remove quarantine if macOS applied it, then restart Resolve.

```bash
sudo xattr -dr com.apple.quarantine /Library/OFX/Plugins/KeystoneOFX.ofx.bundle
sudo codesign --verify --deep --strict /Library/OFX/Plugins/KeystoneOFX.ofx.bundle
```

## Load-safety rule

Region Grade must not add a new library to Keystone's runtime dependency list. If a future detector needs a model/runtime, it should be added only after this dependency-free version is proven stable in Resolve.

## License

GPL-3.0-only.
