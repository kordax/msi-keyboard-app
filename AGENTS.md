# Repository instructions

## Legacy MSI lighting research

- Treat the Debian and Ubuntu `msi-keyboard` package as a protocol research reference only. Its historical upstream is `bparker06/msi-keyboard`; when that repository is unavailable, use the maintained Debian source package at `https://sources.debian.org/src/msi-keyboard/`.
- The legacy tool targets USB `1770:ff00`, not the MSI Strike Pro IDs configured here (`0db0:b231` over USB and `0db0:1620` through the 2.4 GHz receiver).
- Its 8-byte HID feature reports are useful hypotheses for investigation: mode uses `01 02 41 <mode> 00 00 00 ec`; regional color uses `01 02 42 <region> <color> <intensity> 00 ec`.
- Never send those legacy write reports to a supported device based only on brand similarity. Require device-specific USB captures or controlled read-only identification first, then an explicit guarded test for the exact PID and interface.
- Keep future lighting protocols device-specific and data-driven, alongside the existing device definitions. A device without a verified lighting protocol must remain read-only.
- Preserve the legacy project's BSD-3-Clause attribution if implementation code is copied rather than independently reimplemented from observed protocol behavior.

## Packaging

- Do not publish this project under the binary package name `msi-keyboard`; Ubuntu and Debian already use that name for the unrelated legacy lighting tool. Use a distinct package name such as `msi-keyboard-app` and provide an explicit migration from releases that used the conflicting name.
