# Keystone v1.5 — Native ColorGradr Engine Audit

## Parity architecture

Keystone v1.5 does not reimplement ColorGradr Fix math. It bundles the known-working
`colorgradr-main-license-bypass-v4-clean` OFX and loads its `OfxGetPlugin(0)` entry.
The child plugin receives the same Resolve OFX host pointer and the same effect instance,
Source clip, Output clip, render time, and render arguments.

- `colorgradr_fix` is the native ColorGradr push-button parameter, relabelled **Analyze Match**.
- `colorgradr_enable` is the native ColorGradr enable parameter, relabelled **Enable Match**.
- Instance-changed actions for `colorgradr_*` parameters are delegated to ColorGradr.
- Render is delegated to ColorGradr first. Keystone only begins processing after ColorGradr
  has written its native result into the Output image.
- Keystone keeps its own C++ instance state in a separate map, leaving ColorGradr's
  `kOfxPropInstanceData` untouched.
- **ColorGradr Only = On** returns immediately after the ColorGradr render. This is the
  diagnostic parity path and applies no Keystone math after the child OFX.

## Binary integrity

The v4-clean binary differs from the original ColorGradr binary at only 23 bytes; the
processing engine, embedded Fix commands, LUT resources, and OFX entry points are retained.
The v4-clean bundle is used to avoid reintroducing the license/expiry behavior already
removed in the user's previously working build.

## What "exact" means here

The ColorGradr stage is the actual ColorGradr OFX executable, not reconstructed histogram,
hue, affine, gamma, or LUT math. Therefore Keystone is not approximating that stage.

A macOS/Resolve runtime test is still required to verify that Resolve accepts nested
child-plugin delegation on the target machine. Linux source checks cannot execute a macOS
OFX binary. The GitHub macOS job builds/signs the wrapper and validates the nested bundle.
