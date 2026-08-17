# Luma Color System

LookLab WB, Advanced Toner, PresenceOFX, Keystone, and LUTManagerOFX are separate tools with separate responsibilities.

## Core responsibilities

| Tool | Job | Typical placement |
|---|---|---|
| **LookLab WB** | Source/target WB, tint, creative white point | Before FilmMatrix |
| **Advanced Toner** | Environmental and narrative palette design | In its documented AWG3 / LogC3 space, before Keystone |
| **Keystone** | Technical balance plus creative tone/color/skin, optional internal negative working space, output safety | Main grading hub |
| **PresenceOFX** | Spatial/lens/optical presence | After palette/technical shaping, pre-ODT |
| **LUTManagerOFX** | Folder-backed look audition | Usually after ODT for display LUTs |

## Current AWG3 / LogC3 pipeline

`Camera/CST -> LookLab WB -> FilmMatrix -> Advanced Toner -> Keystone -> PresenceOFX -> scene-referred finishing -> ODT -> display LUT -> diagnostics`

Keystone RC26 can internally wrap its own creative controls in a reversible negative-style working space. That internal sandwich does not put downstream nodes such as PresenceOFX into negative space. Use a separate external Forward/Invert sandwich only when external nodes must also react in that novel space.

## Design rules

- LookLab WB owns source/target WB, tint, and creative white-point selection.
- Advanced Toner owns palette and expects its documented working space.
- Keystone owns primary technical balance, tone/color/skin grading, optional internal Negative Space, cleanup, and encode safety.
- Input repair/WB stays outside Keystone's internal Negative Space.
- Keystone creative tone/color/skin runs inside Negative Space when enabled.
- Output cleanup and delivery safety stay outside the internal Negative Space.
- ODT and display LUTs remain downstream.
- Neutral/bypass behavior is a release requirement.

FilmMatrix in this workflow refers to [`PD-LogC3-FilmMatrix.dctl`](https://github.com/mikaelsundell/photographic-dctls/blob/master/PD-LogC3-FilmMatrix.dctl) by Mikael Sundell. It is a separate third-party node and is not distributed with Keystone.
