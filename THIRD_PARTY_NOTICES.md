# Third-party / source provenance

KeystoneOFX is a port of the user's current Keystone DCTL and retains its existing source/provenance obligations.

Key components represented in the current Keystone math include:

- ARRI Wide Gamut 4 / LogC4 matrices and transfer-function constants.
- OpenDRT Creative White CAT02-related math already present in Keystone.
- ACES reference gamut-compression related input repair already present in Keystone.
- Source-derived / recovered Contour density, split-saturation and split-toning behavior already present in Keystone.
- Source-derived / recovered Genesis 5207 interlayer polynomial term already present in Keystone.
- Primera-derived tone/skin operations already present in Keystone.
- ToneLab-derived Fade and Bleach behavior already present in Keystone.
- The exact user-supplied `Referent_LogC4_to_Rec709.cube`, bundled as a resource without resampling.

This repository does not bundle Contour or Genesis OFX binaries, their LUT export paths, stock D-LogE tables, halation, grain, overlays or watermark code.

See the reference DCTL comments and the user's upstream project documentation for detailed attribution applicable to the math carried forward here.


## Scene Grade semantic analysis

The Scene Grade region-selection and scene-decision logic is adapted from the GPL-3.0 OneGrade project by Matt Grdinic. Keystone is already GPL-3.0-only, so the adapted source remains under compatible GPL terms.

The bundled ADE20K PP-MobileSeg model is distributed under Apache-2.0 terms; a copy is included at `vendor/licenses/ADE20K-model-Apache-2.0.txt`.

The optional inference runtime is ncnn, pinned in CMake to commit `5e66f094bf7c597b4569cc014a8be84104748678`; its BSD-3-Clause license is included at `vendor/licenses/ncnn-BSD-3-Clause.txt`.
