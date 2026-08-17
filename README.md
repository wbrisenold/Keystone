# Keystone

Keystone is the technical grading hub in the Luma Color System. RC28 keeps the internal Film Negative Space development model, but removes technical plumbing from the UI and upgrades the tone/color engine around automatic scene-referred anchors.

## Source

- Version: `1.0-RC28`
- DCTL: `Keystone-v1.0-RC28.dctl`
- Input/output: selected source space in, same source space out
- Placement: pre-ODT
- License: GPL-3.0-or-later
- SHA-256: `a4ed9deb235c33e81ae65a01a444f929d1d98389f133dbf2ee0db7f5aed5ff7a`

Keystone includes the GPL-covered ME_Desatch compatibility module and retains Primera's soft black-point equation. See `THIRD_PARTY.md`.

## RC28 architecture

Signal order:

`input decode -> repair/WB -> true scene Exposure -> Film Negative Space ENTER -> printer lights -> Black/Contrast/Shadows/Highlights -> Film Negative Space EXIT -> monotonic Roll Off -> Chroma/Vibrance/Hue -> skin -> ME_Desatch working-transfer stage -> White/Black Clean -> technical safety -> native encode`

Neutral defaults remain an exact input bypass through the explicit neutral fast path. RC28 does not add an automatic look.

## What is automatic

RC28 automatically derives technical decisions that should not require grading sliders:

- Contrast pivot from the selected input transfer/EI and 18% scene gray.
- Shadow and highlight zone boundaries in scene stops relative to that gray anchor.
- Film Negative Space Mid/Above/Below response parameters; these are fixed internal plumbing and no longer exposed.
- Destination/native-gamut limits for positive Chroma, Hue rotation, Vibrance, and skin color edits.
- Skin protection for White Clean and Black Clean.
- Catastrophic negative/overflow and encode-domain safety.
- Roll Off knee placement in scene exposure.

Keystone never chooses the creative amount of Exposure, Contrast, Shadows, Highlights, Roll Off, color, skin, printer lights, or clean controls for the user.

## UI

### Input

- `Input / Space`
- `Input / EI`
- `Input / Repair`

### WB

- `WB / Temp`
- `WB / Tint`

The former native `Balance / R/G/B` trims were removed. In the current Luma pipeline, LookLab handles technical WB upstream and Keystone's printer lights provide the intended negative-style RGB balance.

### Negative Development

- `Negative / Printer R`
- `Negative / Printer G`
- `Negative / Printer B`

`25 / 25 / 25` is neutral. Printer lights are a real grade inside the reversible negative working stage and are not canceled on exit.

### Tone

- `Tone / Exposure` — true scene-linear stop gain (`2^stops`) before negative development.
- `Tone / Black` — soft luma-preserving black-point adjustment.
- `Tone / Contrast` — luminance-driven contrast inside negative development, pivoted automatically from selected space/EI and 18% gray.
- `Tone / Shadows` — automatic shadow-zone exposure; protects the deepest toe and is gone before middle gray.
- `Tone / Highlights` — automatic highlight-zone exposure; begins above middle gray and reaches full influence around +2 EV.
- `Tone / Roll Off` — automatic-knee, luminance-preserving monotonic shoulder after negative development.

The manual Pivot slider was removed. Negative Mid/Below/Above controls were also removed because they were response-definition parameters, not useful standalone grades.

### Color

- `Color / Chroma`
- `Color / Vibrance`
- `Color / Hue`

RC28 adds destination-gamut-aware limiting to Chroma and Hue. Positive Chroma expansion is allowed until the actual selected/native gamut boundary is approached, then smoothly limited. Hue rotation is also constrained so a legal color rotation cannot silently create invalid native RGB.

### Skin

- `Skin / Target`
- `Skin / Hue`
- `Skin / Chroma`
- `Skin / Exposure`
- `Skin / Hue Uniformity`

`Hue Uniformity` is the renamed former `Evenness` control. It converges selected skin hues toward the target while preserving the intended color-only behavior; it is not spatial skin smoothing.

### Selective DeSatch

- `Sat / Global`
- `Sat / Red`
- `Sat / Green`
- `Sat / Blue`
- `Sat / Cyan`
- `Sat / Magenta`
- `Sat / Yellow`

The exact ME_Desatch-compatible cone-coordinate behavior is retained in the selected native encoded working transfer, then decoded immediately back to scene-linear before output cleanup.

### Output

- `Output / White Clean`
- `Output / Black Clean`

Both are manual creative cleanup controls. Skin protection is automatic. The former `Output / Negatives` and `Output / Skin Protect` sliders were removed; catastrophic-negative and encode safety are always on and cannot be disabled.

## Recommended pipeline

1. Camera/CST to the desired scene-referred grading space
2. LookLab WB
3. PD LogC3 FilmMatrix when using the AWG3 / LogC3 workflow
4. Advanced Toner in its documented AWG3 / LogC3 space
5. Keystone RC28
6. PresenceOFX
7. scene-referred finishing such as HB ColorSeparation configured for the current space
8. ODT
9. optional display-referred LUT
10. diagnostics

Keystone's internal Negative Space affects Keystone only. Downstream nodes remain in the selected source/output grading space.

## Install on macOS

Run `scripts/INSTALL.command`. The installer removes older `Keystone*.dctl` releases from Keystone's own Resolve folder before installing RC28.

Run `scripts/UNINSTALL.command` to remove Keystone.

## Validation

```bash
python3 ci/validate_dctl.py
python3 ci/behavioral_validate.py
```

The behavioral gate compiles the actual DCTL math into a CPU harness with the emergency finite fallback disabled and runs 2.7 million deterministic randomized transforms plus targeted tone-zone, pivot, color-boundary, cleaner, printer-light, and ME_Desatch tests.

## Runtime qualification

RC28 is a production-candidate code/repository build. Static and CPU behavioral validation cannot reproduce DaVinci Resolve's DCTL compiler or target GPU backend. Before a studio lock, verify DCTL load, neutral difference, printer-light response, shadow/highlight isolation, smooth roll-off ramps, White/Black Clean, and representative color/desaturation moves on the target Resolve/GPU host.
