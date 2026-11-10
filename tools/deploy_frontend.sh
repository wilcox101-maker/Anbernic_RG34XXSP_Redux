#!/bin/sh
# Deploy the Redux frontend to the RG34XXSP.
#
# DANGER: writes to the device. Run only after Dale has reviewed.
# Writes only to appfs (/mnt/vendor). Never touches the Roms partition.
#
# Usage:
#   ./tools/deploy_frontend.sh <device-ip> [binary-path]
#   ./tools/deploy_frontend.sh --restore <device-ip>
#
# The binary must be an aarch64 build with -DREDUX_PLATFORM=stock.
# The script refuses anything else.

set -e

RESTORE=0
if [ "$1" = "--restore" ]; then
    RESTORE=1
    shift
fi

IP="${1:?device IP required}"
BIN="${2:-frontend/build/rg34xxsp-redux}"

if [ "$RESTORE" = 1 ]; then
    echo "Restoring stock dmenu_ln on $IP"
    ssh "root@$IP" 'test -f /mnt/vendor/ctrl/dmenu_ln.bak && mv /mnt/vendor/ctrl/dmenu_ln.bak /mnt/vendor/ctrl/dmenu_ln || echo "no backup found"'
    exit 0
fi

if [ ! -f "$BIN" ]; then
    echo "binary not found: $BIN" >&2
    exit 1
fi

if ! file "$BIN" | grep -q aarch64; then
    echo "refusing non-aarch64 binary: $BIN" >&2
    echo "build on the device or with an aarch64 toolchain + -DREDUX_PLATFORM=stock" >&2
    exit 1
fi

echo "Copying $BIN to root@$IP:/mnt/vendor/redux/"
ssh "root@$IP" 'mkdir -p /mnt/vendor/redux'
scp "$BIN" "root@$IP:/mnt/vendor/redux/rg34xxsp-redux"

echo
echo "On the device, after reading /mnt/vendor/ctrl/dmenu_ln:"
echo "  cp /mnt/vendor/ctrl/dmenu_ln /mnt/vendor/ctrl/dmenu_ln.bak"
echo "  # edit the wrapper to call /mnt/vendor/redux/rg34xxsp-redux"
echo "  # do not use exec if pwr_new.sh kills by process name"
echo
echo "To restore: ./tools/deploy_frontend.sh --restore $IP"
