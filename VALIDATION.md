# Validation

Keystone RC28 uses structural and behavioral release gates.

## Structural gate

`python3 ci/validate_dctl.py` verifies:

- exactly one RC28 DCTL with the expected filename/header;
- Resolve transform/UI structure and balanced delimiters;
- GPL SPDX and no merge markers;
- no Metal UI/combo/function symbol collisions;
- manual Negative Mid/Below/Above and manual Pivot are absent;
- native Balance R/G/B, Output Negatives, and Output Skin Protect remain removed;
- fixed reversible Negative Space plumbing and live printer lights are present;
- true scene Exposure and selected-space/EI automatic contrast pivot are present;
- automatic Shadows/Highlights zone functions and monotonic Roll Off are present;
- gamut-aware Chroma/Hue protection is present;
- `Skin / Hue Uniformity` replaces the misleading former Evenness label;
- all seven ME_Desatch controls remain present in their working-transfer stage;
- White Clean and Black Clean remain manual output controls with automatic skin protection;
- catastrophic scene/encode safety remains present;
- removed Color Volume and creative White Point remain absent;
- README and this file contain the current DCTL SHA-256;
- tagged releases include scripts, third-party notices, and behavioral CI.

## Behavioral gate

`python3 ci/behavioral_validate.py` compiles the actual DCTL into a C++ CPU harness. The final `finite_or_zero()` fallback is replaced with a raw passthrough so upstream NaN/Inf cannot be hidden.

The harness checks:

- gamut matrix inverse round trips;
- supported transfer encode/decode round trips and every LogC3 EI option;
- bit-exact neutral identity across all 18 input spaces;
- fixed Negative Space forward/inverse monotonicity and roundtrip;
- automatic middle-gray derivation through every transfer/EI;
- Contrast preserves the derived 18% pivot across all 18 spaces and all LogC3 EI choices;
- global Exposure behaves as a true +1 scene-linear stop across all input spaces;
- printer R/G/B each produce a real correctly directed grade;
- Shadows affect the intended shadow zone without moving 18% gray/highlights;
- Highlights affect the intended highlight zone without moving shadows/18% gray;
- Roll Off is middle-gray neutral, highlight-only, and monotonic;
- Chroma/Hue limiting respects the native gamut safety margin;
- Skin Hue Uniformity changes selected skin color without materially changing luminance;
- exact ME_Desatch parity when it is the only active module;
- White Clean and Black Clean reduce near-neutral contamination in their respective tonal regions;
- **2,700,000 deterministic randomized full transforms** across all 18 input spaces with the emergency finite fallback disabled.

## Current source hash

`a4ed9deb235c33e81ae65a01a444f929d1d98389f133dbf2ee0db7f5aed5ff7a`  `Keystone-v1.0-RC28.dctl`

## Runtime gate

A DaVinci Resolve runtime smoke test remains required on the deployment host because CPU/static validation cannot reproduce Resolve's DCTL compiler or target GPU backend.
