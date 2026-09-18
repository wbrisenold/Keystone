#!/usr/bin/env bash
set -euo pipefail
BUNDLE="${1:-KeystoneOFX.ofx.bundle}"
test -d "$BUNDLE" || { echo "Usage: $0 /path/to/KeystoneOFX.ofx.bundle" >&2; exit 2; }
sudo mkdir -p /Library/OFX/Plugins
sudo rm -rf /Library/OFX/Plugins/KeystoneOFX.ofx.bundle
sudo cp -R "$BUNDLE" /Library/OFX/Plugins/
echo "Installed /Library/OFX/Plugins/KeystoneOFX.ofx.bundle — restart DaVinci Resolve."
