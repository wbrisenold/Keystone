# Changelog

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
