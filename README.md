# Keystone

Keystone is a DaVinci Resolve DCTL for grading in **ARRI Wide Gamut 3 / LogC3 EI800**. It is designed as a compact balance, tone, color, and skin-shaping node that sits **before the output transform**.

Current version: **v1.13.1**

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
- `WB / Temp K` — 2500K to 20000K, D65 default 6504K
- `WB / Tint` — -100 to +100

White balance is the **exact implementation from Keystone v1.7 Studio Production Candidate**: CCT is converted to a source-white locus, Tint offsets that white in uv space, then one Bradford chromatic adaptation returns the source white to D65. A scalar normalization keeps D65-neutral scene luminance unchanged.

### Negative
- `Neg / R`
- `Neg / G`
- `Neg / B`

These are printer-light controls inside Keystone's reversible film-negative working model.

### Tone
- `Tone / Exp`
- `Tone / Black Pt`
- `Tone / Contrast`

`Tone / Black Pt` now uses Primera's exact Black Point behavior: `-0.05` to `+0.05`, step `0.0001`, with the fixed `0.005` soft knee. It runs independently on scene-linear AWG3 R/G/B immediately after Exposure.

- `Tone / Shadows`
- `Tone / Highlights`
- `Tone / Roll`

Contrast now uses **Primera's rolling contrast equation** independently on AWG3 / LogC3 RGB channels, at Primera's default Pivot of `0.0` (encoded 18% gray). This is intentionally per-channel: the contrast increase also creates the saturation/color-separation behavior of Primera. No separate contrast-saturation coupling is added.

### Color
- `Color / Pos Sat`
- `Color / Oklab Pos`

`Color / Pos Sat` is the Primera-style encoded-domain HSV saturation control:
1. encode current AWG3 linear RGB to LogC3
2. convert LogC3 RGB to HSV
3. multiply HSV saturation by `2^amount`
4. cap HSV saturation at `1.0`
5. convert back to RGB and decode LogC3

`Color / Oklab Pos` is a second, separate positive-only saturation control for direct comparison:
1. convert current scene XYZ to Oklab
2. keep Oklab `L` fixed
3. multiply Oklab chroma by `2^amount`
4. keep hue angle fixed
5. convert back to XYZ and rescale to preserve original scene `Y`
6. apply a **small density coupling** based on the achieved chroma increase

That density coupling is intentionally modest. Neutrals stay mostly untouched, while more colorful pixels get a slightly richer, denser feel so the Oklab slider is not as clinically clean as a pure lightness-preserving saturation operator.

Both sliders are `0` to `1`, where `0` is exact bypass. For a clean comparison, keep one of them at `0`.


### Output cleanup
- `Out / White`
- `Out / Black`

These controls reduce chroma only in near-neutral highlights or shadows while preserving scene luminance.

They retain **automatic skin protection**, but there are no user-facing Skin controls. A small internal fixed skin qualifier is used only to keep Output cleanup from pulling useful color out of faces.

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
// Keystone v1.13.1
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
