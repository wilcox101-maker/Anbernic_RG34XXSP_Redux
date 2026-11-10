#!/bin/sh
# Deploy the Redux frontend to the RG34XXSP.
#
# DANGER: writes to the device. Run only after Dale has reviewed.
# Assumes the device is reachable over SSH and the binary is already built.
#
# Usage:
#   ./tools/deploy_frontend.sh <device-ip> [binary-path]
#
# It copies the binary to /mnt/mmc/Roms/APPS/ and prints the command to
# swap it in as the launcher. It does not restart dmenu_ln itself.

set -e

IP="${1:?device IP required}"
BIN="${2:-frontend/build/rg34xxsp-redux}"

if [ ! -f "$BIN" ]; then
    echo "binary not found: $BIN" >&2
    exit 1
fi

echo "Copying $BIN to root@$IP:/mnt/mmc/Roms/APPS/"
scp "$BIN" "root@$IP:/mnt/mmc/Roms/APPS/rg34xxsp-redux"

echo
echo "On the device, to use it as the launcher (manual):"
echo "  mv /mnt/vendor/ctrl/dmenu_ln /mnt/vendor/ctrl/dmenu_ln.bak"
echo "  echo '#!/bin/sh' > /mnt/vendor/ctrl/dmenu_ln"
echo "  echo 'exec /mnt/mmc/Roms/APPS/rg34xxsp-redux' >> /mnt/vendor/ctrl/dmenu_ln"
echo "  chmod +x /mnt/vendor/ctrl/dmenu_ln"
echo
echo "To restore stock:"
echo "  mv /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln"
