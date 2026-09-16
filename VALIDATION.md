# Validation status

## Completed in the generation environment

- Current v4.5.11 Referent-only DCTL used as the math source.
- Original `Referent_LogC4_to_Rec709.cube` copied without modification.
- Cube SHA-256 verified as `19b2feb5ed8cb767d980e9f9b351b6e1823a3990974277fdb4a46d1f709d251c`.
- Cube parser verifies exactly 33^3 / 35,937 nodes.
- DCTL-derived shared CPU math compiles as C++17.
- OFX host source compiles as C++17 on Linux (Metal bridge excluded from Linux compile by design).
- Neutral analyzer model tests pass on neutral and warm-cast synthetic frames.
- CPU rendering model test returns finite output.
- Source sanity verifies that frame analysis is not called from render.
- Source sanity verifies continuous controls are exposed through the slider helper.
- Source sanity rejects ToneLab/Referent hybrid identifiers.

## Performed by GitHub Actions on macOS

The included workflow is designed to verify what cannot be tested in this Linux environment:

- Objective-C++ / Metal framework compile.
- `KeystoneKernels.metal` compile and `.metallib` link.
- Universal arm64 + x86_64 binary.
- Required OFX exports.
- Bundle `CFBundleExecutable` consistency.
- No Homebrew runtime dependencies.
- Ad-hoc code signature verification.
- Final release ZIP contents and original Referent LUT hash.

Until that workflow runs successfully, macOS/Resolve runtime compatibility should be considered build-candidate status rather than claimed as tested in Resolve.
