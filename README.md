# Keystone

**Part of the Luma Color System.**

Primary balance, white balance, exposure/tone, color volume, gamut/neutral cleanup, and technical finishing.

## Current source

- Version: **1.0-RC24**
- DCTL: `Keystone-v1.0-RC24-Metal-Collision-Fix.dctl`
- License: **GPL-3.0-or-later**
- SHA-256: `66daa1c3c0dc1e58ff99d896a7064baf91215d4ca172b67259000d5c6225fa04`

## Pipeline role

**Placement:** After optical/film-matrix preparation and before final creative look/ODT.

**Input:** Multiple supported camera/grading spaces selected inside Keystone.

**Output:** Returns to the selected grading space; designed to remain pre-ODT.

Keystone contains an exact-match ME_Desatch-compatible module and therefore retains GPL-3.0-or-later distribution terms.

## Luma Color System

Current full-stack reference:

`Camera/CST -> FilmMatrix -> Advanced Toner -> Lens / optical stage -> Keystone -> LookLab -> ODT`

The tools stay separate on purpose:

- **Advanced Toner** = palette and environment
- **Keystone** = primary balance and technical grade
- **LookLab** = final creative grade

Companion OFX tools in the same system: **PresenceOFX** provides the lens/optical presence stage, and **LUTManagerOFX** provides folder-backed LUT browsing for look management.

See [`SYSTEM.md`](SYSTEM.md).

## Install on macOS

Run `scripts/INSTALL.command`. It installs the DCTL into:

`Luma Color System/Keystone`

inside Resolve's user LUT/DCTL folder.

## GitHub workflow

Every push and pull request runs static DCTL validation. A `v*` tag creates a downloadable release ZIP automatically.

The validator includes an explicit UI/combo-symbol vs `__DEVICE__` function collision audit to catch the Metal namespace failure that previously affected Keystone.

## Funding

`.github/FUNDING.yml` is included as part of the standard repository template.
