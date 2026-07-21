#!/usr/bin/env bash
set -euo pipefail

if (( EUID != 0 )); then
    echo "Run this helper as root: sudo $0" >&2
    exit 1
fi

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
project_root=$(cd -- "${script_dir}/../.." && pwd)
temporary_dir=$(mktemp -d)
cleanup() {
    rm -rf -- "${temporary_dir}"
}
trap cleanup EXIT

source_rule="${temporary_dir}/70-msi-keyboard.rules"
generated_header="${temporary_dir}/DeviceDefinitionsData.h"
target_rule="/etc/udev/rules.d/70-msi-keyboard.rules"
cmake \
    "-DMSI_KEYBOARD_DEVICE_CONFIG=${project_root}/config/devices.json" \
    "-DMSI_KEYBOARD_GENERATED_HEADER=${generated_header}" \
    "-DMSI_KEYBOARD_UDEV_OUTPUT=${source_rule}" \
    -P "${project_root}/cmake/GenerateUdevRules.cmake"

install -D -m 0644 "${source_rule}" "${target_rule}"
rm -f \
    /etc/udev/rules.d/70-msi-strike-pro.rules \
    /etc/udev/rules.d/99-msi-strike-pro.rules

udevadm control --reload-rules
udevadm trigger --action=add --subsystem-match=hidraw
udevadm settle

echo "Installed ${target_rule} from config/devices.json"
echo "Run: ./build/release/msi-keyboard --cli --battery"
