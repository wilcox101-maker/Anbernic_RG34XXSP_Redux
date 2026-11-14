#!/bin/sh
# Redux installer, device side (Dale's decision A, 2026-11-14:
# notes/2026-11-14-dale-decision-installer-a.md).
#
# Runs on the RG34XXSP as root. tools/install.py sends it over one SSH session (`sh -s`),
# prefixed with REDUX_MODE, REDUX_SHA256, REDUX_SIZE and payload functions: redux_payload (the
# new dmenu.bin), redux_payload_run (installer/run.sh, the start wrapper that dmenu_ln runs) and,
# optionally, redux_payload_logo (the boot logo).
# The TF2 package (option B, later) will call this same script with REDUX_SRC instead.
#
# Modes (REDUX_MODE): check | install | uninstall. Default: check (writes nothing).
#
# Writes only: /mnt/vendor/redux/ (appfs: dmenu.bin, run.sh, bootlogo.bmp), /mnt/vendor/ctrl/dmenu_ln + its one backup
# dmenu_ln.bak (appfs), /mnt/data/redux/install.log (UDISK). Never the Roms card, never the
# boot partition, never formats anything. Every write goes to a temporary name first and is
# renamed into place, so a cut-off run leaves the old file working.
#
# REDUX_ROOT (default empty) prefixes every path; the tests use it to run against a fake tree.

set -u

R="${REDUX_ROOT:-}"
MODE="${REDUX_MODE:-check}"
VENDOR="$R/mnt/vendor"
LN="$VENDOR/ctrl/dmenu_ln"
BAK="$LN.bak"
DIR="$VENDOR/redux"
BIN="$DIR/dmenu.bin"
RUN="$DIR/run.sh"
LOGO="$DIR/bootlogo.bmp"
DATA="$R/mnt/data/redux"
STOCK_CMD='CMD="/mnt/vendor/bin/dmenu.bin"'
REDUX_CMD='CMD="/mnt/vendor/redux/run.sh"'
OLD_CMD='CMD="/mnt/vendor/redux/dmenu.bin"'     # a hand deploy before the installer (deploy_frontend.py)
LOGO_BYTES=921654

say()  { echo "redux-install: $*"; }
fail() { echo "redux-install: STOPPED: $*" >&2; exit 1; }
log()  { mkdir -p "$DATA" 2>/dev/null && echo "$(date '+%Y-%m-%d %H:%M:%S') $MODE $*" >> "$DATA/install.log"; }
have() { command -v "$1" >/dev/null 2>&1; }

# stock | redux | redux-old | unknown | missing, for a dmenu_ln-like file ($1).
# redux = CMD is run.sh (what this installer sets); redux-old = CMD is dmenu.bin directly.
ln_state() {
    [ -f "$1" ] || { echo missing; return; }
    n=$(grep -c '^CMD=' "$1")
    if [ "$n" != 1 ]; then echo unknown
    elif grep -qx "$STOCK_CMD" "$1"; then echo stock
    elif grep -qx "$REDUX_CMD" "$1"; then echo redux
    elif grep -qx "$OLD_CMD" "$1"; then echo redux-old
    else echo unknown; fi
}

sha_of() { sha256sum "$1" | cut -d' ' -f1; }
free_kb() { df -Pk "$1" | awk 'NR==2 {print $4}'; }

do_check() {
    st=$(ln_state "$LN")
    say "launcher (dmenu_ln): $st"
    if [ -f "$BAK" ]; then say "backup dmenu_ln.bak: present ($(ln_state "$BAK"))"
    else say "backup dmenu_ln.bak: none"; fi
    if [ -f "$BIN" ]; then say "Redux dmenu.bin: present, sha256 $(sha_of "$BIN")"
    else say "Redux dmenu.bin: none"; fi
    if [ -f "$RUN" ]; then say "run.sh: present, sha256 $(sha_of "$RUN")"
    else say "run.sh: none"; fi
    [ -d "$VENDOR" ] && say "appfs free: $(free_kb "$VENDOR") KiB"
    for t in sha256sum base64 df sed grep; do have "$t" || say "missing tool: $t"; done
    case "$st" in
        stock)   say "ready to install" ;;
        redux|redux-old) say "Redux is installed; install again to upgrade, or uninstall" ;;
        missing) fail "no $LN: not a stock RG34XXSP card, or appfs is not mounted" ;;
        *)       fail "dmenu_ln has an unknown CMD= line (changed by hand or another firmware); nothing touched" ;;
    esac
}

# Write the payload ($1: function name or file) to $2, check size/hash.
write_payload() {
    src="$1"; out="$2"; want_sha="$3"; want_size="$4"
    rm -f "$out"
    if [ -f "$src" ]; then cat "$src" > "$out" || return 1
    else "$src" > "$out" || return 1; fi
    got_size=$(wc -c < "$out" | tr -d ' ')
    [ "$got_size" = "$want_size" ] || { rm -f "$out"; echo "size $got_size, expected $want_size"; return 1; }
    if [ -n "$want_sha" ]; then
        got=$(sha_of "$out")
        [ "$got" = "$want_sha" ] || { rm -f "$out"; echo "sha256 $got, expected $want_sha"; return 1; }
    fi
    return 0
}

do_install() {
    st=$(ln_state "$LN")
    case "$st" in
        stock|redux|redux-old) ;;
        missing) fail "no $LN: not a stock RG34XXSP card, or appfs is not mounted" ;;
        *) fail "dmenu_ln has an unknown CMD= line; nothing touched" ;;
    esac
    for t in sha256sum base64 df sed grep; do have "$t" || fail "missing tool on the device: $t"; done
    src="${REDUX_SRC:-redux_payload}"
    [ -f "$src" ] || have "$src" || fail "no Redux binary was sent"
    rsrc="${REDUX_SRC_RUN:-redux_payload_run}"
    [ -f "$rsrc" ] || have "$rsrc" || fail "no run.sh was sent"
    [ -n "${REDUX_RUN_SHA256:-}" ] && [ -n "${REDUX_RUN_SIZE:-}" ] || fail "no checksum or size for run.sh"
    [ -n "${REDUX_SHA256:-}" ] && [ -n "${REDUX_SIZE:-}" ] || fail "no checksum or size was sent"
    if [ "$st" != stock ] && [ ! -f "$BAK" ]; then
        fail "Redux is set in dmenu_ln but there is no dmenu_ln.bak, so uninstall could not restore stock; fix by hand first"
    fi

    need=$(( REDUX_SIZE / 1024 + 2048 ))
    mkdir -p "$DIR" || fail "cannot create $DIR"
    avail=$(free_kb "$DIR")
    [ "${avail:-0}" -ge "$need" ] || fail "appfs has ${avail:-0} KiB free, needs $need KiB"

    # 1. Binary: temporary name, checked, then renamed over the old one.
    msg=$(write_payload "$src" "$BIN.new" "$REDUX_SHA256" "$REDUX_SIZE") || fail "binary copy failed: $msg"
    chmod 755 "$BIN.new" && sync && mv -f "$BIN.new" "$BIN" || fail "cannot put dmenu.bin in place"
    say "dmenu.bin copied, sha256 $REDUX_SHA256"
    msg=$(write_payload "$rsrc" "$RUN.new" "$REDUX_RUN_SHA256" "$REDUX_RUN_SIZE") || fail "run.sh copy failed: $msg"
    chmod 755 "$RUN.new" && mv -f "$RUN.new" "$RUN" || fail "cannot put run.sh in place"
    say "run.sh copied"

    # 2. Optional boot logo (installed later on the handheld: Settings > System > Boot Screen).
    lsrc="${REDUX_SRC_LOGO:-redux_payload_logo}"
    if [ -f "$lsrc" ] || have "$lsrc"; then
        msg=$(write_payload "$lsrc" "$LOGO.new" "${REDUX_LOGO_SHA256:-}" "$LOGO_BYTES") || fail "boot logo copy failed: $msg"
        mv -f "$LOGO.new" "$LOGO" || fail "cannot put bootlogo.bmp in place"
        say "boot logo copied (install it on the handheld: Settings > System > Boot Screen)"
    fi

    # 3. Launcher switch: one backup, never overwritten; change only the one CMD= line
    #    (stock, or an older Redux deploy that ran dmenu.bin directly) to run.sh.
    if [ "$st" != redux ]; then
        if [ "$st" = stock ] && [ ! -f "$BAK" ]; then
            cp -p "$LN" "$BAK.tmp" && sync && mv "$BAK.tmp" "$BAK" || fail "cannot back up dmenu_ln"
            say "dmenu_ln backed up to dmenu_ln.bak"
        fi
        cp -p "$LN" "$LN.tmp" || fail "cannot copy dmenu_ln"
        sed "s|^CMD=.*\$|$REDUX_CMD|" "$LN" > "$LN.tmp" || fail "sed failed"
        [ "$(ln_state "$LN.tmp")" = redux ] || { rm -f "$LN.tmp"; fail "launcher edit did not read back; nothing switched"; }
        sync && mv -f "$LN.tmp" "$LN" || fail "cannot put dmenu_ln in place"
        say "launcher switched to Redux"
    else
        say "launcher already on Redux (upgrade)"
    fi
    sync
    [ "$(ln_state "$LN")" = redux ] || fail "dmenu_ln does not read back as Redux"
    log "ok sha256=$REDUX_SHA256 from=$st"
    say "done. Restart the handheld (Power Off, then on) to start Redux."
}

do_uninstall() {
    st=$(ln_state "$LN")
    case "$st" in
        stock) say "the launcher is already stock; nothing to do"; return 0 ;;
        redux|redux-old) ;;
        missing) fail "no $LN: not a stock RG34XXSP card, or appfs is not mounted" ;;
        *) fail "dmenu_ln has an unknown CMD= line; nothing touched" ;;
    esac
    [ -f "$BAK" ] || fail "no dmenu_ln.bak to restore"
    [ "$(ln_state "$BAK")" = stock ] || fail "dmenu_ln.bak is not the stock launcher; nothing touched"
    mv -f "$BAK" "$LN" || fail "cannot restore dmenu_ln"
    sync
    [ "$(ln_state "$LN")" = stock ] || fail "dmenu_ln does not read back as stock"
    log "ok restored stock launcher"
    say "stock launcher restored. Restart the handheld. $DIR is left in place (unused; delete it by hand if you like)."
    say "For the stock boot logo: Settings > System > Boot Screen > Restore, before restarting."
}

case "$MODE" in
    check)     do_check ;;
    install)   do_install ;;
    uninstall) do_uninstall ;;
    *)         fail "unknown mode: $MODE" ;;
esac
