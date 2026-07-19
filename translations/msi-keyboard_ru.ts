<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="en_US">
<context>
    <name>BatteryDecoder</name>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="38"/>
        <source>Protocol profile not found: %1</source>
        <translation>Профиль протокола не найден: %1</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="50"/>
        <source>Invalid JSON: %1</source>
        <translation>Некорректный JSON: %1</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="71"/>
        <source>battery.source must be input or feature</source>
        <translation>battery.source должен быть input или feature</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="94"/>
        <source>battery.match_prefix_hex must contain pairs of hexadecimal digits</source>
        <translation>battery.match_prefix_hex должен содержать пары шестнадцатеричных цифр</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="104"/>
        <source>The profile does not define interface and report_id</source>
        <translation>В профиле не заданы interface и report_id</translation>
    </message>
</context>
<context>
    <name>CommandLine</name>
    <message>
        <location filename="../src/main.cpp" line="85"/>
        <source>Unsupported language &quot;%1&quot;. Use en or ru.</source>
        <translation>Неподдерживаемый язык «%1». Используйте en или ru.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="94"/>
        <source>Linux application for MSI keyboards</source>
        <translation>Приложение для клавиатур MSI в Linux</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="101"/>
        <source>Run without the GUI.</source>
        <translation>Запустить без графического интерфейса.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="104"/>
        <source>Continuously print HID events and diagnostics in CLI mode.</source>
        <translation>Непрерывно выводить события HID и диагностику в режиме CLI.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="109"/>
        <source>Take one read-only HID snapshot in CLI mode, then exit.</source>
        <translation>Снять один снимок HID только для чтения в режиме CLI и завершить работу.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="114"/>
        <source>Print newline-delimited JSON in CLI mode.</source>
        <translation>Выводить JSON с разделением строками в режиме CLI.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="119"/>
        <source>Query the battery once using the confirmed MSI Center command.</source>
        <translation>Один раз запросить заряд подтверждённой командой MSI Center.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="124"/>
        <source>Path to the battery decoder JSON profile.</source>
        <translation>Путь к JSON-профилю декодера батареи.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="127"/>
        <source>path</source>
        <translation>путь</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="130"/>
        <source>Use a language for this run. The default is English.</source>
        <translation>Использовать язык для этого запуска. По умолчанию используется английский.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="144"/>
        <source>Command to run: upgrade.</source>
        <translation>Команда для запуска: upgrade.</translation>
    </message>
</context>
<context>
    <name>msikeyboard::update</name>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="57"/>
        <source>Version is empty.</source>
        <translation>Версия не указана.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="72"/>
        <source>Version has an empty prerelease suffix.</source>
        <translation>У версии пустой суффикс предварительного выпуска.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="80"/>
        <source>Version has no numeric components.</source>
        <translation>В версии нет числовых компонентов.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="87"/>
        <source>Version component is not numeric: %1</source>
        <translation>Компонент версии не является числом: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="103"/>
        <source>Invalid prerelease identifier: %1</source>
        <translation>Некорректный идентификатор предварительного выпуска: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="368"/>
        <source>Invalid GitHub release JSON: %1</source>
        <translation>Некорректный JSON релиза GitHub: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="376"/>
        <source>GitHub release has no tag_name.</source>
        <translation>В релизе GitHub отсутствует tag_name.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="395"/>
        <source>GitHub release has no assets array.</source>
        <translation>В релизе GitHub отсутствует массив assets.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="401"/>
        <location filename="../src/update/UpdaterCore.cpp" line="411"/>
        <source>GitHub release contains an invalid asset.</source>
        <translation>Релиз GitHub содержит некорректный файл.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="446"/>
        <source>The release is not a stable published release.</source>
        <translation>Релиз не является опубликованным стабильным выпуском.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="454"/>
        <source>The release version is a prerelease.</source>
        <translation>Версия релиза является предварительной.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="589"/>
        <source>The platform or product is unsupported.</source>
        <translation>Платформа или продукт не поддерживается.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="631"/>
        <source>No package matches this platform.</source>
        <translation>Нет пакета для этой платформы.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="635"/>
        <source>More than one package matches this platform.</source>
        <translation>Для этой платформы найдено несколько пакетов.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="669"/>
        <source>The release has no SHA-256 checksum asset.</source>
        <translation>В релизе нет файла с контрольными суммами SHA-256.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="715"/>
        <source>The package metadata response is invalid.</source>
        <translation>Ответ с метаданными пакета некорректен.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="735"/>
        <source>The package name is %1, expected %2.</source>
        <translation>Имя пакета: %1, ожидалось: %2.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="745"/>
        <source>The package version is %1, expected %2.</source>
        <translation>Версия пакета: %1, ожидалась: %2.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="770"/>
        <source>The package architecture is %1, expected %2.</source>
        <translation>Архитектура пакета: %1, ожидалась: %2.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="809"/>
        <source>Invalid SHA-256 manifest line: %1</source>
        <translation>Некорректная строка списка SHA-256: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="814"/>
        <source>SHA-256 manifest contains an invalid file name.</source>
        <translation>Список SHA-256 содержит некорректное имя файла.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="821"/>
        <source>SHA-256 manifest has conflicting entries for %1.</source>
        <translation>Список SHA-256 содержит противоречивые записи для %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="829"/>
        <source>SHA-256 manifest is empty.</source>
        <translation>Список SHA-256 пуст.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="857"/>
        <source>SHA-256 manifest has ambiguous file names.</source>
        <translation>Список SHA-256 содержит неоднозначные имена файлов.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="868"/>
        <source>SHA-256 manifest has no entry for %1.</source>
        <translation>В списке SHA-256 нет записи для %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="879"/>
        <source>Expected SHA-256 checksum is invalid.</source>
        <translation>Ожидаемая контрольная сумма SHA-256 некорректна.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="910"/>
        <source>The package has not passed SHA-256 verification.</source>
        <translation>Пакет не прошёл проверку SHA-256.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="918"/>
        <source>No supported system package installer is available.</source>
        <translation>Поддерживаемый системный установщик пакетов недоступен.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="923"/>
        <source>Package path is not an absolute %1 file.</source>
        <translation>Путь к пакету не является абсолютным путём к файлу %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="938"/>
        <source>Unsupported system package installer: %1</source>
        <translation>Неподдерживаемый системный установщик пакетов: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="128"/>
        <source>The network request timed out.</source>
        <translation>Истекло время ожидания сетевого запроса.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="132"/>
        <source>The server response is unexpectedly large.</source>
        <translation>Ответ сервера оказался неожиданно большим.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="137"/>
        <location filename="../src/update/UpgradeRunner.cpp" line="235"/>
        <source>The server returned HTTP %1.</source>
        <translation>Сервер вернул HTTP %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="153"/>
        <source>The package size reported by GitHub is invalid.</source>
        <translation>GitHub сообщил некорректный размер пакета.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="225"/>
        <source>The package download timed out.</source>
        <translation>Истекло время ожидания загрузки пакета.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="231"/>
        <source>The downloaded package could not be written.</source>
        <translation>Не удалось записать загруженный пакет.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="229"/>
        <location filename="../src/update/UpgradeRunner.cpp" line="237"/>
        <source>The downloaded package size does not match the release metadata.</source>
        <translation>Размер загруженного пакета не совпадает с метаданными релиза.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="82"/>
        <source>The network request URL is not trusted.</source>
        <translation>Адрес сетевого запроса не является доверенным.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="130"/>
        <location filename="../src/update/UpgradeRunner.cpp" line="227"/>
        <source>The server redirected to an untrusted URL.</source>
        <translation>Сервер перенаправил запрос на недоверенный адрес.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="159"/>
        <source>The package download URL is not trusted.</source>
        <translation>Адрес загрузки пакета не является доверенным.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="351"/>
        <source>A required trusted system utility is not available for the upgrade.</source>
        <translation>Для обновления недоступна необходимая доверенная системная утилита.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="415"/>
        <source>A system utility returned unexpectedly large output.</source>
        <translation>Системная утилита вернула неожиданно большой объём данных.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="420"/>
        <source>A system utility process crashed.</source>
        <translation>Процесс системной утилиты аварийно завершился.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="428"/>
        <source>A system utility exited with status %1.</source>
        <translation>Системная утилита завершилась со статусом %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="514"/>
        <source>The protected package copy failed SHA-256 verification.</source>
        <translation>Защищённая копия пакета не прошла проверку SHA-256.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="565"/>
        <source>The following command will request administrator privileges:</source>
        <translation>Следующая команда запросит права администратора:</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="566"/>
        <source>The following package manager command will be run:</source>
        <translation>Будет выполнена следующая команда пакетного менеджера:</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="569"/>
        <source>Install the update? [y/N] </source>
        <translation>Установить обновление? [д/Н] </translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="583"/>
        <source>Checking GitHub for a stable update...</source>
        <translation>Проверка стабильного обновления на GitHub...</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="590"/>
        <source>This Linux distribution or CPU architecture is not supported.</source>
        <translation>Этот дистрибутив Linux или архитектура процессора не поддерживается.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="604"/>
        <source>No stable GitHub release has been published yet.</source>
        <translation>Стабильный релиз на GitHub ещё не опубликован.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="608"/>
        <source>Could not check for updates: %1</source>
        <translation>Не удалось проверить обновления: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="617"/>
        <source>Could not read the latest release: %1</source>
        <translation>Не удалось прочитать последний релиз: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="624"/>
        <source>Could not compare release versions: %1</source>
        <translation>Не удалось сравнить версии релизов: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="628"/>
        <source>MSI Keyboard %1 is already up to date.</source>
        <translation>MSI Keyboard %1 уже обновлён до последней версии.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="641"/>
        <source>No verified package is available for this system: %1</source>
        <translation>Для этой системы нет проверенного пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="647"/>
        <source>Update %1 is available.</source>
        <translation>Доступно обновление %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="648"/>
        <source>Downloading checksums...</source>
        <translation>Загрузка контрольных сумм...</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="657"/>
        <source>Could not download SHA-256 checksums: %1</source>
        <translation>Не удалось загрузить контрольные суммы SHA-256: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="665"/>
        <source>Could not parse SHA-256 checksums: %1</source>
        <translation>Не удалось разобрать контрольные суммы SHA-256: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="672"/>
        <source>Could not find the package checksum: %1</source>
        <translation>Не удалось найти контрольную сумму пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="681"/>
        <source>Could not create a temporary download directory.</source>
        <translation>Не удалось создать временный каталог для загрузки.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="687"/>
        <source>Downloading %1...</source>
        <translation>Загрузка %1...</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="696"/>
        <source>Could not download the package: %1</source>
        <translation>Не удалось загрузить пакет: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="704"/>
        <source>The downloaded package SHA-256 checksum does not match.</source>
        <translation>Контрольная сумма SHA-256 загруженного пакета не совпадает.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="706"/>
        <source>Package verification failed: %1</source>
        <translation>Проверка пакета не пройдена: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="709"/>
        <source>SHA-256 verification passed.</source>
        <translation>Проверка SHA-256 пройдена.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="717"/>
        <source>Could not prepare package installation: %1</source>
        <translation>Не удалось подготовить установку пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="726"/>
        <source>Could not prepare protected installation: %1</source>
        <translation>Не удалось подготовить защищённую установку: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="739"/>
        <source>Could not prepare protected installation.</source>
        <translation>Не удалось подготовить защищённую установку.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="767"/>
        <source>Could not create a protected package copy: %1</source>
        <translation>Не удалось создать защищённую копию пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="786"/>
        <source>Package metadata verification failed: %1</source>
        <translation>Проверка метаданных пакета не пройдена: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="791"/>
        <source>Protected package verification passed.</source>
        <translation>Защищённая копия пакета успешно проверена.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="748"/>
        <source>This copy is not managed by the system package manager; the package will be installed system-wide.</source>
        <translation>Эта копия не управляется системным пакетным менеджером; пакет будет установлен для всей системы.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="756"/>
        <source>Update cancelled.</source>
        <translation>Обновление отменено.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="804"/>
        <source>The package manager failed: %1</source>
        <translation>Ошибка пакетного менеджера: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="808"/>
        <source>MSI Keyboard was upgraded to %1.</source>
        <translation>MSI Keyboard обновлён до версии %1.</translation>
    </message>
</context>
<context>
    <name>strikepro::BatteryGauge</name>
    <message>
        <location filename="../src/gui/BatteryGauge.cpp" line="129"/>
        <source>BATTERY</source>
        <translation>БАТАРЕЯ</translation>
    </message>
</context>
<context>
    <name>strikepro::CliRunner</name>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="63"/>
        <source>Battery profile loaded</source>
        <translation>Профиль батареи загружен</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="70"/>
        <source>The battery decoder is not configured</source>
        <translation>Декодер батареи не настроен</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="82"/>
        <source>MSI Strike Pro 0db0:1620/b231 was not found</source>
        <translation>MSI Strike Pro 0db0:1620/b231 не найдена</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="116"/>
        <source>MSI Strike Pro found, interfaces %1</source>
        <translation>Найдена MSI Strike Pro, интерфейсы %1</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="125"/>
        <source>No access to vendor hidraw. Install the rule with scripts/linux/install-udev-rule.sh</source>
        <translation>Нет доступа к vendor hidraw. Установите правило через scripts/linux/install-udev-rule.sh</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="146"/>
        <source>Confirmed MSI Center battery query sent</source>
        <translation>Отправлен подтверждённый запрос батареи MSI Center</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="152"/>
        <source>The keyboard did not answer the battery query</source>
        <translation>Клавиатура не ответила на запрос батареи</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="182"/>
        <source>request=%1 response=%2</source>
        <translation>запрос=%1 ответ=%2</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="183"/>
        <source>id=%1</source>
        <translation>идентификатор=%1</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="187"/>
        <source>%1 if%2 %3 size=%4 data=%5</source>
        <translation>%1 интерфейс%2 %3 размер=%4 данные=%5</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="213"/>
        <source>Battery: %1%</source>
        <translation>Заряд: %1%</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="216"/>
        <source>, charging</source>
        <translation>, зарядка</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="217"/>
        <source>, on battery</source>
        <translation>, от батареи</translation>
    </message>
</context>
<context>
    <name>strikepro::DebugWindow</name>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="223"/>
        <source>Debug · MSI Keyboard</source>
        <translation>Отладка · MSI Keyboard</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="224"/>
        <source>Application logs</source>
        <translation>Логи приложения</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="225"/>
        <location filename="../src/gui/DebugWindow.cpp" line="232"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="226"/>
        <source>Logs</source>
        <translation>Логи</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="227"/>
        <source>HID telemetry</source>
        <translation>Телеметрия HID</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="229"/>
        <source>Raw reports and protocol research tools</source>
        <translation>Сырые отчёты и инструменты исследования протокола</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="230"/>
        <source>Take snapshot</source>
        <translation>Снять снимок</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="231"/>
        <source>Reload profile</source>
        <translation>Перечитать профиль</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="233"/>
        <source>Export JSON</source>
        <translation>Экспорт JSON</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="235"/>
        <source>Time</source>
        <translation>Время</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="235"/>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="235"/>
        <source>Report</source>
        <translation>Отчёт</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="235"/>
        <source>HEX data</source>
        <translation>HEX-данные</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="236"/>
        <source>Telemetry</source>
        <translation>Телеметрия</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="239"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="36"/>
        <source>INPUT</source>
        <translation>INPUT</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="40"/>
        <source>FEATURE</source>
        <translation>FEATURE</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="44"/>
        <source>OUTPUT</source>
        <translation>OUTPUT</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="48"/>
        <source>UNKNOWN</source>
        <translation>НЕИЗВЕСТНО</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="358"/>
        <source>Export diagnostics</source>
        <translation>Экспорт диагностики</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="360"/>
        <source>JSON (*.json)</source>
        <translation>JSON (*.json)</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="391"/>
        <source>Could not open the export file</source>
        <translation>Не удалось открыть файл экспорта</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="396"/>
        <source>Could not finish the export</source>
        <translation>Не удалось завершить экспорт</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="399"/>
        <source>Diagnostics exported: %1</source>
        <translation>Диагностика экспортирована: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="287"/>
        <source>%1:if%2  ·  req 0x%3 / resp 0x%4  ·  %5 B</source>
        <translation>%1:if%2  ·  запрос 0x%3 / ответ 0x%4  ·  %5 Б</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/gui/DebugWindow.cpp" line="247"/>
        <source>%n packet(s)</source>
        <translation>
            <numerusform>%n пакет</numerusform>
            <numerusform>%n пакета</numerusform>
            <numerusform>%n пакетов</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="293"/>
        <source>%1:if%2  ·  id 0x%3  ·  %4 B</source>
        <translation>%1:if%2  ·  id 0x%3  ·  %4 Б</translation>
    </message>
</context>
<context>
    <name>strikepro::HidMonitor</name>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="79"/>
        <source>HID interface 1 was not found</source>
        <translation>HID-интерфейс 1 не найден</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="86"/>
        <source>No access to interface 1 of device 0db0:%1. Reinstall the udev rule.</source>
        <translation>Нет доступа к интерфейсу 1 устройства 0db0:%1. Переустановите правило udev.</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="98"/>
        <source>No write access to %1: %2</source>
        <translation>Нет доступа на запись к %1: %2</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="122"/>
        <source>Could not send the battery query: %1</source>
        <translation>Не удалось отправить запрос батареи: %1</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="124"/>
        <source>Battery query was incomplete: %1 of %2 bytes</source>
        <translation>Запрос батареи отправлен не полностью: %1 из %2 байт</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="165"/>
        <source>Could not open %1: %2</source>
        <translation>Не удалось открыть %1: %2</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="217"/>
        <source>Read error on %1: %2</source>
        <translation>Ошибка чтения %1: %2</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="248"/>
        <source>HID interface %1 was not found</source>
        <translation>HID-интерфейс %1 не найден</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="259"/>
        <source>No access to %1. Install the udev rule from packaging.</source>
        <translation>Нет доступа к %1. Установите правило udev из packaging.</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="279"/>
        <source>%1 if%2 0x%3: %4</source>
        <translation>%1 if%2 0x%3: %4</translation>
    </message>
</context>
<context>
    <name>strikepro::MainWindow</name>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="501"/>
        <source>MSI Keyboard</source>
        <translation>MSI Keyboard</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="502"/>
        <source>Settings</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="503"/>
        <source>Language</source>
        <translation>Язык</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="504"/>
        <source>English</source>
        <translation>Английский</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="505"/>
        <source>Russian</source>
        <translation>Русский</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="506"/>
        <source>Debug</source>
        <translation>Отладка</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="507"/>
        <source>Logs</source>
        <translation>Логи</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="508"/>
        <source>Telemetry</source>
        <translation>Телеметрия</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="517"/>
        <source>MSI Keyboard manager for Linux</source>
        <translation>Менеджер клавиатур MSI для Linux</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="518"/>
        <source>BATTERY</source>
        <translation>АККУМУЛЯТОР</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="519"/>
        <source>READ ONLY</source>
        <translation>ТОЛЬКО ЧТЕНИЕ</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="520"/>
        <source>mode</source>
        <translation>режим</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="521"/>
        <source>DEVICE</source>
        <translation>УСТРОЙСТВО</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="522"/>
        <source>Connection</source>
        <translation>Подключение</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="565"/>
        <source>Battery profile unavailable</source>
        <translation>Профиль батареи недоступен</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="559"/>
        <source>Protocol profile loaded: %1</source>
        <translation>Профиль протокола загружен: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="566"/>
        <source>Profile error: %1</source>
        <translation>Ошибка профиля: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="587"/>
        <source>CONNECTED</source>
        <translation>ПОДКЛЮЧЕНА</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="587"/>
        <source>NOT CONNECTED</source>
        <translation>НЕ ПОДКЛЮЧЕНА</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="594"/>
        <source>MSI Strike Pro disconnected</source>
        <translation>MSI Strike Pro отключена</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="599"/>
        <source>No device</source>
        <translation>Нет устройства</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="601"/>
        <source>Waiting for a USB receiver or wired connection.</source>
        <translation>Ожидается USB-приёмник или проводное подключение.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="606"/>
        <source>Keyboard not detected</source>
        <translation>Клавиатура не обнаружена</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="613"/>
        <source>MSI Strike Pro · USB</source>
        <translation>MSI Strike Pro · USB</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="614"/>
        <source>MSI Strike Pro · 2.4 GHz</source>
        <translation>MSI Strike Pro · 2,4 ГГц</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="620"/>
        <source>Wired connection</source>
        <translation>Проводное подключение</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="621"/>
        <source>Wireless receiver</source>
        <translation>Беспроводной приёмник</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="626"/>
        <source>MSI Strike Pro detected over USB</source>
        <translation>MSI Strike Pro обнаружена через USB</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="627"/>
        <source>MSI Strike Pro detected through the 2.4 GHz receiver</source>
        <translation>MSI Strike Pro обнаружена через приёмник 2,4 ГГц</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="631"/>
        <location filename="../src/gui/MainWindow.cpp" line="666"/>
        <source>Reading battery…</source>
        <translation>Получение заряда…</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="632"/>
        <source>The first query runs automatically.</source>
        <translation>Первый запрос выполняется автоматически.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="639"/>
        <source>HID access required</source>
        <translation>Нужен доступ к HID</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="641"/>
        <source>The device was found, but the system denied battery access.</source>
        <translation>Устройство найдено, но система запретила чтение батареи.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="655"/>
        <source>Battery query: %1</source>
        <translation>Запрос батареи: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="657"/>
        <location filename="../src/gui/MainWindow.cpp" line="680"/>
        <source>No response</source>
        <translation>Нет ответа</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="678"/>
        <source>The keyboard did not answer the battery query</source>
        <translation>Клавиатура не ответила на запрос батареи</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="682"/>
        <source>The next attempt will run automatically.</source>
        <translation>Следующая попытка будет выполнена автоматически.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="710"/>
        <source>The keyboard is charging.</source>
        <translation>Клавиатура подключена к зарядке.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="711"/>
        <source>The keyboard is running on battery.</source>
        <translation>Клавиатура работает от аккумулятора.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="713"/>
        <source>Battery status received over USB HID.</source>
        <translation>Заряд получен через USB HID.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="716"/>
        <source>Battery updated: %1%</source>
        <translation>Заряд обновлён: %1%</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="299"/>
        <source>MSI STRIKE PRO</source>
        <translation>MSI STRIKE PRO</translation>
    </message>
</context>
</TS>
