# Packaging

The application, desktop entry, and udev rule are regular CMake install
artifacts. A distribution package should install them under `/usr`, not
`/usr/local`.

The build dependencies include CMake 3.24+, a C++20 compiler, Qt 6.4
development, Linguist tools and image format plugins, `dpkg-dev`, `file`, and
`rpm`.

## Packages

Build, test, run the smoke check, and create both package formats:

```bash
task package
```

The packages are written to `build/release`.

## Staged install

Use `DESTDIR` to inspect the package payload without changing the host:

```bash
task build
DESTDIR="$PWD/package-root" cmake --install build/release --prefix /usr
```

The resulting payload includes:

```text
usr/bin/msi-keyboard
usr/share/applications/io.github.kordax.MsiKeyboard.desktop
usr/share/doc/msi-keyboard/copyright
usr/share/licenses/msi-keyboard/LICENSE
usr/lib/udev/rules.d/70-msi-strike-pro.rules
```

The udev rule always uses `usr/lib/udev/rules.d`, independently of the
multiarch library directory.

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
