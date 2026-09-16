# KeystoneOFX

KeystoneOFX is the OFX/Metal version of the current Keystone AWG4/LogC4 grading pipeline. It keeps the original user-supplied `Referent_LogC4_to_Rec709.cube` as the final output transform and adds a **real one-frame Auto Neutral lock** that a DCTL cannot persist.

## What changed from the DCTL

The grade pipeline remains:

`LogC4 decode -> input gamut repair -> Hoya ND -> WB -> exposure/tone -> Sat Split -> Density -> Pos Sat -> Genesis Interlayer -> locked Neutral -> Split Tone -> bleach/fade -> look -> skin -> Creative White -> safety -> LogC4 -> original Referent -> Bleach`

The important Auto Neutral difference is state. Pressing **Analyze Current Frame** fetches the frame under the playhead once, analyzes the signal after Keystone's pre-neutral color stages with the recovered analysis front-end, and stores three hidden persistent RGB gains in the OFX instance. Render does not analyze later frames. Those stored gains remain fixed until you analyze again or press **Reset Neutral**.

That placement also fixes the saturation issue from the earlier DCTL: Sat Split, Density, Pos Sat and Interlayer are included before neutral analysis and before the stored correction is applied, so increasing those controls does not simply restore the cast that was neutralized.

## Input / output contract

- **Input:** ARRI Wide Gamut 4 / LogC4
- **Output:** original Referent LogC4 -> Rec.709 / BT.1886, followed by Keystone's exact recovered Bleach when enabled
- Do not place another LogC4-to-display conversion after KeystoneOFX unless you intentionally want a second transform.

The bundled Referent cube SHA-256 is:

`19b2feb5ed8cb767d980e9f9b351b6e1823a3990974277fdb4a46d1f709d251c`

The build checks this hash before packaging.

## Auto Neutral workflow

1. Put the playhead on a representative frame for the shot.
2. Set the Keystone controls that occur before Neutral first: ND, manual WB, tone, curve, Sat Split, Density, Pos Sat and Interlayer.
3. Open **Auto Neutral** and press **Analyze Current Frame**.
4. The calculated correction is stored in hidden persistent OFX parameters and used for every frame in the clip/plugin instance.
5. Use **Amount** to reduce the stored correction without re-analyzing.
6. If you materially change an upstream control or want a different reference frame, press **Analyze Current Frame** again.
7. **Reset Neutral** returns the stored correction to identity.

There is no fake Live/Lock selector and no exposed Temp/Tint storage controls.

## UI

Resolve receives collapsible OFX groups:

- Auto Neutral
- Filter + White Balance
- Tone
- Color
- Split Tone
- Look
- Skin

Every continuous numeric control is defined as a bounded OFX `Double` with display minimum, display maximum, increment and digit precision so the host presents it as a slider. Actual enumerations remain dropdowns; Analyze/Reset are push buttons.

## Build without Xcode installed locally

You do **not** need Xcode on your Mac. Push this repository to GitHub and GitHub Actions builds the universal macOS bundle on a `macos-15` runner.

### Artifact build

Any push to `main`, pull request, or manual **Run workflow** executes:

1. Linux source/model tests.
2. macOS universal `arm64 + x86_64` CMake build.
3. Metal shader compilation with `xcrun metal` and `metallib`.
4. Bundle resource validation.
5. OFX export-symbol validation.
6. Runtime dependency check to reject accidental Homebrew library links.
7. Ad-hoc code signing.
8. Packaging as `KeystoneOFX-macOS-universal.zip`.

Download the ZIP from the `KeystoneOFX-macOS-universal` Actions artifact.

Pushing a tag such as `v1.0.0` also attaches the same ZIP to a GitHub Release.

## Install

Unzip the Actions artifact, then either copy `KeystoneOFX.ofx.bundle` to:

`/Library/OFX/Plugins/`

or run:

```bash
bash scripts/install_macos.sh /path/to/KeystoneOFX.ofx.bundle
```

Restart DaVinci Resolve after installation.

## Metal / CPU paths

On Resolve/macOS, Keystone advertises OFX Metal rendering and uses Resolve's Metal command queue and buffer handles. The `.metallib` is bundled under `Contents/Resources`, following the working PresenceOFX packaging pattern. A CPU implementation using the same DCTL-derived math and the same Referent cube is retained as a fallback and for model tests.

## Provenance

`reference/Keystone-v4_5_11-ReferentOnly.dctl` is included as the source reference used for this port. `generated/KeystoneConstants.inc` and `generated/KeystoneMath.inc` are extracted from that approved DCTL so the CPU and Metal paths do not silently substitute a new grading model.

See `VALIDATION.md` and `THIRD_PARTY_NOTICES.md` for current validation/provenance notes.

## License

GPL-3.0-only, matching the current Keystone DCTL source.

## v1.3 Auto Match
Auto Match now uses an adaptive Y'CbCr skin candidate cluster with spatial-coherence checks. A fixed skin-line hue is not used as the detector. When a credible coherent skin cluster exists, its measured skin-line error may steer the Keystone white-balance solution, while the scene-neutral estimate remains a safety prior. Without sufficient skin support, Auto Match falls back to the Keystone neutral estimators.

## v1.4 control layout
- Input / Filters: Hoya ND, UV Cut, IR Cut
- Auto Match: Analyze, Reset, Amount
- White Balance: Temp, Tint
- Tone: Exposure, Black Pt, Contrast, Shadows, Highlights, Roll, Curve
- Color: Density, Pos Sat, Interlayer, Sat Split
- Film / Finish: Hi Bleach, Lo Bleach, Bleach, Fade
- Split Tone
- Look
- Skin


## v1.4 Skin + UV/IR

The creative Skin section now follows a recovered Skin Tones control surface instead of Keystone's previous Primera Sat/Dense pair. Parameter labels, ranges, defaults and six preset names were recovered. Skin selection uses the recovered HSL hue/lightness Gaussian family already present in Keystone.

UV/IR filtering is placed in Input / Filters, before WB/exposure. UV/IR is wavelength-resolved; Keystone is not. The OFX therefore uses an erf cutoff shape/default edges projected onto a 610/550/450 nm RGB basis, with that limitation documented in source and UI.

## Native Match engine (v1.5)

Auto Match now runs a bundled match OFX directly. `Analyze Match` is the native push-button parameter, and the match OFX renders the image before Keystone's grading stages.
Use **Match Only = On** to bypass every Keystone stage after the match and compare the result directly.

The implementation intentionally does not translate the match into RGB gains or reproduce its internal hue optimizer. See `NATIVE_MATCH_AUDIT.md` for the integration boundary.
