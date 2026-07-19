#!/usr/bin/env bash
set -euo pipefail

readonly actionlint_version="1.7.12"
readonly gitleaks_version="8.30.1"
readonly tools_dir="${TOOLS_DIR:-.cache/tools}"
readonly actionlint_path="${tools_dir}/actionlint-${actionlint_version}"
readonly gitleaks_path="${tools_dir}/gitleaks-${gitleaks_version}"

case "$(uname -m)" in
    x86_64 | amd64)
        readonly actionlint_arch="linux_amd64"
        readonly actionlint_sha256="8aca8db96f1b94770f1b0d72b6dddcb1ebb8123cb3712530b08cc387b349a3d8"
        readonly gitleaks_arch="linux_x64"
        readonly gitleaks_sha256="551f6fc83ea457d62a0d98237cbad105af8d557003051f41f3e7ca7b3f2470eb"
        ;;
    aarch64 | arm64)
        readonly actionlint_arch="linux_arm64"
        readonly actionlint_sha256="325e971b6ba9bfa504672e29be93c24981eeb1c07576d730e9f7c8805afff0c6"
        readonly gitleaks_arch="linux_arm64"
        readonly gitleaks_sha256="e4a487ee7ccd7d3a7f7ec08657610aa3606637dab924210b3aee62570fb4b080"
        ;;
    *)
        echo "Unsupported architecture: $(uname -m)" >&2
        exit 1
        ;;
esac

if [[ -x "${actionlint_path}" && -x "${gitleaks_path}" ]]; then
    exit 0
fi

temp_dir="$(mktemp -d)"
readonly temp_dir
trap 'rm -rf "${temp_dir}"' EXIT
mkdir -p "${tools_dir}"

install_release_binary() {
    local archive_url="$1"
    local expected_sha256="$2"
    local member="$3"
    local destination="$4"
    local archive="${temp_dir}/${member}.tar.gz"

    curl --proto '=https' --tlsv1.2 --fail --silent --show-error --location \
        --retry 3 --output "${archive}" "${archive_url}"
    printf '%s  %s\n' "${expected_sha256}" "${archive}" \
        | sha256sum --check --strict -
    tar -xzf "${archive}" -C "${temp_dir}" "${member}"
    install -m 0755 "${temp_dir}/${member}" "${destination}"
}

if [[ ! -x "${actionlint_path}" ]]; then
    install_release_binary \
        "https://github.com/rhysd/actionlint/releases/download/v${actionlint_version}/actionlint_${actionlint_version}_${actionlint_arch}.tar.gz" \
        "${actionlint_sha256}" \
        actionlint \
        "${actionlint_path}"
fi

if [[ ! -x "${gitleaks_path}" ]]; then
    install_release_binary \
        "https://github.com/gitleaks/gitleaks/releases/download/v${gitleaks_version}/gitleaks_${gitleaks_version}_${gitleaks_arch}.tar.gz" \
        "${gitleaks_sha256}" \
        gitleaks \
        "${gitleaks_path}"
fi
