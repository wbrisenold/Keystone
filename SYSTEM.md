# Luma Color System

## Current pipeline

`Camera/CST -> LookLab WB -> FilmMatrix -> Advanced Toner -> Keystone -> PresenceOFX -> scene-referred finishing -> ODT -> display LUT -> diagnostics`

## Responsibilities

- **LookLab WB**: source/target WB, tint, creative white point.
- **FilmMatrix**: film-style matrix prep in its documented LogC3 workflow.
- **Advanced Toner**: palette/environment/mood in AWG3 / LogC3.
- **Keystone RC28**: true scene Exposure, negative-development printer lights, automatic-pivot tone controls, gamut-aware Chroma/Vibrance/Hue, skin, selective ME_Desatch, White/Black Clean, and technical output safety.
- **PresenceOFX**: spatial/lens/optical presence.
- **ODT/display LUT**: display rendering and final display-referred look.

## Keystone RC28 design rules

- Technical WB stays before negative development.
- Global Exposure is true scene-linear stops before negative development.
- Film Negative Space response parameters are internal, fixed, reversible, and not user-facing grading controls.
- Printer lights are the only exposed negative-space RGB balance controls.
- Contrast uses an automatic selected-space/EI-aware 18% gray anchor; no manual Pivot.
- Shadows and Highlights use automatic scene-stop zones and do not act as split contrast around middle gray.
- Roll Off is a monotonic luminance shoulder with automatic knee placement.
- Chroma, Vibrance, Hue, and Skin operate after Negative Space EXIT and are automatically constrained to the selected/native gamut.
- ME_Desatch is evaluated in the selected native encoded working transfer, then decoded immediately back to scene-linear.
- White Clean and Black Clean are manual; skin protection during cleanup is automatic.
- Catastrophic-negative, overflow, and final encode safety are always on.
- Neutral/bypass identity is a release requirement.
