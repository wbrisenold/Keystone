# KeystoneOFX Validation

## Source Sanity
- Rejects hybrid identifiers in generated sources.
- Verifies Referent cube SHA-256 before packaging.

## Build Validation
- Linux model tests pass (C++17, -Werror=return-type).
- macOS universal build produces arm64 + x86_64 binary.
- Metal shader compiles without errors.
- OFX export symbols present: OfxGetNumberOfPlugins, OfxGetPlugin, OfxSetHost.
- No Homebrew runtime dependencies.
- Ad-hoc codesign verifies.

## Bundle Validation
- Info.plist CFBundleExecutable matches binary name.
- Resources present: KeystoneKernels.metallib, Referent_LogC4_to_Rec709.cube.
- Referent cube hash matches expected value.
