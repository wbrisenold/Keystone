# Validation

Keystone RC27 uses structural and behavioral release gates.

## Structural gate

`python3 ci/validate_dctl.py` verifies:

- exactly one release DCTL with the RC27 filename/header;
- Resolve transform/UI structure and balanced delimiters;
- GPL SPDX and no merge markers;
- no Metal UI/combo/function symbol collisions;
- permanent Film Negative Space architecture is present and the old Mode/Mid In/Blend UI is absent;
- printer lights are applied as a grade and are not divided out by the inverse;
- `Color / Mid Chroma` remains removed;
- all seven ME_Desatch controls remain present;
- ME_Desatch runs through the pre-output working-transfer stage rather than the final encoded tail;
- RC25 monotonic highlight and catastrophic-safety functions remain present;
- removed Color Volume and creative White Point remain absent;
- README and this file contain the current DCTL SHA-256;
- tagged releases include install/uninstall scripts, third-party notices, and behavioral CI.

## Behavioral gate

`python3 ci/behavioral_validate.py` compiles the actual DCTL into a C++ CPU harness. The final `finite_or_zero()` fallback is replaced with a raw passthrough so upstream NaN/Inf cannot be hidden.

The harness checks:

- gamut matrix inverse round trips;
- supported transfer encode/decode round trips and every LogC3 EI option;
- bit-exact neutral identity across all 18 input spaces;
- negative-response forward/inverse roundtrip and monotonicity;
- printer R/G/B each produce a real, correctly directed image change and no longer cancel;
- response-shape parameters alone remain a neutral working-space definition;
- response-shape parameters materially alter a legal tone move;
- Chroma/Vibrance/Hue are outside the negative remap and cannot be contaminated by changing negative response parameters when no in-sandwich grade is active;
- exact ME_Desatch parity when it is the only active module;
- highlight continuity/monotonicity;
- high-risk legal color + selective-desaturation combinations remain finite and above the catastrophic encoded floor;
- **2,700,000 deterministic randomized full transforms** across all 18 input spaces with the emergency finite fallback disabled.

## Current source hash

`71866fafd1a3cad917c31e4db936ca01c04b9c7f47cb1d1366faf3dc5cebdf91`  `Keystone-v1.0-RC27.dctl`

## Runtime gate

A DaVinci Resolve runtime smoke test remains required on the deployment host because CPU/static validation cannot reproduce Resolve's DCTL compiler or target GPU backend.
