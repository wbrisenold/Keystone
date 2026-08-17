#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$(find "$ROOT/dctl" -maxdepth 1 -type f -name '*.dctl' -print -quit)"
test -f "$SRC"
DEST="$HOME/Library/Application Support/Blackmagic Design/DaVinci Resolve/Support/LUT/Luma Color System/Keystone"
mkdir -p "$DEST"
# Remove only older Keystone DCTL releases in Keystone's own install folder so Resolve
# does not expose multiple Keystone RC releases side by side after an upgrade.
find "$DEST" -maxdepth 1 -type f -name 'Keystone*.dctl' -delete
cp "$SRC" "$DEST/$(basename "$SRC")"
echo "Installed $(basename "$SRC")"
echo "Destination: $DEST"
echo "Refresh Resolve's LUT/DCTL list or restart Resolve."
