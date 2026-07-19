#!/usr/bin/env bash
set -euo pipefail

if (( EUID != 0 )); then
    echo "Run this helper as root: sudo $0" >&2
    exit 1
fi

rm -f \
    /etc/udev/rules.d/70-msi-strike-pro.rules \
    /etc/udev/rules.d/99-msi-strike-pro.rules

udevadm control --reload-rules
udevadm trigger --action=add --subsystem-match=hidraw
udevadm settle

echo "Removed MSI Strike Pro udev rules"
