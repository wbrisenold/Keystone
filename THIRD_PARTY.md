# Third-Party Notices

Keystone is distributed under GPL-3.0-or-later. It incorporates or adapts work from the projects below.

## ME_Desatch / Moaz Elgabry DCTLs

Keystone's encoded-output desaturation module is matched to `ME_Desatch.dctl` from the `MoazElgabry/DCTLs` repository at commit:

`5e57387d486e82e416cf25bc8a95aad5e7f33c7a`

That repository is GPL-3.0. The source header also credits cone-model work by Juan Pablo Zambrano, Quinn Leiho, Steve Yedlin, Matthias Stoopman, Jan Karow, Kaur Hendrikson, Nico Wieseneder, and Keidrych Wasley.

Because Keystone combines GPL-covered code with the rest of the DCTL, Keystone is distributed under GPL-3.0-or-later. The full GPL v3 text is included in `LICENSE`.

## Primera Suite / Geoff Smith

Keystone's primary Exposure, Black Point, Shadows, Roll Off, and rolling-contrast equations are derived from Primera Suite v0.5.5. RC26 retains Keystone's replacement of Primera's original highlight-gain equation with Keystone's own continuous/monotonic version after production testing found a discontinuity at code value 1.0.

Primera Suite is licensed under the MIT License:

MIT License

Copyright (c) 2026 Geoff Smith

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Film Negative Space CST / Dec. 18 Studios

Keystone RC26's optional internal Negative Space uses the publicly documented processing model of Dec. 18 Studios' Film Negative Space CST as a behavioral/reference concept: scene-linear decode, middle-gray relocation, independent contrast above/below the pivot, printer-light-style density controls, grading in the remapped space, and inverse return.

Keystone does **not** contain or redistribute the Film Negative Space CST DCTL/DCTLE source or release asset. The RC26 implementation was independently written for Keystone from the public documentation and validated as its own reversible transform. This is attribution for the workflow concept/reference, not a code-license dependency.

Reference project: `https://github.com/Dec18studios/Film-Negative-Space-CST-DCTL`
Documentation: `https://tools.dec18studios.com/color-grading-tools/film-negative-space-cst/`
