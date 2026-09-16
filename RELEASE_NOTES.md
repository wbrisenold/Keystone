# KeystoneOFX v1.5.2

- Removed external plugin/product branding from Keystone README, UI text, source comments, source identifiers, filenames, build scripts, and validation scripts.
- Renamed the internal match bridge and bundled resource paths to Keystone-native names.
- Keystone instance state remains separate from the embedded match engine instance state.
- Preserved required private engine parameter identifiers through runtime-generated IDs so the match engine continues to function without exposing its implementation branding in Keystone source.
- Retained required third-party legal notices separately in `THIRD_PARTY_NOTICES.md`.

# Release notes

## v1.5.2 — loader fix
- Native Native Match bridge now uses lazy symbol binding and an explicit Contents/Resources path derived from Keystone's loaded Mach-O.
- Keystone no longer propagates a nested Native Match Load/Describe error back to Resolve, preventing the wrapper from being rejected during OFX discovery.
- Corrected bundle metadata to v1.5.2.
- macOS validation now checks the nested Native Match Mach-O, OFX exports, and deep code signature.

# Keystone v1.1

- Auto Match now stores and applies the recovered Native Match AdjustLevel affine match directly instead of converting the analysis into Keystone RGB neutral gains.
- Removed the post-analysis Keystone skin guard from the active match path.
- Keystone creative grading remains downstream of Auto Match.
- Resolve plug-in label shortened to `Keystone v1.1`.

## v1.2 — Skin-first Keystone Auto Neutral
- Removed the Native Match affine match from Auto Neutral and returned to Keystone-native RGB gain correction.
- Skin is now an active white-balance reference rather than only a protection/veto signal.
- Credible Keystone-derived skin samples are evaluated across a dense image lattice; highlights, deep shadows, near-neutral pixels, and extreme saturation are gated out.
- The solver searches chromatic red/blue gain around a green anchor to minimize skin-line hue error, with the scene-neutral estimate retained as a soft sanity constraint.
- When reliable skin is absent, Auto falls back to Keystone's Gray World / Shades-of-Gray / Gray Edge / near-neutral estimator.
- Stored Native Match affine parameters are reset to identity by analysis; rendering uses Keystone neutral gains again.
- Resolve label remains short: Keystone v1.2.

# Keystone v1.3

- Replaced the v1.2 Keystone-hue skin classifier with an image-adaptive BT.709 Y'CbCr chroma-cluster detector.
- Skin-line hue is no longer used to decide what is skin; it is only the correction destination after a coherent skin cluster is established.
- Added iterative trimmed chroma clustering, luminance/chroma rejection, spatial cell coherence, minimum support, a tighter chromatic search, and a neutral-estimate safety prior.
- Reorganized Resolve controls into: Input / Filter, Auto Match, White Balance, Tone, Color, Film / Finish, Split Tone, Look, Skin.
- ND is now in Input / Filter; Temp/Tint are isolated under White Balance.
- Keystone-derived Bleach/Fade and highlight/shadow bleach are grouped under Film / Finish.
- Plugin label: Keystone v1.3.


# Keystone v1.4

- Replaced the old two-slider Keystone creative Skin block with Keystone's recovered Skin Tones control layout and ranges.
- Added Keystone Skin preset selector names: Custom, Vibrant, Muted, Cool / Neutralize Red, Warm / Add Warmth, Matte / Reduce Shine.
- Added Saturate Skin, Skin Luminance, Skin Tone, Tone Center, Tone Range, Brightness Center, Brightness Range, Show Mask, and Skin Intensity.
- Keystone parameter names/ranges/defaults are recovered from the supplied Keystone.ofx binary. The current preset recipes are conservative Keystone translations of the recovered preset intent; they are not claimed bit-identical until Keystone's host-side preset setter table is fully recovered.
- Added Keystone-style UV Cut and IR Cut controls under Input / Filters, before white balance and exposure. The source erf cutoff law/default edges are retained; because Keystone is RGB rather than spectral, this is an explicit 610/550/450 nm RGB projection, not spectral equivalence.
- Renamed Input / Filter to Input / Filters.
- Resolve plug-in title: Keystone v1.4.

## v1.5 — Native Native Match Fix engine

- Removed Keystone skin-first/adaptive Auto Match from the correction path.
- Bundles the user's known-working Native Match v4-clean OFX and delegates Analyze Match to its native `native_match_fix` action.
- Delegates render to the Native Match OFX before Keystone grading; no reconstructed Native Match histogram/hue/gain approximation is used.
- Keeps Keystone instance storage separate from Native Match's OFX instance storage.
- Adds **Native Match Only** diagnostic mode. When enabled, the plugin returns the native Native Match render before any Keystone processing.
- Native Match license/activation controls remain hidden; the bundled v4-clean binary is the prior working no-license build.
