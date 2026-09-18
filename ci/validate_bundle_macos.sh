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
test -s "$BUNDLE/Contents/Resources/Referent_LogC4_to_Rec709.cube"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/Referent_LogC4_to_Rec709.cube" | awk '{print $1}')" = "19b2feb5ed8cb767d980e9f9b351b6e1823a3990974277fdb4a46d1f709d251c"

CGBIN="$BUNDLE/Contents/Resources/ColorGradr/colorgradr.ofx.bundle/Contents/MacOS/colorgradr.ofx"
test -f "$CGBIN"
CGARCHS="$(lipo -archs "$CGBIN")"; [[ " $CGARCHS " == *" arm64 "* ]]; [[ " $CGARCHS " == *" x86_64 "* ]]
nm -gU "$CGBIN" | grep -q '_OfxGetNumberOfPlugins'
nm -gU "$CGBIN" | grep -q '_OfxGetPlugin'

codesign --verify --deep --strict "$BUNDLE"
ARCHS="$(lipo -archs "$BIN")"; [[ " $ARCHS " == *" arm64 "* ]]; [[ " $ARCHS " == *" x86_64 "* ]]

SCENE="$BUNDLE/Contents/Resources/KeystoneSceneEngine.dylib"
test -f "$SCENE"
SARCHS="$(lipo -archs "$SCENE")"; [[ " $SARCHS " == *" arm64 "* ]]; [[ " $SARCHS " == *" x86_64 "* ]]
nm -gU "$SCENE" | grep -q '_KeystoneSceneSegment'
! otool -L "$BIN" | grep -Ei 'KeystoneSceneEngine|ncnn'
! otool -L "$SCENE" | grep -E '/opt/homebrew|/usr/local/opt'
codesign --verify --strict "$SCENE"
