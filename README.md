# KeystoneOFX

KeystoneOFX is a self-contained OFX/Metal grading plugin for DaVinci Resolve. It expects ARRI Wide Gamut 4 / LogC4 input and includes its own output transform to Rec.709 / BT.1886.

## Pipeline

`LogC4 decode -> input filtering -> Auto Match -> white balance -> tone -> color -> split tone -> finish -> look -> skin -> output transform`

All processing is presented as Keystone functionality. Auto Match analyzes the current frame on demand and stores the correction in the OFX instance; it does not continuously re-analyze later frames.

## Controls

- **Input / Filters:** ND, UV Cut, IR Cut
- **Auto Match:** Analyze Match, Enable Match, Disable Match, Match Only
- **White Balance:** Temp, Tint
- **Tone:** Exposure, Black Pt, Contrast, Shadows, Highlights, Roll, Curve
- **Color:** Density, Pos Sat, Interlayer, Sat Split
- **Film / Finish:** Hi Bleach, Lo Bleach, Bleach, Fade
- **Split Tone:** Amount, shadow/highlight hue, balance, subtractive amount
- **Look:** Color, Amount, Creative White
- **Skin:** Enable, Preset, Saturate Skin, Skin Luminance, Skin Tone, selection controls, mask preview, Intensity

**Match Only** outputs the Auto Match result without Keystone's later creative grading stages. It is useful for checking the automatic correction by itself.

## Input / output

- Input: ARRI Wide Gamut 4 / LogC4
- Output: Rec.709 / BT.1886
- Do not add another LogC4-to-display conversion after Keystone unless a second transform is intentional.

## Build

The repository includes `.github/workflows/build.yml`. A push to `main`, a pull request, a manual workflow run, or a version tag runs source checks, model tests, a universal macOS build, Metal compilation, bundle validation, signing, and packaging.

The build artifact is `KeystoneOFX-macOS-universal.zip`.

## Install

Copy `KeystoneOFX.ofx.bundle` to:

```text
/Library/OFX/Plugins/
```

Then restart DaVinci Resolve. The included `scripts/install_macos.sh` can also install a built bundle.

## Development

The CPU implementation is retained for model tests and fallback validation. The macOS Resolve path uses the bundled Metal kernels where supported. `reference/Keystone-reference.dctl`, `generated/KeystoneConstants.inc`, and `generated/KeystoneMath.inc` are the retained Keystone reference implementation used to keep CPU and GPU behavior aligned.

See `VALIDATION.md` for validation details. Required third-party licensing notices, where applicable, remain isolated in `THIRD_PARTY_NOTICES.md` and are not part of Keystone's product/UI documentation.

## License

GPL-3.0-only.
