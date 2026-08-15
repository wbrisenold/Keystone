# Luma Color System

Keystone, LookLab, and Advanced Toner are separate tools with separate responsibilities. They are designed to work together without forcing every grade through every tool.

## The three tools

| Tool | Job | Typical placement |
|---|---|---|
| **Advanced Toner** | Environmental and narrative palette design | Before Keystone |
| **Keystone** | Technical balance, tone, color volume, gamut handling, and cleanup | Main grading hub |
| **LookLab** | Full creative grade / finishing look | After Keystone, before ODT |

## Recommended full stack

For the current AWG3 / LogC3 film-matrix workflow:

`Camera/CST -> FilmMatrix -> Advanced Toner -> Lens / optical stage -> Keystone -> LookLab -> ODT`

Use only the tools the shot needs. A neutral technical shot may use Keystone only. A location-driven grade may use Advanced Toner + Keystone. A hero creative shot may use the full stack.

## Design rules

- **Advanced Toner owns palette.**
- **Keystone owns balance and technical grade behavior.**
- **LookLab owns the final creative grade.**
- Shot correction remains upstream of creative look design.
- All three are intended to stay upstream of the display transform unless their documentation explicitly says otherwise.
- Neutral/bypass behavior is a release requirement.

## Repository model

Each tool lives in its own GitHub repository, carries its own license, has independent releases, and includes this system document so the relationship between the tools remains clear.
