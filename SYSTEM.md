# Luma Color System

LookLab WB, Advanced Toner, PresenceOFX, Keystone, and LUTManagerOFX are separate tools with separate responsibilities. They are designed to work together without forcing every grade through every tool.

## The three tools

| Tool | Job | Typical placement |
|---|---|---|
| **Advanced Toner** | Environmental and narrative palette design | Before Keystone |
| **Keystone** | Technical balance, tone, color volume, gamut handling, and cleanup | Main grading hub |
| **LookLab WB** | Source/target white balance and tint | Before FilmMatrix / palette work |
| **PresenceOFX** | Lens/optical presence | Between palette work and Keystone |
| **LUTManagerOFX** | Folder-backed LUT browsing / look audition | Optional display-referred node after ODT |

## Recommended full stack

For the current AWG3 / LogC3 film-matrix workflow:

`Camera/CST -> LookLab WB -> FilmMatrix -> Advanced Toner -> PresenceOFX -> Keystone -> ODT -> LUTManagerOFX`

Resolve node tree:

1. Camera/CST or input transform
2. [LookLab WB](https://github.com/wbrisenold/LookLab-WB)
3. FilmMatrix or film-matrix prep
4. [Advanced Toner](https://github.com/wbrisenold/AdvancedToner)
5. [PresenceOFX](https://github.com/wbrisenold/PresenceOFX)
6. [Keystone](https://github.com/wbrisenold/Keystone)
7. ODT/display transform
8. [LUTManagerOFX](https://github.com/wbrisenold/LUTManagerOFX), optional for Rec.709/display LUT auditioning

Use only the tools the shot needs. A neutral technical shot may use Keystone only. A location-driven grade may use Advanced Toner + Keystone. A hero creative shot may use the full stack.

FilmMatrix credit: the FilmMatrix node refers to [`PD-LogC3-FilmMatrix.dctl`](https://github.com/mikaelsundell/photographic-dctls/blob/master/PD-LogC3-FilmMatrix.dctl) from Mikael Sundell's `photographic-dctls` repository. I did not make that DCTL; I only use it in this node tree.

## Design rules

- **Advanced Toner owns palette.**
- **Keystone owns balance and technical grade behavior.**
- Shot correction remains upstream of creative look design.
- LookLab WB, Advanced Toner, PresenceOFX, and Keystone stay upstream of the display transform.
- LUTManagerOFX usually goes after the display transform because most look LUTs expect Rec.709/display-referred input.
- Neutral/bypass behavior is a release requirement.

## Repository model

Each tool lives in its own GitHub repository, carries its own license, has independent releases, and includes this system document so the relationship between the tools remains clear.

Repositories: [Advanced Toner](https://github.com/wbrisenold/AdvancedToner), [LookLab WB](https://github.com/wbrisenold/LookLab-WB), [Keystone](https://github.com/wbrisenold/Keystone), [PresenceOFX](https://github.com/wbrisenold/PresenceOFX), [LUTManagerOFX](https://github.com/wbrisenold/LUTManagerOFX).
