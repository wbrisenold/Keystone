# Keystone

Keystone is the technical grading hub in the Luma Color System. RC27 uses one permanent internal negative-response tone architecture while remaining scene-referred and pre-ODT.

## Source

- Version: `1.0-RC27`
- DCTL: `Keystone-v1.0-RC27.dctl`
- Input/output: selected source space in, same source space out
- Placement: pre-ODT
- License: GPL-3.0-or-later
- SHA-256: `71866fafd1a3cad917c31e4db936ca01c04b9c7f47cb1d1366faf3dc5cebdf91`

Keystone includes the GPL-covered ME_Desatch compatibility module and Primera-derived tone equations. See `THIRD_PARTY.md`.

## RC27 architecture

RC27 removes the user-facing Negative Space on/off mode. Negative-response tone development is now Keystone's fixed internal architecture. Neutral settings still return the input bit-for-bit through the explicit neutral fast path; there is no hidden look.

Signal order:

`input decode -> gamut repair/WB/native balance -> negative-response ENTER -> printer lights -> tone -> negative-response EXIT -> Chroma/Vibrance/Hue -> skin -> ME_Desatch working-transfer stage -> output cleanup/safety -> output encode`

### Negative-response controls

- `Negative / Mid` — remapped middle-gray location inside the tone-development space; default `0.42`, range `0.10-0.80`.
- `Negative / Below` — response power below middle gray; default `1.0`, range `0.1-3.0`.
- `Negative / Above` — response power above middle gray; default `1.0`, range `0.1-3.0`.
- `Negative / Printer R/G/B` — printer-light-style density grade; `25/25/25` is neutral.

Scene middle gray entering the internal stage is fixed at `0.18` because Keystone has already decoded the selected input transfer to scene-linear light. The crossover blend around the pivot is an internal technical constant rather than a grading control.

Printer lights are a real grade inside the sandwich. RC27 does not divide them back out on exit.

### Tone

Exposure, Black, Contrast, Pivot, Shadows, Highlights, and Roll Off operate directly on the internal negative-response values. RC27 no longer re-encodes those values through the selected camera/log transfer while inside the negative stage.

The RC25 monotonic Highlights repair remains in place.

### Color

After Negative Space EXIT, Keystone returns to the normal scene-referred color domain before colorimetric operations.

The general Color section is intentionally reduced to:

- `Color / Chroma`
- `Color / Vibrance`
- `Color / Hue`

`Color / Mid Chroma` was removed because it overlapped the main chroma tools and added another competing saturation behavior.

Skin controls also operate after Negative Space EXIT.

### ME_Desatch controls retained

The full selective desaturation set remains:

- `Sat / Global`
- `Sat / Red`
- `Sat / Green`
- `Sat / Blue`
- `Sat / Cyan`
- `Sat / Magenta`
- `Sat / Yellow`

RC27 does **not** apply these directly to scene-linear RGB and does not leave them as a final encoded tail operation. ME_Desatch is code-value RGB math, so Keystone temporarily encodes the current scene-referred result in the selected native transfer, applies the exact compatibility module, guards catastrophic encoded negatives, then immediately decodes back to scene-linear before output cleanup.

When ME_Desatch is the only active module, Keystone preserves exact standalone module behavior.

## Placement

Recommended current pipeline:

1. Camera/CST to the desired scene-referred grading space
2. LookLab WB
3. PD LogC3 FilmMatrix when using the AWG3 / LogC3 workflow
4. Advanced Toner in its documented AWG3 / LogC3 space
5. Keystone RC27
6. PresenceOFX
7. scene-referred finishing such as HB ColorSeparation configured for the current space
8. ODT
9. optional display-referred LUT
10. diagnostics

Keystone's internal Negative Space affects Keystone only. It does not put downstream OFX/DCTL nodes into the internal negative-response space.

## Install on macOS

Run `scripts/INSTALL.command`. The installer removes older `Keystone*.dctl` files from Keystone's own Resolve folder before installing RC27, preventing duplicate releases.

Run `scripts/UNINSTALL.command` to remove Keystone.

## Validation

```bash
python3 ci/validate_dctl.py
python3 ci/behavioral_validate.py
```

The behavioral gate compiles the actual DCTL math into a CPU harness with the emergency finite fallback disabled and runs 2.7 million deterministic randomized transforms in addition to targeted production gates.

## Runtime qualification

RC27 is a production-ready code/repository candidate. Static and CPU behavioral validation cannot reproduce DaVinci Resolve's DCTL compiler or the target GPU backend. Before locking a studio deployment, verify DCTL load, neutral difference, printer-light response, a smooth highlight ramp, and representative selective-desaturation moves on the actual Resolve/GPU host.
