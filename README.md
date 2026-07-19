# MSI Strike Pro Manager for Linux

Небольшой Qt 6 менеджер и безопасный инструмент reverse engineering для
`MSI STRIKE PRO WIRELESS Gaming Keyboard` (`0db0:1620`).

Сейчас приложение:

- находит клавиатуру по VID/PID и USB interface, не полагаясь на номер hidraw;
- показывает подключение и состояние доступа;
- пассивно слушает vendor HID input report;
- читает объявленные feature-report `0x07` и `0x0c`;
- выполняет read-only `GET_INPUT` и `GET_OUTPUT` для vendor-report;
- экспортирует обезличенные диагностические снимки;
- умеет показывать точный процент после настройки подтверждённого поля в
  JSON-профиле.

Поле батареи пока не угадано намеренно. В Linux заряд не опубликован через
`power_supply`, а публичной реализации протокола для этой модели не найдено.

## Сборка

Нужны CMake 3.24+, компилятор с C++20 и Qt 6.4+ с компонентами Widgets и Test.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/msi-strike-pro
```

## CLI без окна

Непрерывно следить за подключением и сырыми vendor HID-отчётами:

```bash
./build/msi-strike-pro --cli
```

Сделать один scan, прочитать доступные отчёты без записи в устройство и завершиться:

```bash
./build/msi-strike-pro --cli --once
```

Для машинной обработки доступен newline-delimited JSON:

```bash
./build/msi-strike-pro --cli --json
```

Коды возврата `--once`: `0` — снимок выполнен, `2` — устройство не найдено,
`3` — устройство найдено, но доступ к vendor hidraw закрыт.

## Доступ к HID

Правило ограничено одним VID/PID и выдаёт доступ активной desktop-сессии.
Development helper также заменяет старое правило с неправильным приоритетом:

```bash
sudo ./scripts/install-udev-rule.sh
```

Повторный add-trigger выполняется скриптом, поэтому обычно переподключение не
требуется. Откат:

```bash
sudo ./scripts/uninstall-udev-rule.sh
```

Приложение не нужно и не следует запускать от root.

Установка через CMake и будущий `.deb` описаны в
[`docs/packaging.md`](docs/packaging.md).

## Статус reverse engineering

Подтверждены четыре HID-интерфейса: стандартная клавиатура, двунаправленный
vendor-канал, feature-report `0x07/0x0c` и входной vendor-report `0x0d`.
Неизвестные output-команды не отправляются.

Пошаговый протокол исследования и формат профиля описаны в
[`docs/protocol.md`](docs/protocol.md).

План развития, включая English по умолчанию и опциональную русскую
локализацию, находится в [`ROADMAP.md`](ROADMAP.md).
