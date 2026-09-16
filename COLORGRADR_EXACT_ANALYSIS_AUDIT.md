# ColorGradr exact-analysis port boundary

This build replaces Keystone's old 6x4 / 24-point Gray-World/Shades-of-Gray/Gray-Edge analyzer with the recovered **ColorGradr AdjustLevel analysis front-end** from the supplied `colorgradr.ofx.bundle` ARM64 executable.

## Recovered literally from the binary

ColorGradr Fix contains these embedded commands:

- `command level 0.1 99.9 2 96 30 0 1.25 24`
- `cmdrepeat 10 hue 50 5 30 0 0 360 24`
- `command level 0.1 99.9 3 90 10 0 1.25 24`

The level-analysis path used here was recovered from these named local functions in the ARM64 slice:

- `cOpAdjustLevel::calcTransform` @ `0x36714`
- `cOpAdjustLevel::calcAdjust` @ `0x37358`
- `cHistogramData::findCumPercentBin` @ `0x7a40`
- `cHistogramData::normalise` @ `0x7f78`
- `cHistogramData::average` @ `0x80a8`
- `cHistogramAdjuster::setToMatchThresholds` @ `0x6bfc`
- `cHueVecComparator::isInHueRange` @ `0x8be0`

Recovered front-end behavior:

1. The source RGB threshold histogram is full-frame. The embedded command also sets `CalcSubSampleX = CalcSubSampleY = 24` for the later hue-preservation analysis path.
2. Float RGB bin: `int(clamp(channel * 1023, 0, 1023))`, truncating toward zero.
3. Four 1024-bin histograms: R, G, B, and integer `(rBin+gBin+bBin)/3`.
4. `normalise(1024)`: for each non-negative bin `ceil(count*1024/maxCount)`.
5. `average(16)`: 16-sample arithmetic moving window with zero padding; recovered even-window alignment `j-7..j+8`; nearest/ties-away integer rounding.
6. Source endpoint percentiles are 0.1% and 99.9%.
7. `findCumPercentBin` computes a rounded cumulative target and returns the first bin at/over it.
8. `calcTransform` converts the selected bin to a normalized level using `/1024`.
9. ColorGradr's low/high selection helpers use 30% and 70% level percentiles.
10. ColorGradr's level candidate is an affine channel transform (`slope`, `offset`) constrained by hue-vector comparison.
11. The first Fix level pass uses `MaxAngleDiff=30`, `MinMagRatio=0`, `MaxMagRatio=1.25`; its full-frame hue-vector comparison gates correction strength.
12. Hue-vector chroma below `0.0001` contributes a zero vector; pre-vector magnitude below `0.00001` produces ColorGradr's `99999` ratio sentinel.
13. The comparator wraps hue difference into ±180 degrees and requires both magnitude ratio and max-angle limits to pass.

## Deliberate Keystone handoff

The user explicitly wants **ColorGradr analysis with Keystone math**, not ColorGradr's grading result. Therefore this port stops at the recovered ColorGradr RGB source endpoints. It does not apply ColorGradr's final affine `HistogramAdjuster` to the picture.

The recovered low/high RGB endpoints first pass through ColorGradr's recovered first-level hue-preservation strength search. The accepted/ramped endpoints are then collapsed to an illuminant estimate at their midpoint and passed into Keystone's existing `gainFromIllum` logic. The stored correction remains Keystone's multiplicative AWG4 RGB gain and is still applied at Keystone's existing pipeline position.

That boundary is intentional. Everything before the boundary above is the recovered ColorGradr level-analysis front-end; everything after it is Keystone.

## Explicit skin evidence restored

Keystone's explicit Primera-derived skin evidence is restored after the ColorGradr analysis handoff. It does not replace or bias ColorGradr's histogram endpoint analysis. It evaluates the proposed Keystone RGB neutral gain against detected skin and attenuates the gain only when the correction moves skin farther from `PRIMERA_SKIN_HUE_CENTER`. The old 6x4 skin evidence sampling was intentionally replaced by the same 24-pixel analysis lattice used by the recovered ColorGradr hue-analysis stage, reducing coordinate sensitivity while preserving the original Keystone skin-guard behavior.
