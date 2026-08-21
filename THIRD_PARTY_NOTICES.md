# Third-Party Notices

Keystone v2.5.1 is distributed as a combined work under **GNU GPL v3 only** (`GPL-3.0-only`). The notices below identify source incorporated into or directly adapted by the DCTL.

## Keystone prior code

Keystone v1.13.1 was distributed under the MIT License.

Copyright (c) 2026 Keystone contributors

The v2.4.2 DCTL retains Keystone's AWG3/XYZ and LogC3 plumbing, Bradford white balance, ACES Reference Gamut Compression wrapper, Primera-derived grading operators, Film Negative Space printer-light operator, output cleanup, and technical guards.

## Primera Suite — Geoff Smith

Source: https://github.com/geoffsmithBK/primera-suite
License: MIT

Earlier Keystone development adapted the soft black-point and rolling-contrast behavior from Primera Suite. v2.4.2 keeps those operators under Keystone-native function names while preserving credit here and in the README.

## Speak Film — OpenNR contributors

Source: https://github.com/amateurmenace/Speak
License: MIT
Copyright (c) 2026 OpenNR contributors

Used/adapted in Keystone:
- closed-form H&D characteristic curve
- Speak `neutralProfile()` negative and print parameters
- optical-density conversion
- density-domain subtractive saturation
- generic asymmetric dye-coupler coefficients
- density-domain split toning and its 18% gray pivot

MIT notice:

> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

## spektrafilm — Andrea Volpato

Source: https://github.com/andreavolpato/spektrafilm
Software license: GPLv3

Used/adapted in Keystone:
- non-spatial DIR inhibitor matrix convention (donor layer -> receiver layer)
- default same-layer and inter-layer DIR coefficients
- normalized-density DIR exposure correction
- pre-DIR characteristic-curve correction concept used to keep a gray ramp on the supplied characteristic curve
- enlarger Kodak CC optical-density convention (`100 CC = 1.0 density`)


Keystone does **not** ship or derive from spektrafilm stock profiles or LUTs. Those assets have a separate `CC BY-SA 4.0` license in spektrafilm's `SPEKTRAFILM_LICENSE.txt`; that asset license therefore does not attach to this package.

SpektraFilm's spatial DIR diffusion, grain, halation, diffusion-filter, glare, scanner, stock spectral sensitivity databases, and profile/LUT assets are not copied into this per-pixel DCTL.


## Uffy PhotoChemical Look Process

Source: https://github.com/RichardUffy/Uffy-PhotoChemical-Look-Process-for-DaVinci-Resolve-Studio

No Uffy source code or LUT data is incorporated. The repository was reviewed only as a workflow/component reference. Its README explicitly identifies several upstream/forked components; no repository-wide blanket license was established during this audit, so nothing from it was copied.

## Kodak2383_Emulation

Source: https://github.com/lakravana/Kodak2383_Emulation

No code or look data is incorporated. At audit time the repository exposed a README but no auditable implementation/license grant for a DCTL source, so it is excluded from Keystone.

### Primera Suite Pos Sat
Keystone v2.5.1 directly adapts the Pos Sat HSV algorithm from Primera Suite v0.6.0. Upstream: https://github.com/geoffsmithBK/primera-suite . The implementation provenance and source commit are documented in SOURCE_AUDIT.md.
