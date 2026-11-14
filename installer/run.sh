#!/bin/sh
# Redux start wrapper: /mnt/vendor/redux/run.sh. Stock dmenu_ln runs it as CMD= (installed by
# tools/install.py). It replaces the hand-made run.sh on Dale's unit (2026-11-10), same job:
#   - starts dmenu.bin (Redux; the name keeps stock pwr_new.sh's `pidof dmenu.bin` check working),
#   - appends Redux's output to redux.log with a start and an exit line,
#   - exits with Redux's exit code: dmenu_ln runs /tmp/.next only after exit 0.
# New: redux.log is capped. Over 256 KiB at start, only its last 128 KiB are kept.
#
# REDUX_ROMS is not set here: Redux scans TF1 and TF2 by itself (#104). Set it in the
# environment only to override (one folder, or several separated by ':').
# REDUX_DIR (default /mnt/vendor/redux) is for the tests.

D="${REDUX_DIR:-/mnt/vendor/redux}"
LOG="$D/redux.log"
MAX_BYTES=262144
KEEP_BYTES=131072

if [ -f "$LOG" ]; then
    size=$(wc -c < "$LOG" 2>/dev/null | tr -d ' ')
    if [ "${size:-0}" -gt "$MAX_BYTES" ]; then
        tail -c "$KEEP_BYTES" "$LOG" > "$LOG.tmp" 2>/dev/null && mv -f "$LOG.tmp" "$LOG"
        echo "=== log trimmed to the last $KEEP_BYTES bytes" >> "$LOG"
    fi
fi

echo "=== start $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG"
"$D/dmenu.bin" "$@" >> "$LOG" 2>&1
rc=$?
echo "=== exit $rc $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG"
exit $rc
