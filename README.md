# Keystone

Keystone is the technical grading hub in the Luma Color System. It handles primary balance, white balance, exposure/tone, color volume, gamut/neutral cleanup, and technical finishing. It is designed to stay pre-ODT.

## Source

- Version: `1.0-RC24`
- DCTL: `Keystone-v1.0-RC24-Metal-Collision-Fix.dctl`
- Input: selected inside Keystone
- Output: selected inside Keystone; remains pre-ODT
- License: GPL-3.0-or-later
- SHA-256: `66daa1c3c0dc1e58ff99d896a7064baf91215d4ca172b67259000d5c6225fa04`

Keystone contains an exact-match ME_Desatch-compatible module and therefore keeps GPL-3.0-or-later distribution terms.

## Placement

Use Keystone after WB/input prep, FilmMatrix, palette work, and optical/lens character. It should usually be the last technical DCTL before the ODT.

Recommended Resolve node tree:

1. Camera/CST or input transform
2. [LookLab WB](https://github.com/wbrisenold/LookLab-WB), if source/target white balance or tint correction is needed
3. FilmMatrix or film-matrix prep
4. [Advanced Toner](https://github.com/wbrisenold/AdvancedToner) for palette, environment, and mood
5. [PresenceOFX](https://github.com/wbrisenold/PresenceOFX) for lens/optical presence
6. Keystone for primary balance, tone, color volume, gamut handling, and cleanup
7. ODT/display transform
8. [LUTManagerOFX](https://github.com/wbrisenold/LUTManagerOFX), optional, for display-referred LUT browsing after the ODT

In this published set, the only LookLab repo is `LookLab-WB`, and it is white-balance only.

FilmMatrix credit: the PD FilmMatrix node in this tree refers to [`PD-LogC3-FilmMatrix.dctl`](https://github.com/mikaelsundell/photographic-dctls/blob/master/PD-LogC3-FilmMatrix.dctl) from Mikael Sundell's `photographic-dctls` repository. I did not make that DCTL. I only use it as a separate node in my Resolve node tree.

## Related Repositories

- [LookLab WB](https://github.com/wbrisenold/LookLab-WB): source/target white balance and tint correction
- [Advanced Toner](https://github.com/wbrisenold/AdvancedToner): scene-referred palette and environment toning
- [PD-LogC3-FilmMatrix.dctl](https://github.com/mikaelsundell/photographic-dctls/blob/master/PD-LogC3-FilmMatrix.dctl): third-party FilmMatrix node used in the tree; not made by me
- [PresenceOFX](https://github.com/wbrisenold/PresenceOFX): lens/optical presence OFX
- [LUTManagerOFX](https://github.com/wbrisenold/LUTManagerOFX): folder-backed LUT browsing OFX, usually after ODT for Rec.709/display LUTs

## Install on macOS

Run `scripts/INSTALL.command`. It installs the DCTL into Resolve's user LUT/DCTL folder under:

`Luma Color System/Keystone`

Refresh Resolve's LUT list or restart Resolve after installing.

## Validation and Releases

Run static validation with:

```bash
python3 ci/validate_dctl.py
```

Every push and pull request runs the validator. A `v*` tag creates a release ZIP automatically.

## Disclaimer

This tool was vibe coded with AI assistance. Treat it as an experimental grading tool, not a color-science reference implementation. Validate it on your footage, scopes, and delivery path before using it on paid or archival work.

Creative defaults are intended to be mathematically neutral. Keystone is not an ODT and does not include a hidden display look.
