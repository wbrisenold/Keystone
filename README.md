# Keystone v2.6.1

Keystone is the **make the picture dependable** stage in the KB Tools workflow.
It helps set white balance, exposure, tone, density, print behavior, and
technical cleanup inside DaVinci Resolve Studio.

It is not the first step, not the viewing transform, and not a one-click film
look.

## If you are new

Start with the [KB Tools Beginner Handbook](https://wbrisenold.github.io/KB-Tools/guides/beginner-handbook.html). It explains phone recording, Log, exposure, nodes, CST, LUTs, and scopes in plain language.

The larger order is:

```text
Camera clip
    -> CST
    -> PresenceOFX
    -> Keystone
    -> Color Separation
    -> Referent ODT (always on)
    -> Look LUT
    -> MonoNodes QC
```

**Analogy:** PresenceOFX is arranging the furniture. Keystone makes sure the
floor is level and the lights work. The Look LUT decorates the room only after
the floor is level.

![Resolve node editor](https://wbrisenold.github.io/KB-Tools/assets/images/resolve/nodes.jpg)

## What you need before adding Keystone

- DaVinci Resolve Studio.
- A short camera or phone clip.
- A correct CST before Keystone.
- PresenceOFX before Keystone if you are using the complete KB Tools chain.

The source can be Apple Log, Canon Log 3, ARRI LogC3, or another format. The
first CST must match the real source. Do not choose a format just because the
clip looks flat.

## How to add Keystone

Do this only after the CST and PresenceOFX nodes:

1. Open the Color page.
2. Click the node after PresenceOFX.
3. Open Effects Library in the upper-right.
4. Search for **DCTL**.
5. Add Keystone to its own node.
6. Leave the node easy to bypass while you learn.

The tool is designed for ARRI Wide Gamut 3 / LogC3 EI800. In this system, the
CST creates that working signal before Keystone receives it.

## Use Keystone in this order

### First: make neutral objects look neutral

Use white balance and tint. A white wall should not look obviously yellow,
blue, green, or pink unless that is truly the light in the scene.

### Second: set how light or dark the picture is

Use scene exposure. Exposure means the overall light level. It should not be
used to create a dramatic color style.

### Third: shape the light

Use tone, shadows, highlights, and rolloff. Shadows should keep some detail.
Highlights should change smoothly instead of snapping to white.

### Fourth: add density and print character

Film Profile, Film Strength, print exposure, C/M/Y filtration, Dye, Pos Sat,
and split tone belong here. Start gently. A control that makes a picture more
interesting can still make skin, skies, or neutral objects less believable.

### Fifth: inspect the result

Use the View/Mode options: Result, Neutral Chroma, Density, Gamut Stress, and
Skin Mask. These are ways to see what the tool is doing, not extra looks.

## Controls in everyday language

- **White balance and tint:** make white and gray objects look believable.
- **Scene exposure:** make the whole image lighter or darker.
- **Tone:** decide how strongly dark, middle, and bright parts separate.
- **Shadows and highlights:** change those areas without moving everything.
- **Rolloff:** make bright areas move gently into white.
- **Film Profile:** choose a broad starting behavior such as Neutral, Latitude, Punch, or Chrome.
- **Film Strength:** turn the film-related character up or down without undoing basic balance.
- **Print C/M/Y:** make printer-style color changes, not a generic saturation boost.
- **Master print exposure:** change brightness inside the print behavior.
- **Dye and Pos Sat:** change density color and saturation behavior separately.
- **View/Mode:** look at special views to find where a control is acting.

## Referent is not a toggle-able look

Referent is Cullen Kelly's free viewing LUT and display foundation. In this
workflow it is the ODT. Keep Referent enabled while using Keystone and while
judging the picture. It is not a creative effect to turn off for a normal
before/after comparison.

You may bypass Referent for a moment to inspect the underlying LogC3 signal when
diagnosing a problem. Turn it back on before making grading decisions.

## If the result is wrong

- Everything has the wrong color: check the CST input before Keystone.
- White objects are tinted: use white balance and tint.
- Everything is too dark or bright: use scene exposure.
- The picture is harsh: use tone, rolloff, shadows, and highlights before adding film strength.
- The picture looks painted: lower Dye or Pos Sat.
- The film character is too strong: lower Film Strength.
- A bright area breaks: inspect rolloff and lower the adjustment.
- Keystone is missing: refresh the DCTL/LUT list or restart Resolve.

## Installation and validation

Install `Keystone.dctl` using Resolve's normal DCTL/LUT workflow. Refresh the
LUT list or restart Resolve if it does not appear.

Run the repository check:

```bash
python3 tools/validate.py Keystone.dctl
```

The check covers DCTL structure and supported source patterns. Final testing
must happen in the exact Resolve Studio, GPU, project, and monitoring setup you
will use.

## Fifteen visual lessons for Keystone users

These are the fifteen pictures to review before asking Keystone to solve a
problem. They are deliberately broader than the DCTL so you know which stage
owns the problem:

1. [Download Blackmagic Camera](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#phone-download)
2. [Set up the phone](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#phone-settings)
3. [Understand a pale Log clip](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#log-recording)
4. [Find the Color page](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#color-page)
5. [Read the node editor](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#nodes)
6. [Check the CST before Keystone](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#cst)
7. [Use primary controls](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#primary)
8. [Understand the PresenceOFX handoff](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#presence)
9. [Understand Keystone's job](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#keystone)
10. [Keep the Referent ODT on](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#referent)
11. [Keep the Look LUT after Referent](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#look)
12. [Use waveform to check brightness](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#waveform)
13. [Use parade and vectorscope](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#color-scopes)
14. [Use selections only when needed](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#selections)
15. [Compare before and after](https://wbrisenold.github.io/KB-Tools/guides/resolve-resource-library.html#compare)

These links are part of the Keystone teaching path, not claims that every
picture is a Keystone screenshot. Each card identifies the Resolve component
and links to its authoritative source.

## Credits and license

Keystone combines original integration work with openly available color-science
code and ideas. Attribution is recorded in `THIRD_PARTY_NOTICES.md`.

- [Speak](https://github.com/amateurmenace/Speak)
- [spektrafilm](https://github.com/andreavolpato/spektrafilm)
- [Primera Suite](https://github.com/geoffsmithBK/primera-suite)
- [Thatcher Freeman utility-dctls](https://github.com/thatcherfreeman/utility-dctls)
- [ACES Reference Gamut Compression](https://github.com/ampas/aces-vwg-gamut-mapping-2020)
- [Henry Bobeck Color Separation](https://henrybobeck.com/dctl/ColorSeparation), separate paid tool.

Keystone is distributed under **GPL-3.0-only** because GPLv3-covered source is
incorporated into the combined work.

Further reading: [KB Tools](https://github.com/wbrisenold/KB-Tools), [PresenceOFX](https://github.com/wbrisenold/PresenceOFX), and [Cullen Kelly Referent](https://cullenkellycolor.com/toolkit/referent).
