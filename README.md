# MSI Keyboard for Linux

A Linux application to support several MSI keyboards.

<img src="assets/screenshots/msi-keyboard.png"
     alt="MSI Keyboard for Linux"
     width="100%">

## Supported devices

| Device | VID:PID | Connection |
| --- | --- | --- |
| MSI Strike Pro | `0db0:1620` | 2.4 GHz receiver |
| MSI Strike Pro | `0db0:b231` | USB cable |

## Usage

```bash
task           # Release build, tests, and smoke check
task debug     # Debug build and tests
task package   # DEB and RPM packages
task run       # GUI
task battery   # Battery level
task logs      # Continuous CLI logs
task json      # JSON
task udev      # Install the udev rule

build/release/msi-keyboard upgrade
build/release/msi-keyboard --language ru
```
