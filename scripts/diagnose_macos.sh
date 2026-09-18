#!/usr/bin/env bash
set -euo pipefail
BUNDLE="${1:-}"
if [[ -z "$BUNDLE" || ! -d "$BUNDLE" ]]; then
  echo "usage: $0 /path/to/KeystoneOFX.ofx.bundle" >&2
  exit 2
fi
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"
SCENE="$BUNDLE/Contents/Resources/KeystoneSceneEngine.dylib"
PLIST="$BUNDLE/Contents/Info.plist"

echo "== Keystone bundle =="
echo "$BUNDLE"

echo "\n== Info.plist =="
plutil -lint "$PLIST"
/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$PLIST"
/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$PLIST"

echo "\n== Main OFX architectures =="
lipo -archs "$BIN"

echo "\n== Main OFX exports =="
nm -gU "$BIN" | grep -E '_Ofx(GetNumberOfPlugins|GetPlugin|SetHost)$'

echo "\n== Main OFX runtime dependencies =="
otool -L "$BIN"
if otool -L "$BIN" | grep -Ei 'KeystoneSceneEngine|ncnn'; then
  echo "ERROR: main OFX has a hard scene-runtime dependency" >&2
  exit 3
fi

echo "\n== Main bundle signature =="
codesign --verify --deep --strict --verbose=2 "$BUNDLE"

if [[ -f "$SCENE" ]]; then
  echo "\n== Scene sidecar architectures =="
  lipo -archs "$SCENE"
  echo "\n== Scene sidecar export =="
  nm -gU "$SCENE" | grep '_KeystoneSceneSegment$'
  echo "\n== Scene sidecar runtime dependencies =="
  otool -L "$SCENE"
  echo "\n== Scene sidecar signature =="
  codesign --verify --strict --verbose=2 "$SCENE"
else
  echo "\nScene sidecar not present. Keystone should still load; Analyze Scene will use fallback regions."
fi

echo "\nDIAGNOSTICS_OK"
