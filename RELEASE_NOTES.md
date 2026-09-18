# KeystoneOFX v1.6.0

- Corrected the OpenFX host lifecycle: `setHost` now only stores the host pointer; suites are fetched during `OfxActionLoad`.
- Removed the nested OFX engine architecture.
- Removed Metal from the load path for the compatibility build.
- Reduced supported context to Filter only.
- Removed the non-mandated global `OfxSetHost` export.
- Retained one-frame Auto Match using the recovered histogram/hue-preservation analysis front-end, without the previous skin-first steering.
