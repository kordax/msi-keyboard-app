#!/usr/bin/env bash
set -euo pipefail

if (( EUID != 0 )); then
    echo "Run this helper as root: sudo $0" >&2
    exit 1
fi

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
project_root=$(cd -- "${script_dir}/../.." && pwd)
source_rule="${project_root}/packaging/70-msi-strike-pro.rules"
target_rule="/etc/udev/rules.d/70-msi-strike-pro.rules"
legacy_rule="/etc/udev/rules.d/99-msi-strike-pro.rules"

install -D -m 0644 "${source_rule}" "${target_rule}"
rm -f "${legacy_rule}"

udevadm control --reload-rules
udevadm trigger --action=add --subsystem-match=hidraw
udevadm settle

echo "Installed ${target_rule}"
echo "Run: ./build/release/msi-keyboard --cli --battery"
