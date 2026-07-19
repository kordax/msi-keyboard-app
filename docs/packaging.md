# Packaging

The application, desktop entry, and udev rule are regular CMake install
artifacts. A distribution package should install them under `/usr`, not
`/usr/local`.

The build dependencies include CMake 3.24+, a C++20 compiler, and Qt 6.4
development files.

## Staged install

Use `DESTDIR` to inspect the package payload without changing the host:

```bash
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build-release
DESTDIR="$PWD/package-root" cmake --install build-release
```

The resulting payload includes:

```text
usr/bin/msi-keyboard
usr/share/applications/io.github.kordax.MsiKeyboard.desktop
usr/share/licenses/msi-keyboard/LICENSE
usr/lib/udev/rules.d/70-msi-strike-pro.rules
```

The udev rule always uses `usr/lib/udev/rules.d`, independently of the
multiarch library directory.

## Debian package

For a debhelper package, configure CMake with `CMAKE_INSTALL_PREFIX=/usr` and
let `dh_auto_install` stage the files. The udev rule is already included by
CMake.

An alternative is to omit the CMake udev component and provide the same rule
as `debian/msi-strike-pro.udev`. `dh_installudev` then installs it under
`usr/lib/udev/rules.d` and adds the required maintainer-script integration.
Do not use both approaches in one package.

The rule must sort before `73-seat-late.rules`. That file runs the `uaccess`
builtin, so this project uses priority `70`. A rule named `99-…` adds the tag
too late and leaves the active desktop user without an ACL.

Files under `/etc/udev/rules.d` are reserved for local administrator overrides.
The development helper uses `/etc`, while a `.deb` should use `/usr/lib`.

## Development helper

Install or migrate the local rule:

```bash
sudo ./scripts/linux/install-udev-rule.sh
```

Remove local development rules:

```bash
sudo ./scripts/linux/uninstall-udev-rule.sh
```

The application itself must always run as the desktop user, never as root.
