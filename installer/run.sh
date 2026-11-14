#!/bin/sh
# Redux start wrapper: /mnt/vendor/redux/run.sh. Stock dmenu_ln runs it as CMD= (installed by
# tools/install.py). It replaces the hand-made run.sh on Dale's unit (2026-11-10), same job:
#   - starts dmenu.bin (Redux; the name keeps stock pwr_new.sh's `pidof dmenu.bin` check working),
#   - appends Redux's output to redux.log with a start and an exit line,
#   - exits with Redux's exit code: dmenu_ln runs /tmp/.next only after exit 0.
# New: redux.log is capped. Over 256 KiB at start, only its last 128 KiB are kept.
# Start-failure guard (Dale 1A, 2026-11-14): a quick failure is an exit code other than 0 within
# 10 s of start. After 3 in a row, run.sh starts the stock menu (/mnt/vendor/bin/dmenu.bin) instead,
# for the rest of this boot. The counter and the switch live in /tmp (RAM), so the next boot tries
# Redux again. dmenu_ln is not changed. Exit 0 (a game or app chosen) always resets the counter.
#
# REDUX_ROMS is not set here: Redux scans TF1 and TF2 by itself (#104). Set it in the
# environment only to override (one folder, or several separated by ':').
# REDUX_DIR, REDUX_GUARD, REDUX_STOCK_MENU and REDUX_QUICK_SECS are for the tests.

D="${REDUX_DIR:-/mnt/vendor/redux}"
LOG="$D/redux.log"
MAX_BYTES=262144
KEEP_BYTES=131072
GUARD="${REDUX_GUARD:-/tmp/redux-start}"
STOCK="${REDUX_STOCK_MENU:-/mnt/vendor/bin/dmenu.bin}"
QUICK_SECS="${REDUX_QUICK_SECS:-10}"
MAX_FAILS=3

if [ -f "$LOG" ]; then
    size=$(wc -c < "$LOG" 2>/dev/null | tr -d ' ')
    if [ "${size:-0}" -gt "$MAX_BYTES" ]; then
        tail -c "$KEEP_BYTES" "$LOG" > "$LOG.tmp" 2>/dev/null && mv -f "$LOG.tmp" "$LOG"
        echo "=== log trimmed to the last $KEEP_BYTES bytes" >> "$LOG"
    fi
fi

if [ -f "$GUARD.stock" ]; then
    echo "=== stock menu $(date '+%Y-%m-%d %H:%M:%S') (Redux failed to start $MAX_FAILS times this boot)" >> "$LOG"
    exec "$STOCK" "$@"
fi

echo "=== start $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG"
t0=$(date +%s)
"$D/dmenu.bin" "$@" >> "$LOG" 2>&1
rc=$?
t1=$(date +%s)
echo "=== exit $rc $(date '+%Y-%m-%d %H:%M:%S')" >> "$LOG"

if [ "$rc" -ne 0 ] && [ $((t1 - t0)) -lt "$QUICK_SECS" ]; then
    n=$(cat "$GUARD.fails" 2>/dev/null)
    case "$n" in ''|*[!0-9]*) n=0 ;; esac
    n=$((n + 1))
    echo "$n" > "$GUARD.fails"
    if [ "$n" -ge "$MAX_FAILS" ]; then
        : > "$GUARD.stock"
        echo "=== $n quick failures in a row: stock menu for the rest of this boot" >> "$LOG"
        exec "$STOCK" "$@"
    fi
else
    rm -f "$GUARD.fails"
fi
exit $rc
