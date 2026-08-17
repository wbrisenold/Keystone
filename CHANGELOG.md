# Changelog

## 1.0-RC28

- Removed user-facing Negative Mid/Below/Above response controls; Film Negative Space response is now fixed internal technical plumbing.
- Removed native `Balance / R/G/B`; negative printer lights are Keystone's exposed density-style RGB balance.
- Removed manual `Tone / Pivot`; Contrast now derives its anchor from the selected input transfer/EI and 18% scene gray.
- Moved global Exposure to a true scene-linear `2^stops` operation before negative development.
- Replaced broad split-contrast Shadows/Highlights with automatic scene-stop exposure zones that leave middle gray alone.
- Replaced channelwise Roll Off with an automatic-knee, luminance-preserving monotonic scene-linear shoulder.
- Added destination/native-gamut-aware positive Chroma limiting and Hue-rotation protection.
- Renamed `Skin / Evenness` to `Skin / Hue Uniformity` to match its actual hue-convergence behavior.
- Retained all seven ME_Desatch controls in the selected native working-transfer stage.
- Replaced `Output / Negatives` with always-on technical negative/encode safety.
- Removed manual `Output / Skin Protect`; cleanup skin protection is automatic.
- Kept `Output / White Clean` and `Output / Black Clean` as manual creative cleanup sliders.
- Expanded behavioral CI for auto pivot, true Exposure, tonal isolation, monotonic Roll Off, gamut-aware color, Hue Uniformity, White/Black Clean, and 2.7 million randomized transforms.

## 1.0-RC27

- Made Keystone's internal Film Negative Space tone-development architecture permanent and removed the user-facing Off/Internal mode.
- Fixed printer lights: R/G/B are now a real grade inside the negative sandwich and are no longer canceled by the inverse transform.
- Fixed the neutral fast path so printer-light moves cannot be skipped.
- Fixed the tone architecture: Exposure, Black, Contrast, Pivot, Shadows, Highlights and Roll Off now operate directly in negative-response values instead of re-encoding that novel space through the selected camera/log transfer.
- Moved Chroma, Vibrance, Hue and Skin outside Negative Space, back into the normal scene-referred color domain.
- Removed `Color / Mid Chroma` to eliminate overlapping general-saturation behavior.
- Retained all seven ME_Desatch controls. Moved the exact module into a controlled native-working-transfer encode/apply/decode substage after Negative Space EXIT and before output cleanup.
- Added a final catastrophic-only scene ceiling before output encode for extreme legal multi-control combinations.
- Added behavioral gates for live printer lights, color/FNS separation, exact standalone ME_Desatch parity, negative-response monotonicity and 2.7 million randomized transforms.
- Retained RC25 monotonic Highlights and transfer-aware catastrophic-negative safety.

## 1.0-RC26

- Added optional `Negative Space / Mode = Internal Sandwich`.
- Added reversible scene-linear middle-gray remap with independent `Below` / `Above` powers and optional smooth pivot blend.
- Added per-channel printer-light-style working-space density offsets (`Printer R/G/B`, neutral 25).
- Routes Keystone creative tone/color/skin operations inside the negative working space while leaving technical input normalization and output safety outside.
- Internal Forward/Inverse cancels at neutral; enabling Negative Space by itself remains an exact image bypass.
- Added a catastrophic-only `+/-8192` scene-domain ceiling after negative inversion to prevent shallow legal inverse powers from overflowing under extreme grades while preserving RGB ratios.
- Verified the Negative Space Off path bit-for-bit against RC25 over 20,000 randomized non-neutral grades; added RC25 golden vectors to behavioral CI.
- Added Film Negative Space forward/inverse roundtrip and monotonicity tests plus 45,000 blended-negative stress evaluations.
- Retained RC25 highlight continuity and encoded-negative safeguards unchanged.

## 1.0-RC25

- Replaced the discontinuous Primera-style `Tone / Highlights` function with a C1-continuous, strictly monotonic Keystone implementation while keeping the existing `-1` to `+1` stop UI range.
- Added transfer-aware catastrophic-negative protection before final encode. Positive-Y cases preserve scene Y while reducing only chroma; imaginary/negative-Y cases use ratio-preserving scaling toward black.
- Added a final encoded-domain catastrophic-negative guard after ME_Desatch so encoded finishing cannot bypass the safety floor.
- Preserved exact neutral/bypass behavior.
- Added behavioral CI covering gamut/transfer round trips, neutral identity, highlight monotonicity, encoded safety, and 2.7 million deterministic randomized transforms across all 18 input spaces.
- Added complete third-party provenance/notices.
- Tagged release packages now include install/uninstall scripts, changelog, validation documentation, and third-party notices.
- Installer removes superseded `Keystone*.dctl` files from Keystone's own Resolve install folder before copying RC25, preventing duplicate RC entries after upgrade.

## 1.0-RC24

- Initial standalone GitHub repository packaging for the Luma Color System.
- Added funding, validation CI, tagged release packaging, system docs, and macOS install/uninstall scripts.
- Removed the optional Color Volume controls and their runtime path from Keystone.
- Removed the creative `White / Point` dropdown from Keystone; that behavior moved to LookLab WB.
