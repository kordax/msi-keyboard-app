# Supported device catalog

Every supported keyboard is declared in `config/devices.json`. This is the
single source for device detection, USB and dongle grouping, and generated
udev permissions.

A device record contains:

- `id`: stable internal model ID used in logical device IDs;
- `name`: name shown in the interface and CLI;
- `vendor_id`: four hexadecimal USB vendor digits;
- `usb_product_id`: product ID used by a direct USB connection;
- `dongle_product_id`: receiver product ID, or an empty string when absent;
- `artwork`: Qt resource path shown in the device panel and hover popover;
- `battery_protocol`: implemented protocol name, or an empty string;
- `battery_interface`: HID interface used for battery requests, or `-1`.

To add a keyboard that uses an existing battery protocol, add one JSON record
and its artwork resource. CMake validates IDs, rejects duplicate USB pairs,
and regenerates both the C++ definitions and `70-msi-keyboard.rules` whenever
the catalog changes.

A keyboard with a different battery command or report layout also needs a new
protocol implementation. Keep `battery_protocol` empty and
`battery_interface` set to `-1` until that decoder is verified.
