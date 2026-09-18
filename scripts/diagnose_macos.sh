#!/usr/bin/env bash
set -u
BUNDLE="${1:-/Library/OFX/Plugins/KeystoneOFX.ofx.bundle}"
echo "Keystone OFX diagnostic"
echo "Bundle: $BUNDLE"
if [ ! -d "$BUNDLE" ]; then echo "ERROR: bundle not found"; exit 2; fi
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"
echo "-- Info.plist --"
plutil -lint "$BUNDLE/Contents/Info.plist" || true
/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$BUNDLE/Contents/Info.plist" 2>/dev/null || true
echo "-- Main binary --"
file "$BIN" || true
lipo -archs "$BIN" 2>/dev/null || true
otool -L "$BIN" 2>/dev/null || true
codesign --verify --deep --strict --verbose=2 "$BUNDLE" 2>&1 || true
echo "-- Embedded match engine --"
ENGDIR="$BUNDLE/Contents/Resources/NativeMatch/NativeMatch.ofx.bundle/Contents/MacOS"
if [ -d "$ENGDIR" ]; then
  ENG="$(find "$ENGDIR" -maxdepth 1 -type f -print -quit)"
  echo "Engine present: ${ENG:+yes}"
  if [ -n "$ENG" ]; then file "$ENG" || true; lipo -archs "$ENG" 2>/dev/null || true; otool -L "$ENG" 2>/dev/null || true; fi
else
  echo "Engine directory missing"
fi
echo "-- Runtime loader log --"
if [ -f /tmp/KeystoneOFX-loader.log ]; then cat /tmp/KeystoneOFX-loader.log; else echo "No runtime loader log yet"; fi
