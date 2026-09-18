#!/usr/bin/env bash
set -u
BUNDLE="${1:-/Library/OFX/Plugins/KeystoneOFX.ofx.bundle}"
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"
echo "== Keystone bundle =="
ls -ld "$BUNDLE" "$BIN" 2>&1 || true
echo "== architectures =="
lipo -archs "$BIN" 2>&1 || true
echo "== dependencies =="
otool -L "$BIN" 2>&1 || true
echo "== exports =="
nm -gU "$BIN" 2>&1 | grep -E 'OfxGetPlugin|OfxGetNumberOfPlugins|OfxSetHost' || true
echo "== signature =="
codesign -dv --verbose=4 "$BUNDLE" 2>&1 || true
codesign --verify --deep --strict --verbose=4 "$BUNDLE" 2>&1 || true
echo "== quarantine =="
xattr -l "$BUNDLE" 2>&1 || true
echo "== recent Resolve references =="
for LOG in "$HOME/Library/Application Support/Blackmagic Design/DaVinci Resolve/logs/ResolveDebug.txt" "$HOME/Library/Application Support/Blackmagic Design/DaVinci Resolve/logs/LogArchive/ResolveDebug.txt"; do
  [ -f "$LOG" ] && grep -i -C 8 -E 'Keystone|OFX.*fail|failed.*OFX|dlopen|code signature|image not found|symbol not found' "$LOG" | tail -n 160
done
