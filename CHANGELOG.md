# Keystone v2.5.1

- Rebuilt Tone / Black Pt as a monotonic luminance-domain toe.
- Black (0.0) and middle gray (0.18) are fixed points.
- Removed per-channel exponential toe behavior that could create low-end chroma breakup.
- New black-point operation preserves XYZ chromaticity by applying one bounded scale to all channels.

## v2.5.1

- Replaced `Color / Sub Sat` with `Color / Pos Sat`.
- Pos Sat follows Primera Suite v0.6.0 HSV positive-saturation math in encoded LogC3.
- Range changed from 0–1.5 to Primera's 0–1 Pos Sat range.
- Density dye coupling remains as a separate `Color / Dye` stage.
- Removed subtractive-saturation runtime math and stale validator expectations.
- Updated tooltips, README, source audit, and code review.

## v2.4.2

- Removed the camera UV/IR RGB-proxy controls. The upstream feature is spectral and cannot be represented faithfully in a three-channel DCTL.
- Retained print C/M/Y CC filtration and density-look presets.
- Updated UI/tooltips/validation/docs for the reduced control set.

# Changelog

## 2.4.0

- Added source-parameterized camera UV/IR cut controls using an explicitly documented RGB projection of the upstream spectral band-pass equation.
- Added print-stage Cyan/Magenta/Yellow CC filtration; 100 CC = 1.0 optical density.
- Added `Split / Preset` and `Split / Preset Amt` to the density split-tone engine.
- Ported the shadow/highlight intent of Advanced Toning's Warm Cool, Cool Warm, Amber Cyan, Cyan Amber, Olive Cream, Teal Orange, Warm Vintage, Cool Silver, Sodium Cyan, and Moonlight Warm Skin families into density offsets.
- Added Keystone-authored Sepia, Bleach Cool, Golden Hour, Dusk Purple, and Tobacco Teal density looks.
- Presets are additive to the manual split density controls and preserve the split pivot.
- Film Strength now also scales print C/M/Y filtration and preset density offsets.
- Added hover help for every new control.
- Updated the documented Resolve workflow to include HB Color Separation, KH Gamut Compressor, Referent ODT, FilmBox Rec.709 look, and MonoNodes Balance Charts.
- Documented Advanced Toning and inactive LookLab WB as normally redundant in the current workflow.
- Extended static and numerical audits for UV/IR default identity, CC density convention, and density-preset pivot identity.

## 2.3.0

- Removed the fixed 3x3 Film Matrix from the UI and image path.
- Removed the former matrix licensing/runtime dependency.
- Kept the photochemical engine matrix-free.

### v2.4.2 hotfix
- Define `KS_PRINTER_PT` as `0.025f` log10 exposure per printer point; fixes Metal compile error in print-density profile.


## 2.6.1
- Withdraws v2.6.0 due to invalid DCTL helper ordering and mismatched runtime clamps.
- Density safety helpers are now defined before Film Response, Dye, and Split Tone.
- Runtime Dye limit is 1.0, printer RGB limits are 10–40, and print C/M/Y limits are ±40 CC.
- Keeps Color / Dye with near-neutral protection and smooth invalid-domain fadeout.
