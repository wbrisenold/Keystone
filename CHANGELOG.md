# Changelog

## v1.13.1

- Updated `Color / Oklab Pos` to be density-coupled instead of purely scene-Y-preserving.
- Oklab chroma still scales by `2^amount`, with Oklab `L` and hue angle held stable.
- Added a modest density response driven by achieved relative chroma gain.
- Density response fades in only when the source already has meaningful chroma, so near-neutrals stay largely unaffected.
- `Color / Pos Sat` remains unchanged for direct A/B comparison.

## v1.13.0

- Added `Color / Oklab Pos` as a second, separate positive-only saturation slider.
- `Color / Oklab Pos` multiplies Oklab chroma by `2^amount` while keeping Oklab `L` and hue angle fixed.
- The Oklab result is converted back to XYZ and rescaled to preserve original scene `Y`.
- Preserved the existing Primera `Color / Pos Sat` slider so both saturation styles can be compared directly.
- For A/B testing, set one saturation slider to zero while adjusting the other.
- Preserved Primera Black Point, Primera Contrast, v1.7 Studio WB, and automatic Output cleanup skin protection.

## v1.12.0

- Removed all six user-facing Skin controls and removed the skin-shaping processing pass.
- Retained a small internal skin qualifier only for automatic `Out / White` and `Out / Black` skin protection.
- Replaced Keystone's former Black control math with Primera's exact `soft_black_point` behavior.
- `Tone / Black Pt` now uses Primera's `-0.05` to `+0.05` range and `0.0001` step.
- Black Point now runs independently on scene-linear AWG3 R/G/B immediately after Exposure, matching Primera's placement.
- Preserved Primera rolling Contrast, Primera Pos Sat, v1.7 Studio WB, and Output cleanup skin protection.

## v1.11.0

- Replaced Daniele contrast with Primera's exact rolling-contrast equation.
- `Tone / Contrast` now operates independently on AWG3 / LogC3 RGB channels at Primera's default Pivot `0.0`, using encoded 18% gray as the pivot.
- The per-channel contrast intentionally produces Primera's contrast-driven saturation/color-separation behavior.
- Contrast slider precision now matches Primera at `0.001`.
- Replaced the later WB Hue/Amt/Warm/Tint controls with the exact WB implementation from `Keystone-v1_7-STUDIO-PRODUCTION-CANDIDATE.dctl`.
- WB is now `WB / Temp K` (2500K-20000K, 6504K default) plus `WB / Tint` (-100..100).
- Preserved `Color / Pos Sat`, Primera-style Skin controls, and Output cleanup skin protection.

## v1.10.0

- Replaced Keystone's former `Color / Sat` control with `Color / Pos Sat`.
- `Color / Pos Sat` now follows Primera's positive-saturation behavior:
  HSV saturation is multiplied by `2^amount` and capped at `1.0`.
- Saturation is performed in Keystone's fixed AWG3 / LogC3 EI800 encoded signal, matching Primera's encoded-domain approach.
- Slider range is now `0` to `1`, default `0`.
- Removed Keystone's former Oklab global saturation expansion/density implementation.
- Kept Daniele as Keystone's contrast engine; contrast behavior was not changed in this release.
- Kept v1.9.9 output-cleanup skin protection, v1.9.7 WB trims, and the Primera-style Skin section.

## v1.9.9

- Fixed Resolve/Metal compile error in output-cleanup skin protection.
- Replaced stale `skin_confidence_xyz()` call with the current skin path:
  `ks_skin_rg()` -> `ks_skin_mask_rg()`.
- Output-cleanup behavior is otherwise unchanged from v1.9.8.
- Added a stronger source audit for unresolved function calls.

## v1.9.8

- Added automatic skin protection to `Out / White` and `Out / Black`.
- Output cleanup now reuses Keystone's skin qualification and attenuates cleanup on confident skin.
- Skin protection is automatic and does not add another UI slider.
- A broadened minimum protection range is used so tight Skin settings do not accidentally disable protection.
- Kept the v1.9.7 WB trims, v1.9.6 Primera-style Skin section, and split-tone removal.

## v1.9.7

- Kept `WB / Hue` and `WB / Amt` as the primary HDR-style white-balance controls.
- Added `WB / Warm` and `WB / Tint` as precision trim sliders.
- Warm/Tint trims are limited to 25% of the main WB wheel radius.
- Trim response is signed-square for finer control near zero.
- Main Hue/Amt and trims are combined into one source-white vector before a single Bradford CAT.
- `WB / Warm = 0` and `WB / Tint = 0` reproduce the v1.9.6 WB result exactly.
- Kept the v1.9.6 Primera-style Skin section and v1.9.5 split-tone removal.

## v1.9.6

- Rebuilt the Skin section so the controls behave like a Primera-style skin module rather than Keystone's earlier custom Oklab skin shaper.
- Skin qualification is now centered on brightness-independent normalized skin chromaticity.
- `Skin / Hue` now rotates skin around the skin center.
- `Skin / Sat` now changes distance from the skin center.
- `Skin / Dense` now behaves like density: slight darkening/lightening plus chroma enrichment/relaxation.
- `Skin / Even` now pulls blotchy skin back toward the skin center.
- `Skin / Sep` still focuses the qualifier on the most confident skin pixels.
- Kept the 1.9.5 split-tone removal and the 1.9.4 WB Hue / Amount interface.

## v1.9.5

- Removed split toning completely.
- Removed all four split-tone UI controls.
- Removed the split-tone Oklab injection math and transform-stage call.
- Keystone now flows directly from Skin controls to Output cleanup.
- UI control count reduced from 25 to 21.
- Kept WB Hue / Amount, current Skin controls, tone controls, saturation, and output cleanup unchanged.

## v1.9.4

- Added GitHub release automation: changing `VERSION` and the DCTL header now publishes a new tagged GitHub Release from `main`.

- Restored `WB / Hue` and `WB / Amt`.
- Kept the HDR-style source-white displacement and Bradford CAT white-balance model.
- Retained the v1.9 skin controls.
- Retained the skin-angle compile fix that removes symbolic `PI` and `while` wrapping.
- Kept the shortened Resolve UI labels.

## v1.9.x

- Added selective skin controls:
  - Range
  - Hue
  - Saturation
  - Density
  - Evenness
  - Separation
- Added an improved skin qualification model using normalized chromaticity plus perceptual and luminance gates.

## v1.8

- Replaced conventional white-balance controls with an HDR-wheel-style Hue / Amount source-white model.
- Bradford chromatic adaptation retained underneath.

## v1.7

- Added shadow/highlight split-tone controls.
- Cleaned Resolve parser-sensitive UI labels.
- Removed dead Metal code.

## v1.4+

Major corrective rewrite after code audit:

- replaced unstable saturation math with bounded Oklab chroma scaling
- moved Daniele contrast to scene luminance
- rewrote film-negative inverse in stop space
- improved safety/fallback behavior
- removed dead Studio code paths
