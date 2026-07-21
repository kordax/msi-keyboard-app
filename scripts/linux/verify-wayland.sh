#!/usr/bin/env bash
set -euo pipefail

binary="${1:?pass the desktop-environment-tests executable}"
if [[ ! -x "$binary" ]]; then
    echo "Wayland test executable is not runnable: $binary" >&2
    exit 1
fi
if ! command -v weston >/dev/null 2>&1; then
    echo "SKIP: Weston is not installed" >&2
    exit 77
fi

runtime_dir="$(mktemp -d "${TMPDIR:-/tmp}/msi-keyboard-wayland.XXXXXX")"
chmod 700 "$runtime_dir"
socket_name="msi-keyboard-wayland-$$"
weston_log="$runtime_dir/weston.log"
weston_pid=""

cleanup() {
    status=$?
    trap - EXIT INT TERM
    if [[ -n "$weston_pid" ]] && kill -0 "$weston_pid" 2>/dev/null; then
        kill "$weston_pid" 2>/dev/null || true
        wait "$weston_pid" 2>/dev/null || true
    fi
    rm -r -- "$runtime_dir"
    exit "$status"
}
trap cleanup EXIT INT TERM

weston_help="$(weston --help 2>&1 || true)"
weston_backend=(--backend=headless-backend.so)
if [[ "$weston_help" == *"-B, --backend"* ]]; then
    weston_backend=(-B headless)
fi

XDG_RUNTIME_DIR="$runtime_dir" \
    weston "${weston_backend[@]}" \
    --socket="$socket_name" \
    --idle-time=0 \
    --log="$weston_log" &
weston_pid=$!

for _ in {1..100}; do
    if [[ -S "$runtime_dir/$socket_name" ]]; then
        break
    fi
    if ! kill -0 "$weston_pid" 2>/dev/null; then
        echo "Weston exited before creating its Wayland socket" >&2
        sed -n '1,160p' "$weston_log" >&2
        exit 1
    fi
    sleep 0.05
done

if [[ ! -S "$runtime_dir/$socket_name" ]]; then
    echo "Weston did not become ready" >&2
    sed -n '1,160p' "$weston_log" >&2
    exit 1
fi

XDG_RUNTIME_DIR="$runtime_dir" \
WAYLAND_DISPLAY="$socket_name" \
QT_QPA_PLATFORM=wayland \
QT_WAYLAND_DISABLE_WINDOWDECORATION=1 \
    "$binary" rendersOnWayland
