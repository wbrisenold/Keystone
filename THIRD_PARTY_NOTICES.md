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
