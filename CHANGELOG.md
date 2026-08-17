# Changelog

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
