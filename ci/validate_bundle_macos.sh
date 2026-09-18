#!/usr/bin/env bash
set -euxo pipefail
BUNDLE="${1:-dist/KeystoneOFX.ofx.bundle}"
test -d "$BUNDLE"
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"
test -f "$BIN"
PLIST="$BUNDLE/Contents/Info.plist"
plutil -lint "$PLIST"
test "$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$PLIST")" = "KeystoneOFX.ofx"
test -s "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube" | awk '{print $1}')" = "9bd910e505f4f8fdfef67f97b85b7127f80528c4fda99e87ebb25eaa985c6d54"
ARCHS="$(lipo -archs "$BIN")"
[[ " $ARCHS " == *" arm64 "* ]]
[[ " $ARCHS " == *" x86_64 "* ]]
nm -gU "$BIN" | grep -q '_OfxGetNumberOfPlugins'
nm -gU "$BIN" | grep -q '_OfxGetPlugin'
! nm -gU "$BIN" | grep -q '_OfxSetHost'
! otool -L "$BIN" | grep -E '/opt/homebrew|/usr/local/opt'
codesign --verify --deep --strict "$BUNDLE"
