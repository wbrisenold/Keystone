# Keystone v2.6.1

Keystone is the primary balance and tone stage in a small DaVinci Resolve system. It works in **ARRI Wide Gamut 3 / LogC3 EI800**, handling exposure, tone, white balance, gamut/neutral cleanup, density color, and technical finishing before the display transform.

This is not a collection of unrelated nodes. The companion workflow uses CST for the camera transform, PresenceOFX for spatial image character, Keystone for technical balance, Henry Bobeck's paid [Color Separation DCTL](https://henrybobeck.com/dctl/ColorSeparation) for the dedicated separation stage, a Referent ODT, a display-referred look LUT, and MonoNodes charts for final display QC. The Color Separation DCTL is Henry Bobeck's product. If you use it, support its creator by purchasing it from the official page.

## Keystone's role

Keystone is the point where the image gets technically organized before it is
shown through a display transform. It is responsible for balance, tone,
density behavior, gamut and neutral cleanup, and the safety checks that keep
creative adjustments from turning into broken output. PresenceOFX comes before
it; Referent, the look LUT, and the chart/QC tools come after it.

Keystone does not replace the other stages. Keeping those stages separate makes
it easier to see whether a problem comes from the camera transform, image
character, balance, separation, display foundation, or look.

## What changed in v2.6.1

### Enlarger C / M / Y filtration

`Print / C CC`, `Print / M CC`, and `Print / Y CC` live between the negative and print stages. The source convention is Kodak CC optical-density units: **100 CC = 1.0 density = 10% transmission**. Keystone translates positive C/M/Y to reduced red/green/blue print exposure respectively. The controls are neutral at 0 and require Film Response to be active.

### Density-look presets

`Split / Preset` and `Split / Preset Amt` feed directly into Keystone's density split-tone engine. Manual `Split / Sh R/G/B` and `Split / Hi R/G/B` remain available as trims on top of the preset. The pivot remains untouched by construction, so using a preset does not silently tint the protected middle-gray zone.

The first ten presets preserve a warm/cool shadow/highlight intent while translating the look into density offsets instead of running a separate three-zone Oklab engine:

- Warm Cool
- Cool Warm
- Amber Cyan
- Cyan Amber
- Olive Cream
- Teal Orange
- Warm Vintage
- Cool Silver
- Sodium Cyan
- Moonlight Warm Skin

Additional Keystone-authored familiar tone families are:

- Sepia
- Bleach Cool
- Golden Hour
- Dusk Purple
- Tobacco Teal

These are creative starting points, not claims of matching a named film stock, lab process, or commercial LUT.

## Processing order

Input repair -> Bradford WB -> scene exposure -> soft black point -> negative-space RGB printer trims -> LogC3 rolling contrast -> shadows/highlights -> rolloff -> H&D negative development + optional DIR -> enlarger C/M/Y filtration + master print exposure -> print H&D -> density subtractive color -> density look/split tone -> Safe guard -> white/black cleanup with skin protection -> technical guards -> LogC3 EI800.

## Current Resolve workflow

Recommended working tree for the current setup:

```text
CST: camera -> AWG3 / LogC3
    -> PresenceOFX
    -> Keystone
    -> HB Color Separation DCTL
    -> Referent ODT: LogC3 -> display space
    -> Look LUT: display-referred look
    -> MonoNodes Chart DCTL: final chart / display QC
```

The system boundary is deliberate: keep technical work in LogC3 until Keystone is complete, then use [Referent](https://cullenkellycolor.com/toolkit/referent), Cullen Kelly's free viewing LUT and display foundation, to move into display space before the look LUT and chart check. [MonoNodes](https://mononodes.com/dctls/) publishes DCTLs and workflow tools for colorists; the chart stage belongs at the end for display QC. HB Color Separation remains a separate creative stage; its paid license and support belong to Henry Bobeck, not this repository.

## Film controls

`Film / Profile` selects Neutral, Latitude, Punch, or Chrome behavioral H&D families. These are family shapes, not commercial stock matches. `Film / Print M` is the master printer exposure in printer points, where one point is 0.025 log10 exposure. `Film / DIR` controls non-spatial inter-layer inhibitor behavior. `Color / Pos Sat` uses Primera Suite's positive HSV saturation behavior in LogC3-encoded AWG3; `Color / Dye` remains the independent density-domain dye-coupling control.

`Film / Strength` scales Film Response, DIR, master print exposure, C/M/Y filtration, subtractive color, and split/look density offsets. It does not change WB, scene exposure, contrast, shadows/highlights, rolloff, or output cleanup.

## Diagnostics

`View / Mode` provides Result, Neutral Chroma, Density, Gamut Stress, and Skin Mask views.

## Credits and upstream sources

Keystone combines original integration work with openly available color-science code and ideas. Upstream names are intentionally kept out of the DCTL implementation itself; attribution is explicit here and in `THIRD_PARTY_NOTICES.md`.

### Incorporated or directly adapted

- **Speak** — H&D negative/print curve model, behavioral profile families, printer-point convention, density-domain subtractive color, dye coupling, and density split-toning foundations. GitHub: https://github.com/amateurmenace/Speak
- **spektrafilm** by Andrea Volpato — DIR donor/receiver development model, DIR source defaults and pre-correction concept, and enlarger CC filtration convention. GitHub: https://github.com/andreavolpato/spektrafilm
- **Primera Suite** by Geoff Smith — black-point and rolling-contrast behavior retained from earlier Keystone development. GitHub: https://github.com/geoffsmithBK/primera-suite
- **Thatcher Freeman utility-dctls** — reference work used during earlier tone-scale and DCTL development. GitHub: https://github.com/thatcherfreeman/utility-dctls
- **ACES 1.3 Reference Gamut Compression** — optional input gamut-repair reference implementation. GitHub: https://github.com/ampas/aces-vwg-gamut-mapping-2020

### Reviewed or workflow references

- **Photographic DCTLs** by Mikael Sundell — previous film-matrix research reference; the fixed matrix remains removed. GitHub: https://github.com/mikaelsundell/photographic-dctls
- **Uffy PhotoChemical Look Process** — workflow/component reference only. GitHub: https://github.com/RichardUffy/Uffy-PhotoChemical-Look-Process-for-DaVinci-Resolve-Studio
- **Kodak2383_Emulation** — research reference only; no implementation copied. GitHub: https://github.com/lakravana/Kodak2383_Emulation
- **Dec. 18 Studios** — Film Negative Space workflow/reference influence during earlier development. GitHub: https://github.com/Dec18studios
- **HB Color Separation DCTL** — used as a separate downstream color-separation stage in the recommended Resolve workflow. It is not incorporated into Keystone.

Keystone is distributed under **GPL-3.0-only** because GPLv3-covered source is incorporated into the combined work. No spektrafilm stock profile/LUT database assets are bundled.

## Vibe-coded disclosure

**This entire Keystone project was vibe coded.** The DCTL, math translations, integration decisions, creative preset translations, refactors, validation scripts, documentation, packaging, and release automation were produced through iterative human-directed AI coding. Source files and licenses were reviewed and credited, but AI-generated code can still contain mistakes, mistranslations, edge cases, or host-specific issues. The project is provided without a warranty of correctness, fitness, or production safety; users remain responsible for validating it in their own Resolve environment and on their own material.

The connected workflow was tested on footage recorded in Apple Log and Canon Log 3. Those tests document the author's working setup; they are not a guarantee that every camera, CST configuration, GPU, or Resolve version will behave identically.

## Production status

v2.6.1 is code- and package-hardened: all exposed controls have hover help, new features default to identity, matrix code remains absent, filtration placement is explicit, numerical guards remain in place, and release metadata/validators cover the new modules. Resolve host/runtime testing is intentionally outside this preparation pass.

### Camera UV/IR note

Keystone intentionally does not expose camera UV/IR filters. The upstream operation changes wavelength-resolved film sensitivity before film exposure; a three-channel RGB DCTL cannot reproduce that faithfully. Use a spectral film simulator for this stage.


## v2.6.1 stability correction
v2.6.0 is withdrawn. v2.6.1 rebuilds the density-domain hardening from v2.5.1 with all helper functions declared before use and runtime clamps matched to their UI ranges.
