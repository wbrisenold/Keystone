#!/usr/bin/env bash
set -euxo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"; cd "$ROOT"
command -v cmake >/dev/null; command -v clang++ >/dev/null; xcrun --sdk macosx --find metal >/dev/null; xcrun --sdk macosx --find metallib >/dev/null; command -v lipo >/dev/null
python3 ci/source_sanity.py
rm -rf build dist
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel "$(sysctl -n hw.ncpu)"
# Compile the same DCTL-derived math used by the CPU fallback as a Metal kernel.
xcrun --sdk macosx metal -O3 -Igenerated -Ishaders -c shaders/KeystoneKernels.metal -o build/KeystoneKernels.air
test -s build/KeystoneKernels.air
xcrun --sdk macosx metallib build/KeystoneKernels.air -o build/KeystoneKernels.metallib
test -s build/KeystoneKernels.metallib
BUNDLE="build/KeystoneOFX.ofx.bundle"; if [ ! -d "$BUNDLE" ]; then BUNDLE="$(find build -type d -name 'KeystoneOFX.ofx.bundle' -print -quit)"; fi
test -n "$BUNDLE"; test -d "$BUNDLE"
mkdir -p "$BUNDLE/Contents/Resources"
cp build/KeystoneKernels.metallib "$BUNDLE/Contents/Resources/KeystoneKernels.metallib"
cp resources/Keystone_Output_LogC4_to_Rec709.cube "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/Keystone_Output_LogC4_to_Rec709.cube" | awk '{print $1}')" = "9bd910e505f4f8fdfef67f97b85b7127f80528c4fda99e87ebb25eaa985c6d54"
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX"; if [ -f "$BIN" ]; then mv "$BIN" "$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"; fi
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"; test -f "$BIN"
PLIST="$BUNDLE/Contents/Info.plist"; test -f "$PLIST"; test "$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$PLIST")" = "KeystoneOFX.ofx"; plutil -lint "$PLIST"
ARCHS="$(lipo -archs "$BIN")"; case " $ARCHS " in *" arm64 "*) ;; *) echo 'Missing arm64' >&2; exit 1;; esac; case " $ARCHS " in *" x86_64 "*) ;; *) echo 'Missing x86_64' >&2; exit 1;; esac
SYMS="$(nm -gU "$BIN")"; printf '%s\n' "$SYMS" | grep -q '_OfxGetNumberOfPlugins'; printf '%s\n' "$SYMS" | grep -q '_OfxGetPlugin'; printf '%s\n' "$SYMS" | grep -q '_OfxSetHost'
if otool -L "$BIN" | grep -E '/opt/homebrew|/usr/local/opt'; then echo 'Unexpected Homebrew runtime dependency' >&2; exit 1; fi
codesign --force --deep --sign - "$BUNDLE"; codesign --verify --deep --strict "$BUNDLE"
mkdir -p dist; ditto --keepParent -c -k --sequesterRsrc --zlibCompressionLevel 9 "$BUNDLE" dist/KeystoneOFX-macOS-universal.zip
test -s dist/KeystoneOFX-macOS-universal.zip
