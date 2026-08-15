#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$(find "$ROOT/dctl" -maxdepth 1 -type f -name '*.dctl' -print -quit)"
test -f "$SRC"
DEST="$HOME/Library/Application Support/Blackmagic Design/DaVinci Resolve/Support/LUT/Luma Color System/Keystone"
mkdir -p "$DEST"
cp "$SRC" "$DEST/$(basename "$SRC")"
echo "Installed $(basename "$SRC")"
echo "Destination: $DEST"
echo "Refresh Resolve's LUT/DCTL list or restart Resolve."
