#!/usr/bin/env bash
set -euxo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
command -v cmake >/dev/null
command -v clang++ >/dev/null
command -v lipo >/dev/null
python3 ci/source_sanity.py
rm -rf build dist
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel "$(sysctl -n hw.ncpu)"
BIN="$(find build -type f -name 'KeystoneOFX.ofx' -print -quit)"
test -n "$BIN"; test -f "$BIN"
ARCHS="$(lipo -archs "$BIN")"
[[ " $ARCHS " == *" arm64 "* ]]
[[ " $ARCHS " == *" x86_64 "* ]]
BUNDLE="dist/KeystoneOFX.ofx.bundle"
mkdir -p "$BUNDLE/Contents/MacOS" "$BUNDLE/Contents/Resources"
cp "$BIN" "$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"
cp src/Info.plist.in "$BUNDLE/Contents/Info.plist"
cp resources/Keystone_Output_LogC4_to_Rec709.cube "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube" | awk '{print $1}')" = "9bd910e505f4f8fdfef67f97b85b7127f80528c4fda99e87ebb25eaa985c6d54"
plutil -lint "$BUNDLE/Contents/Info.plist"
SYMS="$(nm -gU "$BUNDLE/Contents/MacOS/KeystoneOFX.ofx")"
printf '%s\n' "$SYMS" | grep -q '_OfxGetNumberOfPlugins'
printf '%s\n' "$SYMS" | grep -q '_OfxGetPlugin'
if otool -L "$BUNDLE/Contents/MacOS/KeystoneOFX.ofx" | grep -E '/opt/homebrew|/usr/local/opt'; then echo 'Unexpected Homebrew dependency' >&2; exit 1; fi
codesign --force --sign - "$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"
codesign --force --sign - "$BUNDLE"
codesign --verify --deep --strict "$BUNDLE"
bash ci/validate_bundle_macos.sh "$BUNDLE"
ditto --keepParent -c -k --sequesterRsrc --zlibCompressionLevel 9 "$BUNDLE" dist/KeystoneOFX-macOS-universal.zip
test -s dist/KeystoneOFX-macOS-universal.zip
