# Luma Color System

LookLab WB, Advanced Toner, PresenceOFX, Keystone, and downstream finishing tools have separate jobs.

## Current pipeline

`Camera/CST -> LookLab WB -> FilmMatrix -> Advanced Toner -> Keystone -> PresenceOFX -> scene-referred finishing -> ODT -> display LUT -> diagnostics`

## Responsibilities

- **LookLab WB**: source/target WB, tint, creative white point.
- **FilmMatrix**: film-style matrix prep in its documented LogC3 workflow.
- **Advanced Toner**: palette/environment/mood in AWG3 / LogC3.
- **Keystone RC27**: primary balance, permanent internal negative-response tone development, printer lights, Chroma/Vibrance/Hue, skin, selective ME_Desatch controls, output cleanup and safety.
- **PresenceOFX**: spatial/lens/optical presence.
- **ODT/display LUT**: display rendering and final display-referred look.

## Keystone RC27 design rules

- Input repair, WB, and native channel balance stay outside the internal negative-response stage.
- Printer lights and all tone controls run inside the negative-response stage.
- Chroma, Vibrance, Hue, Skin, and ME_Desatch run after Negative Space EXIT.
- ME_Desatch is evaluated in the selected native encoded working transfer, then decoded immediately back to scene-linear before output cleanup.
- Output cleanup and catastrophic safety remain downstream of creative color.
- Neutral/bypass identity is a release requirement.
- Keystone's internal negative stage affects Keystone only; downstream nodes remain in the normal selected source space.
