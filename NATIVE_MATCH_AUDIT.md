# Keystone v1.5 — Native Match Engine Audit

Keystone v1.5 does not reimplement match math. It bundles the known-working OFX and loads its `OfxGetPlugin(0)` entry.
- `fix` is the native push-button parameter, relabelled **Analyze Match**.
- `enable` is the native enable parameter, relabelled **Enable Match**.
- Instance-changed actions for `fix`/`enable` parameters are delegated to the match OFX.
- Render is delegated to the match OFX first. Keystone only begins processing after the match OFX renders.
- Keystone keeps its own C++ instance state in a separate map, leaving the match OFX's state untouched.
- **Match Only = On** returns immediately after the match OFX render. This is the diagnostic mode.
The bundled binary differs from the original binary at only 23 bytes; the match stage is the actual OFX executable, not a reconstructed histogram/hue/gain approximation.
