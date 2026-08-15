# Validation

This repository packages the existing DCTL source without changing its image-processing code.

## Automated checks

- exactly one release DCTL;
- Resolve `transform` entry point;
- UI declarations;
- balanced delimiters;
- no Git merge-conflict markers;
- SPDX identifier;
- no UI/combo enum collision with global `__DEVICE__` functions;
- SHA-256 output for the release source.

## Current source hash

`66daa1c3c0dc1e58ff99d896a7064baf91215d4ca172b67259000d5c6225fa04`  `Keystone-v1.0-RC24-Metal-Collision-Fix.dctl`

Static validation does not replace DaVinci Resolve runtime validation.
