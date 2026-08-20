# Third-Party Notices

Keystone includes or adapts ideas from third-party open-source color-science work. These notices are kept with the repository so redistribution preserves the relevant attribution.

## Thatcher Freeman — utility-dctls

Keystone's Daniele tone-scale core follows Thatcher Freeman's MIT-licensed DCTL implementation, based on Daniele Siragusano's tone-scale proposal.

MIT License

Copyright (c) 2023 Thatcher Freeman

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Project: `thatcherfreeman/utility-dctls`

## Geoff Smith — PrimeraSkin / primera-suite

Keystone's selective skin-control workflow was informed by PrimeraSkin, including the use of brightness-independent chromaticity ideas for skin qualification.

MIT License

Copyright (c) 2026 Geoff Smith

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Project: `geoffsmithBK/primera-suite`

## Academy Color Encoding System — ACES

The optional `Input / Fix` mode implements the core structure of ACES 1.3 Reference Gamut Compression in an AP1 wrapper. ACES reference transforms are distributed by the Academy Software Foundation under the Apache License 2.0.

Project: `aces-aswf/aces` and related ACES reference repositories.

Nothing in this repository implies endorsement by the Academy Software Foundation, the Academy of Motion Picture Arts and Sciences, ARRI, Blackmagic Design, or any other vendor whose color spaces or products are referenced.

## Keystone Skin Section

The current Keystone Skin section is intentionally designed to behave like a Primera-style skin-control module. It is an independent Keystone implementation informed by that workflow and redistributed under Keystone's repository license structure with this attribution notice preserved.

## Primera Pos Saturation

`Color / Pos Sat` in Keystone v1.10.0 adapts the Pos. Saturation algorithm from Primera by Geoff Smith: encoded RGB is converted to HSV, saturation is multiplied by `2^amount`, capped at 1.0, then converted back to RGB. Keystone applies this in its fixed AWG3 / LogC3 EI800 signal.

Primera / primera-suite is MIT licensed. Copyright (c) 2026 Geoff Smith. The full Primera MIT notice above remains applicable.

## Primera Contrast

`Tone / Contrast` in Keystone v1.11.0 adapts Primera's `rolling_contrast` algorithm by Geoff Smith. Keystone applies it in its fixed AWG3 / LogC3 EI800 signal at Primera's default Pivot value of 0.0.

Primera / primera-suite is MIT licensed. Copyright (c) 2026 Geoff Smith. The Primera MIT notice in this repository remains applicable.

## Primera Black Point

`Tone / Black Pt` in Keystone v1.12.0 adapts Primera's `soft_black_point` algorithm by Geoff Smith. It uses the same `0.005` knee and Primera slider semantics, applied to Keystone's scene-linear AWG3 RGB immediately after Exposure.

Primera / primera-suite is MIT licensed. Copyright (c) 2026 Geoff Smith. The Primera MIT notice in this repository remains applicable.
