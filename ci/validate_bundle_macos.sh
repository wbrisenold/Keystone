#!/usr/bin/env bash
set -euxo pipefail
ZIP=dist/KeystoneOFX-macOS-universal.zip
test -s "$ZIP"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
ditto -x -k "$ZIP" "$TMP"
BUNDLE="$(find "$TMP" -type d -name 'KeystoneOFX.ofx.bundle' -print -quit)"; test -d "$BUNDLE"
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"; test -f "$BIN"
PLIST="$BUNDLE/Contents/Info.plist"; plutil -lint "$PLIST"; test "$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$PLIST")" = "KeystoneOFX.ofx"
test -s "$BUNDLE/Contents/Resources/KeystoneKernels.metallib"
test -s "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube" | awk '{print $1}')" = "9bd910e505f4f8fdfef67f97b85b7127f80528c4fda99e87ebb25eaa985c6d54"

CGBIN="$BUNDLE/Contents/Resources/NativeMatch/NativeMatch.ofx.bundle/Contents/MacOS/NativeMatch.ofx"
test -f "$CGBIN"
CGARCHS="$(lipo -archs "$CGBIN")"; [[ " $CGARCHS " == *" arm64 "* ]]; [[ " $CGARCHS " == *" x86_64 "* ]]
nm -gU "$CGBIN" | grep -q '_OfxGetNumberOfPlugins'
nm -gU "$CGBIN" | grep -q '_OfxGetPlugin'

codesign --verify --deep --strict "$BUNDLE"
ARCHS="$(lipo -archs "$BIN")"; [[ " $ARCHS " == *" arm64 "* ]]; [[ " $ARCHS " == *" x86_64 "* ]]

# Native NativeMatch bridge payload must itself be a valid universal Mach-O bundle.
CGBIN="$BUNDLE/Contents/Resources/NativeMatch/NativeMatch.ofx.bundle/Contents/MacOS/NativeMatch.ofx"
test -f "$CGBIN"
file "$CGBIN" | grep -q 'Mach-O universal binary'
otool -L "$CGBIN" >/dev/null
nm -gU "$CGBIN" | grep -q '_OfxGetNumberOfPlugins'
nm -gU "$CGBIN" | grep -q '_OfxGetPlugin'
codesign --verify --deep --strict "$BUNDLE"
