# Keystone v1.4 — Keystone Skin / Keystone UV-IR audit

## Keystone source
- Source: user-supplied `Keystone.ofx`.
- Recovered skin parameter IDs: `SkinEnable`, `SkinPreset`, `SkinSaturate`, `SkinColour`, `SkinPop`, `SkinCenter`, `SkinRange`, `SkinBrightness`, `SkinBrightnessRange`, `ShowSkinSelection`, `SkinIntensity`.
- Recovered preset names: Custom; Vibrant; Muted; Cool / Neutralize Red; Warm / Add Warmth; Matte / Reduce Shine.
- Recovered slider ranges/defaults:
  - Saturate: -1..1, default 0.2
  - Luminance/density: -1..1, default 0
  - Hue rotation: -60..60 deg, default 0
  - Tone center: -30..30 deg, default 0
  - Tone range: 15..65 deg, default 40
  - Brightness center: 0..1, default 0.5
  - Brightness range: 0.25..1.75, default 1.0
  - Intensity: 0..200, default 100

### Precision boundary
Keystone's HSL hue/lightness selector geometry is recovered and used. The host-side numeric recipes loaded by the six Skin presets have not yet been fully recovered from the binary. Keystone v1.4 therefore uses conservative recipes matching the recovered preset intent; these recipes are explicitly not represented as bit-identical Keystone preset values.

## Keystone source
- Source: user-supplied `keystone_filterfilm-OFX-macOS.pkg`.
- UV default cutoff: 410 nm; transition width retained internally at 8 nm.
- IR default cutoff: 675 nm; transition width retained internally at 15 nm.
- The source erf cutoff law is retained.
- Keystone applies filters to wavelength-resolved data. Keystone has only AWG4 RGB, so v1.4 evaluates the cutoff at a documented 610/550/450 nm RGB basis and mean-normalizes transmission. This is a compact projection, not a spectral clone.
