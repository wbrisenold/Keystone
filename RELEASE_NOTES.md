# KeystoneOFX Release Notes

## v1.5 — Native Match Engine
- Bundles a known-working match OFX and delegates Analyze Match to its native action.
- Delegates render to the match OFX before Keystone grading; no reconstructed match approximation is used.
- Keeps Keystone instance storage separate from the match OFX instance storage.
- Adds **Match Only** diagnostic mode. When enabled, the plugin returns the native match render before any Keystone processing.
- Match license/activation controls remain hidden; the bundled binary is the prior working build.

## v1.4
- Auto Match stores and applies the recovered affine match directly instead of converting the analysis into Keystone RGB neutral gains.
- Removed the affine match from Auto Neutral and returned to Keystone-native RGB gain correction.
- Stored affine parameters are reset to identity by analysis; rendering uses Keystone neutral gains again.
- Bleach/Fade and highlight/shadow bleach are grouped under Film / Finish.
- Replaced the old two-slider Primera creative Skin block with a recovered Skin Tones control layout and ranges.
- Added Skin preset selector names: Custom, Vibrant, Muted, Cool / Neutralize Red, Warm / Add Warmth, Matte / Reduce Shine.
- Parameter names/ranges/defaults are recovered. The current preset recipes are conservative Keystone translations of the recovered preset intent; they are not claimed bit-identical until the host-side preset setter table is fully recovered.

## v1.3
- Auto Match uses adaptive Y'CbCr skin candidate cluster with spatial-coherence checks.
- Fixed skin-line hue is not used as the detector.
- When a credible coherent skin cluster exists, its measured skin-line error may steer the Keystone white-balance solution, while the scene-neutral estimate remains a safety prior.
- Without sufficient skin support, Auto Match falls back to the Keystone neutral estimators.
