# KeystoneOFX v1.5.3

- Fixed Resolve discovery/load failure caused by changing the embedded match engine executable basename.
- The repo stores that executable under a neutral filename; the macOS build restores its private runtime basename only inside the finished bundle.
- Removed embedded-engine loading from `OfxSetHost` so OFX host discovery is non-reentrant.
- Embedded-engine errors no longer propagate as Keystone load failures.
- Added Keystone-owned clip definitions and a direct Keystone render fallback.
- Added `/tmp/KeystoneOFX-loader.log` and `scripts/diagnose_macos.sh` for runtime diagnostics.
- README and source contain Keystone-only product naming; required legal notices remain isolated in `THIRD_PARTY_NOTICES.md`.
