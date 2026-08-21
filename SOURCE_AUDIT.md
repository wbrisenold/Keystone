# Keystone v2.5.1 Source Audit

## spektrafilm enlarger C/M/Y filtration

Source: `src/spektrafilm/model/color_filters.py` and `src/spektrafilm/runtime/params_schema.py`.

The source states that Kodak CC values are proportional to optical density and converts CC to transmittance as `10 ** -(CC/100)`. Keystone maps C/M/Y shifts into the print-exposure gap as channel density offsets with the same `CC/100` relationship.

## Advanced Toning presets

Source reviewed: `AdvancedToning-v1.0.dctl` supplied in the user's file library. Its preset table defines shadow/midtone/highlight hues and per-zone amounts for Warm Cool, Cool Warm, Amber Cyan, Cyan Amber, Olive Cream, Teal Orange, Warm Vintage, Cool Silver, Sodium Cyan, and Moonlight Warm Skin.

Keystone v2.4 preserves the shadow/highlight look intent but does not copy the old Oklab three-zone runtime. Presets are translated into fixed density vectors so they use Keystone's current split-tone engine and neutral-pivot behavior.

## Existing photochemical sources

- Speak: https://github.com/amateurmenace/Speak
- spektrafilm: https://github.com/andreavolpato/spektrafilm
- Primera Suite: https://github.com/geoffsmithBK/primera-suite
- Thatcher Freeman utility-dctls: https://github.com/thatcherfreeman/utility-dctls
- ACES gamut mapping reference: https://github.com/ampas/aces-vwg-gamut-mapping-2020

See `THIRD_PARTY_NOTICES.md` for licensing notices.

## Camera UV/IR exclusion

The upstream UV/IR filtration is spectral and modifies wavelength-resolved film sensitivity before RGB-to-film-raw conversion. Keystone v2.4.2 intentionally excludes an RGB proxy rather than presenting non-equivalent controls.

## Primera Pos Sat — v2.5.1

Source: https://github.com/geoffsmithBK/primera-suite/blob/5ad163a29af60fd82eb94727c96dab666b1176e5/src/Primera/body.dctlc

Keystone v2.5.1 removes its previous density subtractive-saturation operator. The replacement follows Primera v0.6.0 Pos Sat: operate on encoded RGB, convert to HSV, multiply saturation by `pow(2, amount)`, clamp saturation to 1.0, convert back to RGB. Keystone's working space is fixed to AWG3/LogC3, so it encodes/decode around this operator. Primera's optional luma-preservation toggle defaults off; Keystone retains that default behavior. Dye coupling remains a separate Keystone photochemical stage and is not part of Primera Pos Sat.
