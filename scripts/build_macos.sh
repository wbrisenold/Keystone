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
cp resources/Referent_LogC4_to_Rec709.cube "$BUNDLE/Contents/Resources/Referent_LogC4_to_Rec709.cube"
mkdir -p "$BUNDLE/Contents/Resources/SceneModel"
cp resources/SceneModel/ade20k.param "$BUNDLE/Contents/Resources/SceneModel/ade20k.param"
cp resources/SceneModel/ade20k.bin "$BUNDLE/Contents/Resources/SceneModel/ade20k.bin"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/SceneModel/ade20k.bin" | awk '{print $1}')" = "908a6785debbca3502ce11a08462a00504f4718e86c326042fd6b076acf84790"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/SceneModel/ade20k.param" | awk '{print $1}')" = "ba8e532e6357899f7f1fd8deeaf75322ce3f063487602ae32338a415bde569da"
mkdir -p "$BUNDLE/Contents/Resources/Licenses"
cp vendor/licenses/ADE20K-model-Apache-2.0.txt "$BUNDLE/Contents/Resources/Licenses/"
cp vendor/licenses/ncnn-BSD-3-Clause.txt "$BUNDLE/Contents/Resources/Licenses/"
test "$(shasum -a 256 "$BUNDLE/Contents/Resources/Referent_LogC4_to_Rec709.cube" | awk '{print $1}')" = "19b2feb5ed8cb767d980e9f9b351b6e1823a3990974277fdb4a46d1f709d251c"
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX"; if [ -f "$BIN" ]; then mv "$BIN" "$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"; fi
BIN="$BUNDLE/Contents/MacOS/KeystoneOFX.ofx"; test -f "$BIN"
PLIST="$BUNDLE/Contents/Info.plist"; test -f "$PLIST"; test "$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$PLIST")" = "KeystoneOFX.ofx"; plutil -lint "$PLIST"
ARCHS="$(lipo -archs "$BIN")"; case " $ARCHS " in *" arm64 "*) ;; *) echo 'Missing arm64' >&2; exit 1;; esac; case " $ARCHS " in *" x86_64 "*) ;; *) echo 'Missing x86_64' >&2; exit 1;; esac
SYMS="$(nm -gU "$BIN")"; printf '%s\n' "$SYMS" | grep -q '_OfxGetNumberOfPlugins'; printf '%s\n' "$SYMS" | grep -q '_OfxGetPlugin'; printf '%s\n' "$SYMS" | grep -q '_OfxSetHost'
if otool -L "$BIN" | grep -E '/opt/homebrew|/usr/local/opt'; then echo 'Unexpected Homebrew runtime dependency' >&2; exit 1; fi
# LOAD-SAFE CONTRACT: the main Resolve plugin must never have a hard dependency on the optional
# inference runtime. Scene analysis is a sidecar opened only after Analyze Scene is pressed.
if otool -L "$BIN" | grep -Ei 'KeystoneSceneEngine|ncnn'; then echo 'Main OFX unexpectedly links scene inference runtime' >&2; exit 1; fi
SCENE="$BUNDLE/Contents/Resources/KeystoneSceneEngine.dylib"; test -f "$SCENE"
SARCHS="$(lipo -archs "$SCENE")"; case " $SARCHS " in *" arm64 "*) ;; *) echo 'Scene engine missing arm64' >&2; exit 1;; esac; case " $SARCHS " in *" x86_64 "*) ;; *) echo 'Scene engine missing x86_64' >&2; exit 1;; esac
nm -gU "$SCENE" | grep -q '_KeystoneSceneSegment'
if otool -L "$SCENE" | grep -E '/opt/homebrew|/usr/local/opt'; then echo 'Unexpected Homebrew dependency in scene sidecar' >&2; exit 1; fi
codesign --force --sign - "$SCENE"
codesign --force --deep --sign - "$BUNDLE"; codesign --verify --deep --strict "$BUNDLE"
mkdir -p dist; ditto --keepParent -c -k --sequesterRsrc --zlibCompressionLevel 9 "$BUNDLE" dist/KeystoneOFX-macOS-universal.zip
test -s dist/KeystoneOFX-macOS-universal.zip
