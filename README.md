# Keystone v2.6.1

Keystone is a DaVinci Resolve DCTL for balancing and finishing an image in
ARRI Wide Gamut 3 / LogC3 EI800. In beginner terms, it is the stage where you
make the picture technically dependable before you add a display look.

Keystone is not a one-click film look. It gives you separate controls for
white balance, exposure, tone, density color, print behavior, gamut cleanup,
and diagnostics. That separation lets you understand which decision changed
the image.

## Start here if you are new

Keystone is the **make the picture dependable** part of the larger system. It is
not the first thing you add, and it is not the final look. The beginner path is:

1. Record or import a short clip. A supported phone can use Blackmagic Camera and may offer Apple Log.
2. Open Resolve Studio and go to the Color page.
3. Add the CST first so the camera file is translated into the working space.
4. Add PresenceOFX if the image needs a little more shape or softness.
5. Add Keystone and fix white balance, exposure, tone, and density.
6. Add Referent and the look only after the picture is stable.

![Resolve node editor](https://wbrisenold.github.io/KB-Tools/assets/images/resolve/nodes.jpg)

**Analogy:** if PresenceOFX arranges the furniture, Keystone makes sure the
floor is level and the lights work. Do not decorate the room before fixing the
floor.

If you do not know the words yet, use the [Beginner Handbook](https://wbrisenold.github.io/KB-Tools/guides/beginner-handbook.html). It explains phone setup, Log, exposure, nodes, CST, LUTs, and scopes in ordinary language before returning here for Keystone's controls.

### How to add Keystone

On the Color page, select a node, open **Effects Library** in the upper-right,
search for **DCTL**, and place Keystone on its own node. If the DCTL does not
appear, refresh Resolve's LUT list or restart Resolve. The node should stay
easy to bypass so you can compare the picture with and without Keystone.

## Before you start

You need **DaVinci Resolve Studio** to use this workflow as documented. You
also need footage from a camera that can record a Log image, or a phone that
offers a Log recording mode. Many people already have a usable Log camera in
their pocket: a supported phone can record Apple Log when its camera app or
capture workflow exposes that option. A cinema camera, mirrorless camera, or
phone can all be the source; the important part is knowing which Log format was
recorded.

### What Log means

Log is a way of storing a wide range of brightness values in a camera file.
Instead of making the footage look finished in the camera, Log leaves room for
bright skies, faces, and shadows to be shaped later. Straight out of the file,
Log footage often looks gray, flat, or low contrast. That is normal. It is not
the final image and it is not a sign that the footage is broken.

Log is not one universal format. Apple Log, Canon Log 3, ARRI LogC3, and other
formats use different curves and color primaries. The CST at the start of the
tree must know what the camera recorded. Never select ARRI LogC3 just because
the image looks flat; select it only when the source and transform are correct.

The documented system converts the camera signal to ARRI Wide Gamut 3 / LogC3
for the working stages. The connected workflow was tested on Apple Log and
Canon Log 3 footage, but the exact CST settings still depend on the camera,
recording mode, and project setup.

### Why LogC3 is the working space here

Keystone is authored around ARRI Wide Gamut 3 / LogC3 as a predictable,
scene-referred working signal. The first CST translates the camera's native Log
format into that common language, so Keystone can apply exposure, tone, density,
print behavior, and cleanup without guessing which camera curve arrived at its
input.

The wide working gamut leaves room for creative changes before the display
transform, and LogC3 gives the next Referent stage a known handoff. This does
not make LogC3 the right space for every project. ACEScct, DaVinci Wide Gamut,
or another managed space may be better elsewhere. Use LogC3 here because this
tool, this node order, and this display handoff were designed together.

## What is a DCTL?

A DCTL is a small color-processing program that runs inside DaVinci Resolve.
It appears as a tool in the Color page and can expose sliders, menus, and
diagnostic views. A DCTL does not replace Resolve's node graph. It lives inside
one node, which means you can bypass it, compare it, and place it next to other
stages in a controlled order.

## Keystone's role in the system

The connected system is documented in the [KB Tools node guide](https://wbrisenold.github.io/KB-Tools/guides/resolve-node-guide.html). Beginners can start with the [Beginner Handbook](https://wbrisenold.github.io/KB-Tools/guides/beginner-handbook.html), then use the [Resolve visual atlas](https://wbrisenold.github.io/KB-Tools/guides/resolve-visual-atlas.html) to see the real Color page before installing anything. Keystone sits after [PresenceOFX](https://github.com/wbrisenold/PresenceOFX) and before the display-side tools:

```text
Camera transform -> LogC3
    -> PresenceOFX: image character
    -> Keystone: balance and technical finishing
    -> Color Separation: creative color relationship
    -> Referent: display foundation
    -> Look LUT: local display look
    -> MonoNodes: chart and display QC
```

The order matters. PresenceOFX can shape the image while it is still in the
working space. Keystone then stabilizes exposure, tone, white balance, and
range. After that, the image can move into display space and receive a look.
Do not use a later look LUT to repair a bad input transform or an unstable
balance.

Henry Bobeck's [Color Separation DCTL](https://henrybobeck.com/dctl/ColorSeparation)
is a separate paid product. It is not included in Keystone. If you use it,
purchase it from Henry and support its creator.

## Reference pictures

![Blackmagic cinema camera](https://wbrisenold.github.io/KB-Tools/assets/images/resolve/camera.jpg)

The camera is the source of the signal. Before Keystone can balance the image,
the first CST must identify what the camera recorded, including whether it was
Apple Log, Canon Log 3, ARRI LogC3, or another format.

![ColorChecker chart](https://wbrisenold.github.io/KB-Tools/assets/images/resolve/color-checker.svg)

The ColorChecker is a neutral reference while setting white balance, exposure,
density, and saturation. It does not replace a creative decision; it helps you
separate a technical problem from a look choice.

## What Keystone does internally

The processing order is:

```text
Input repair
-> Bradford white balance
-> Scene exposure
-> Soft black point
-> Negative-space RGB printer trims
-> LogC3 rolling contrast
-> Shadows and highlights
-> Rolloff
-> H&D negative development and optional DIR
-> Enlarger C/M/Y filtration and master print exposure
-> Print H&D
-> Density subtractive color
-> Density look / split tone
-> Safety guard
-> White/black cleanup with skin protection
-> Technical guards
-> LogC3 EI800 output
```

You do not need to memorize this list to use the DCTL. It explains why the
controls are grouped: balance happens before density and look behavior, and
safety cleanup happens before the signal leaves Keystone.

## How the image should feel

Start by looking for stability, not style. A good neutral setup should give you
a settled middle gray, cleaner neutrals, readable faces, and highlights that
roll into place instead of snapping or collapsing. Once that foundation is
working, the film controls can add density and color character without carrying
the burden of basic correction.

If the image becomes less stable when you move a creative control, return to
the neutral state and solve the earlier problem first. Maximum slider values
are not the goal. The useful result is an image that still has room for the
separation and display stages that follow.

## Controls in plain language

### White balance and tint

These controls settle neutral objects and unwanted green or magenta casts. Use
them before judging a look. If a gray card, white wall, or neutral highlight is
wrong, do not try to fix it with a color-separation slider.

### Scene exposure

Exposure moves the image as a whole. It should change where the image sits,
not turn into a new contrast style. If exposure changes the mood too much,
check the tone controls rather than pushing exposure harder.

### Tone, shadows, highlights, and rolloff

These controls decide how the image distributes contrast. Shadows should gain
shape without becoming empty, highlights should retain a graceful transition,
and rolloff should protect bright values without making the image dull. Move
one tonal area at a time and compare against bypass.

### Film Profile

`Film / Profile` chooses a starting behavior such as Neutral, Latitude, Punch,
or Chrome. These are authored response families, not claims to reproduce a
named stock. The slider or menu exists so you can choose a broad direction
before making smaller trims.

### Film Strength

`Film / Strength` scales the film-related response: development behavior, DIR,
print exposure, C/M/Y filtration, subtractive color, and split/look density.
It does not change white balance, scene exposure, basic contrast, shadows,
highlights, rolloff, or output cleanup. That boundary keeps the control useful:
you can reduce the film character without undoing the technical balance.

### Print C/M/Y and master print exposure

The C, M, and Y filtration controls behave like explicit printer decisions.
They change color density through the print stage rather than acting like a
generic RGB saturation knob. Master print exposure changes the print brightness
inside that same model. Keep these controls subtle until the neutral image is
already working.

### Density look and split tone

Presets provide a starting relationship between shadows and highlights. The
pivot is protected so the preset does not silently tint the middle gray. Use
the manual shadow and highlight trims when the broad preset is close but not
right for the shot.

### Color / Dye and Pos Sat

These controls change density behavior and saturation behavior separately. Dye
is about how color behaves inside the density model. Pos Sat is a more direct
saturation response in the LogC3-encoded working signal. If the image starts
to look painted, reduce these before changing the display LUT.

### View / Mode

Diagnostics include Result, Neutral Chroma, Density, Gamut Stress, and Skin
Mask views. Use them to see where a control is acting. The diagnostic views are
especially useful when a slider looks attractive in one part of the frame but
creates unwanted color or range problems elsewhere.

## What changed in v2.6.1

Version 2.6.1 keeps the matrix-free photochemical core, print-stage C/M/Y
filtration, and density-look presets. It replaces the artifact-prone density
subtractive-saturation control with Primera Suite Pos Sat behavior.

It also includes the stability correction that withdraws v2.6.0 and rebuilds
the density-domain hardening from v2.5.1 with helper declarations and runtime
clamps matched to the UI ranges.

## Installation and validation

The repository contains `Keystone.dctl` and the project license. Install the
DCTL using Resolve's normal LUT/DCTL workflow, then refresh the LUT list or
restart Resolve if the tool does not appear.

Run the repository checks with:

```bash
python3 tools/validate.py Keystone.dctl
```

The repository's current validation script checks DCTL structure and supported
source patterns. GitHub Actions runs the same command on pushes and releases.
The final compatibility check still belongs in the exact Resolve Studio version,
GPU, project color management, and monitoring setup where you will use Keystone.

## Learn the surrounding ideas

- [Blackmagic Design Color training](https://www.blackmagicdesign.com/products/davinciresolve/training) — official beginner lessons for the Color page, scopes, and color management.
- [Cullen Kelly Color](https://www.youtube.com/@CullenKellyColor) — scene-referred grading and display-transform education.
- [Cullen Kelly Referent search](https://www.youtube.com/results?search_query=Cullen+Kelly+Referent+LUT) — Referent-specific tutorials and current usage examples.
- [KB Tools node guide](https://wbrisenold.github.io/KB-Tools/guides/resolve-node-guide.html) — the complete connected chain and exact node order.

Use the general lessons to understand grading. Use this README to understand
what Keystone owns: neutral balance, tone, density, film response, gamut
cleanup, and diagnostics. PresenceOFX comes before it; Referent, the Look LUT,
and MonoNodes come after it.

## Testing and limitations

This project was vibe coded with human direction and AI assistance. The source,
licenses, tests, packaging, and validation steps were reviewed and organized
around repeatable checks, but generated code can still contain mistakes or
host-specific problems.

The connected workflow was tested on Apple Log and Canon Log 3 footage. Those
tests describe the author's setup, not universal support for every camera,
input transform, GPU, Resolve version, or monitoring pipeline. Before paid or
archival work, test the DCTL in the exact Resolve environment where it will be
used.

## Credits and license

Keystone combines original integration work with openly available color-science
code and ideas. Attribution is also recorded in `THIRD_PARTY_NOTICES.md`.

- [Speak](https://github.com/amateurmenace/Speak): H&D negative/print curves, profiles, printer points, density color, dye coupling, and split-tone foundations.
- [spektrafilm](https://github.com/andreavolpato/spektrafilm): DIR development behavior and enlarger filtration conventions.
- [Primera Suite](https://github.com/geoffsmithBK/primera-suite): black-point and rolling-contrast behavior retained from earlier development.
- [Thatcher Freeman utility-dctls](https://github.com/thatcherfreeman/utility-dctls): reference work for tone-scale and DCTL development.
- [ACES Reference Gamut Compression](https://github.com/ampas/aces-vwg-gamut-mapping-2020): optional gamut-repair reference.
- [Photographic DCTLs](https://github.com/mikaelsundell/photographic-dctls), [Uffy PhotoChemical Look Process](https://github.com/RichardUffy/Uffy-PhotoChemical-Look-Process-for-DaVinci-Resolve-Studio), [Kodak2383 Emulation](https://github.com/lakravana/Kodak2383_Emulation), and [Dec. 18 Studios](https://github.com/Dec18studios): reviewed workflow and research references.
- [Henry Bobeck Color Separation](https://henrybobeck.com/dctl/ColorSeparation): separate downstream creative tool, not incorporated into Keystone.

Keystone is distributed under **GPL-3.0-only** because GPLv3-covered source is
incorporated into the combined work. No spektrafilm stock profile or LUT
database assets are bundled.

### Visual references

- [Blackmagic Cinema Camera](https://commons.wikimedia.org/wiki/File:Blackmagic_Cinema_Camera.JPG) — MMuzammils, CC BY-SA 3.0.
- [Color Checker SVG](https://commons.wikimedia.org/wiki/File:Color_Checker.svg) — Glrx, CC0/Public Domain Dedication.

## Further reading

- [KB Tools system guide](https://wbrisenold.github.io/KB-Tools/guides/resolve-node-guide.html)
- [PresenceOFX](https://github.com/wbrisenold/PresenceOFX), the preceding image-character stage
- [Cullen Kelly Referent](https://cullenkellycolor.com/toolkit/referent), the free display foundation
- [MonoNodes DCTLs](https://mononodes.com/dctls/), the chart and workflow-tool source
