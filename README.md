# Keystone

Keystone is a DaVinci Resolve DCTL for grading in **ARRI Wide Gamut 3 / LogC3 EI800**. It is designed as a compact balance, tone, color, and skin-shaping node that sits **before the output transform**.

Current version: **v1.9.4**

## Node placement

Recommended order:

```text
Camera normalization -> Look / Film Matrix -> Advanced Toning -> Keystone -> ODT -> optional display look
```

Keystone expects **AWG3 / LogC3 EI800 in and out**. It does not contain a display transform.

## Controls

### Input
- `Input / Fix` — optional ACES 1.3 reference-gamut-compression repair

### White balance
- `WB / Hue` — direction around the HDR-style white-balance wheel
- `WB / Amt` — strength of the white-balance correction

White balance is not implemented as RGB offsets. Keystone constructs a source-white displacement around D65 and applies a Bradford chromatic adaptation.

### Negative
- `Neg / R`
- `Neg / G`
- `Neg / B`

These are printer-light controls inside Keystone's reversible film-negative working model.

### Tone
- `Tone / Exp`
- `Tone / Black`
- `Tone / Contrast`
- `Tone / Shadows`
- `Tone / Highlights`
- `Tone / Roll`

Contrast is applied to scene luminance using the Daniele/Siragusano tone-shape architecture rather than independently reshaping RGB channels.

### Color
- `Color / Sat`

Saturation uses an Oklab hue-preserving chroma model with bounded wide-gamut expansion.

### Skin
- `Skin / Range`
- `Skin / Hue`
- `Skin / Sat`
- `Skin / Dense`
- `Skin / Even`
- `Skin / Sep`

The skin qualifier combines normalized chromaticity, perceptual hue/chroma qualification, and scene-luminance gating. It is color-based, not semantic face detection.

### Split tone
- `Split / Sh Hue`
- `Split / Sh Amt`
- `Split / Hi Hue`
- `Split / Hi Amt`

### Output cleanup
- `Out / White`
- `Out / Black`

These controls reduce chroma only in near-neutral highlights or shadows while preserving scene luminance.

## Installation

Copy `Keystone.dctl` into your DaVinci Resolve LUT/DCTL folder, then refresh LUTs or restart Resolve.

Common locations:

### macOS

```text
~/Library/Application Support/Blackmagic Design/DaVinci Resolve/Support/LUT/
```

### Windows

```text
%AppData%\Blackmagic Design\DaVinci Resolve\Support\LUT\
```

### Linux

```text
~/.local/share/DaVinciResolve/LUT/
```

You can then load Keystone through Resolve's DCTL OFX or apply it from the LUT/DCTL list depending on your workflow.

## Input requirement

Normalize camera footage to **ARRI Wide Gamut 3 / LogC3 EI800** before Keystone.

Example:

```text
Apple Log -> CST to AWG3/LogC3 -> Keystone -> ODT
```

Do not put Keystone after a Rec.709 ODT.

## Validation

This repository includes a lightweight validator:

```bash
python3 tools/validate.py Keystone.dctl
```

It checks for common Resolve DCTL failure points including:

- malformed or multiline UI macros
- missing or duplicate `transform()`
- transform-signature drift
- unbalanced braces and parentheses
- parser-sensitive UI labels
- `while` loops
- unresolved symbolic `PI`
- unused `__DEVICE__` helper functions

GitHub Actions runs the same validation on pushes and pull requests.

**Passing these checks does not prove Resolve/Metal runtime compatibility. The final test is loading the exact DCTL in DaVinci Resolve.**

## Releases

The repository automatically creates a new GitHub Release when the version changes.

Versioning is controlled by:

```text
VERSION
```

and the first line of `Keystone.dctl`:

```text
// Keystone v1.9.4
```

Those two values must match.

When a commit reaches `main`, the release workflow:

1. validates `Keystone.dctl`
2. reads the version
3. skips publishing if that version already has a GitHub Release
4. creates tag `vX.Y.Z`
5. packages the repository as `Keystone-vX.Y.Z.zip`
6. attaches both the ZIP and `Keystone-vX.Y.Z.dctl`
7. publishes the release as the latest GitHub Release

To publish the next version, update both `VERSION` and the DCTL header, then push to `main`.

You can also run **Release Keystone** manually from the Actions tab.

## License

Keystone is distributed under the MIT License. Third-party code and algorithm notices are documented in `THIRD_PARTY_NOTICES.md`.
