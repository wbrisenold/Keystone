# Validation

Keystone RC26 uses structural and behavioral release gates.

## Structural gate

`python3 ci/validate_dctl.py` verifies:

- exactly one release DCTL and the expected RC26 filename/header;
- Resolve transform/UI structure and balanced delimiters;
- GPL SPDX and no merge markers;
- no Metal-style UI/combo/function symbol collisions;
- RC25 monotonic-highlight and encode-safety functions remain present;
- RC26 internal Negative Space forward/inverse controls/functions are present;
- removed Color Volume and creative white-point controls remain absent;
- README/Validation hashes match the release DCTL;
- release workflow includes scripts, third-party notices, and behavioral CI.

## Behavioral gate

`python3 ci/behavioral_validate.py` compiles the DCTL into a C++ CPU harness. The final `finite_or_zero()` fallback is replaced with a raw passthrough so upstream NaN/Inf cannot be hidden.

The harness checks:

- all 13 gamut matrix/inverse round trips;
- all supported transfer encode/decode paths and all 11 LogC3 EI selections;
- bit-exact neutral bypass across all 18 input spaces;
- RC25 off-path golden outputs across all 18 input spaces;
- Negative Space forward/inverse roundtrip across positive, zero, and negative scene-linear values;
- Negative Space monotonicity across hard and blended pivot transitions;
- exact neutral bypass with Negative Space enabled and deliberately extreme working-space settings;
- a material response difference between standard Keystone tone behavior and the internal Negative Space tone behavior;
- RC26 highlight continuity/monotonicity and full-transform upper-range monotonicity;
- encoded safety under known high-risk legal color-control combinations;
- 2,700,000 deterministic randomized transforms across all 18 input spaces with Negative Space randomly enabled/disabled;
- 45,000 additional randomized blended-Negative-Space transforms;
- no non-finite outputs, no catastrophic encoded-negative escapes, and no runaway encoded outputs in those stress sets.

Development qualification also compared RC26 with Negative Space Off against RC25 across 20,000 randomized non-neutral grades and measured bit-for-bit identical output.

## Current source hash

`3f10ade4940d86483f870c8dc3f563c4867be59fad37413a71b14df0189c4143`  `Keystone-v1.0-RC26.dctl`

## Host gate

A DaVinci Resolve runtime smoke test remains required on the deployment host because CPU/static validation cannot reproduce Resolve's DCTL compiler or the target GPU backend.
