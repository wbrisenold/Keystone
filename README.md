# Keystone

Keystone is the technical grading hub in the Luma Color System. It handles primary balance, white balance, exposure/tone, color, skin, gamut/neutral cleanup, and technical finishing while remaining scene-referred and pre-ODT.

## Source

- Version: `1.0-RC26`
- DCTL: `Keystone-v1.0-RC26.dctl`
- Input/output: selected source space in, same source space out
- Placement: pre-ODT
- License: GPL-3.0-or-later
- SHA-256: `3f10ade4940d86483f870c8dc3f563c4867be59fad37413a71b14df0189c4143`

Keystone includes GPL-covered ME_Desatch-derived code and Primera-derived tone equations. See `THIRD_PARTY.md` for provenance and notices.

The optional Color Volume section remains removed. Creative white-point selection belongs in LookLab WB.

## RC26: Internal Film Negative Space

RC26 adds an optional reversible negative-style working-space sandwich inside Keystone. It follows the publicly documented signal model of Dec. 18 Studios' Film Negative Space CST at the behavior/topology level: scene-linear middle-gray remapping, separate power above and below the pivot, an optional smooth transition, printer-light-style density offsets, a creative grade in the middle, then the exact inverse.

This is an independent Keystone implementation. No Film Negative Space CST source code is copied into this repository.

### Signal order

When `Negative Space / Mode` is `Internal Sandwich`, Keystone routes processing as:

`input decode -> input repair/WB/native balance -> negative-space forward -> Keystone creative grade -> negative-space inverse -> output cleanup/safety -> output encode`

Inside the internal sandwich:

- Exposure / Black
- Contrast / Pivot / Shadows / Highlights / Roll Off
- Chroma / Vibrance / Mid Chroma / Hue
- Skin hue/chroma/evenness/exposure

Outside the sandwich:

- Input gamut repair
- Temperature / Tint
- Native RGB balance
- White/black output cleanup
- Catastrophic encode safety
- ME_Desatch-compatible encoded-output stage

This keeps technical normalization and delivery safety in known source-space math while giving Keystone's creative controls the altered response of the negative-style working space.

### Negative Space controls

- `Negative Space / Mode` — Off or Internal Sandwich. Default: Off.
- `Negative / Mid In` — source scene-linear pivot. Range `0.01-1.0`; default `0.18`.
- `Negative / Mid Out` — remapped pivot used inside the sandwich. Range `0.01-1.0`; Keystone starting default `0.42`.
- `Negative / Below` — power below the pivot. Range `0.1-3.0`; default `1.0`.
- `Negative / Above` — power above the pivot. Range `0.1-3.0`; default `1.0`.
- `Negative / Blend` — smooth transition around the pivot. Range `0.0-0.5`; default `0.0`.
- `Negative / Printer R/G/B` — per-channel log-density working-space offsets. Range `0-50`; `25` is neutral.

The forward and inverse transforms cancel when no creative Keystone control is moved. RC26 therefore keeps exact neutral/bypass behavior even when the internal sandwich is enabled and its negative-space parameters are non-neutral.

`Mid Out = 0.42` is a Keystone creative starting point, not a claim of byte-for-byte matching to the external CST. Copy the settings you prefer from an external negative-space workflow when you want to approximate the same working response.

## RC25 safety retained

RC26 retains the RC25 production-safety fixes:

- `Tone / Highlights` is continuous and strictly monotonic through code value 1.0 and extended highlights.
- Transfer-aware catastrophic-negative protection runs before final encode.
- A final encoded-domain safety pass runs after ME_Desatch.
- Ordinary extended values are preserved until the catastrophic safety threshold is reached.

RC26 adds an additional catastrophic-only scene ceiling around the internal negative inverse. The supported negative powers can mathematically magnify an extreme legal grade to millions of scene-linear units; values within `+/-8192` are untouched, while larger excursions are uniformly scaled to preserve RGB ratios instead of overflowing.

## Placement

For the user's AWG3 / LogC3 film-matrix workflow, use:

1. Camera/CST or input transform
2. [LookLab WB](https://github.com/wbrisenold/LookLab-WB)
3. FilmMatrix or film-matrix prep
4. [Advanced Toner](https://github.com/wbrisenold/AdvancedToner) while the signal is still AWG3 / LogC3
5. Keystone, optionally with `Negative Space / Mode = Internal Sandwich`
6. [PresenceOFX](https://github.com/wbrisenold/PresenceOFX)
7. other scene-referred finishing such as HB ColorSeparation when configured for the current log/gamut
8. ODT/display transform
9. optional display-referred look LUTs
10. final diagnostics

The internal Keystone sandwich affects Keystone only. If PresenceOFX or another external node must also react inside negative space, keep a separate external Forward/Invert sandwich around those nodes rather than assuming Keystone's internal mode changes downstream nodes.

FilmMatrix credit: the PD FilmMatrix node refers to [`PD-LogC3-FilmMatrix.dctl`](https://github.com/mikaelsundell/photographic-dctls/blob/master/PD-LogC3-FilmMatrix.dctl) from Mikael Sundell's `photographic-dctls` repository. It is not included in Keystone.

## Install on macOS

Run:

```bash
scripts/INSTALL.command
```

The installer removes older `Keystone*.dctl` files from Keystone's own Resolve folder before copying RC26, preventing duplicate RC entries.

Refresh Resolve's LUT/DCTL list or restart Resolve after installing. To remove Keystone, run `scripts/UNINSTALL.command`.

## Validation

Run both release gates:

```bash
python3 ci/validate_dctl.py
python3 ci/behavioral_validate.py
```

The behavioral gate compiles the DCTL math into a CPU harness with the final finite fallback disabled. It checks transfer/matrix round trips, RC25 off-path golden vectors, exact neutral identity, negative-space forward/inverse behavior, negative-space monotonicity, highlight monotonicity, encoded safety, 2.7 million randomized full-transform evaluations, and a separate blended-negative stress set.

## Runtime qualification

RC26 is a production-ready code/repository candidate. Static and CPU behavioral validation cannot reproduce Resolve's DCTL compiler or the target GPU backend. Before locking a studio image, verify DCTL load, neutral difference, and a short gradient/stress chart on the exact Resolve/GPU deployment host.
