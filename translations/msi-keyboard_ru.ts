<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="en_US">
<context>
    <name>BatteryDecoder</name>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="37"/>
        <source>Protocol profile not found: %1</source>
        <translation>Профиль протокола не найден: %1</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="50"/>
        <source>Invalid JSON: %1</source>
        <translation>Некорректный JSON: %1</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="72"/>
        <source>battery.source must be input or feature</source>
        <translation>battery.source должен быть input или feature</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="100"/>
        <source>battery.match_prefix_hex must contain pairs of hexadecimal digits</source>
        <translation>battery.match_prefix_hex должен содержать пары шестнадцатеричных цифр</translation>
    </message>
    <message>
        <location filename="../src/device/BatteryDecoder.cpp" line="112"/>
        <source>The profile does not define interface and report_id</source>
        <translation>В профиле не заданы interface и report_id</translation>
    </message>
</context>
<context>
    <name>CommandLine</name>
    <message>
        <location filename="../src/main.cpp" line="81"/>
        <source>Unsupported language &quot;%1&quot;. Use en or ru.</source>
        <translation>Неподдерживаемый язык «%1». Используйте en или ru.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="90"/>
        <source>Linux application for MSI keyboards</source>
        <translation>Приложение для клавиатур MSI в Linux</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="97"/>
        <source>Run without the GUI.</source>
        <translation>Запустить без графического интерфейса.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="100"/>
        <source>Continuously print HID events and diagnostics in CLI mode.</source>
        <translation>Непрерывно выводить события HID и диагностику в режиме CLI.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="105"/>
        <source>Take one read-only HID snapshot in CLI mode, then exit.</source>
        <translation>Снять один снимок HID только для чтения в режиме CLI и завершить работу.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="110"/>
        <source>Print newline-delimited JSON in CLI mode.</source>
        <translation>Выводить JSON с разделением строками в режиме CLI.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="115"/>
        <source>Query the battery once using the confirmed MSI Center command.</source>
        <translation>Один раз запросить заряд подтверждённой командой MSI Center.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="120"/>
        <source>Path to the battery decoder JSON profile.</source>
        <translation>Путь к JSON-профилю декодера батареи.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="123"/>
        <source>path</source>
        <translation>путь</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="126"/>
        <source>Use a language for this run. The default is English.</source>
        <translation>Использовать язык для этого запуска. По умолчанию используется английский.</translation>
    </message>
    <message>
        <location filename="../src/main.cpp" line="140"/>
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
        <location filename="../src/update/UpdaterCore.cpp" line="81"/>
        <source>Version has no numeric components.</source>
        <translation>В версии нет числовых компонентов.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="90"/>
        <source>Version component is not numeric: %1</source>
        <translation>Компонент версии не является числом: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="106"/>
        <source>Invalid prerelease identifier: %1</source>
        <translation>Некорректный идентификатор предварительного выпуска: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="381"/>
        <source>Invalid GitHub release JSON: %1</source>
        <translation>Некорректный JSON релиза GitHub: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="390"/>
        <source>GitHub release has no tag_name.</source>
        <translation>В релизе GitHub отсутствует tag_name.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="409"/>
        <source>GitHub release has no assets array.</source>
        <translation>В релизе GitHub отсутствует массив assets.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="415"/>
        <location filename="../src/update/UpdaterCore.cpp" line="428"/>
        <source>GitHub release contains an invalid asset.</source>
        <translation>Релиз GitHub содержит некорректный файл.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="459"/>
        <source>The release is not a stable published release.</source>
        <translation>Релиз не является опубликованным стабильным выпуском.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="467"/>
        <source>The release version is a prerelease.</source>
        <translation>Версия релиза является предварительной.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="606"/>
        <source>The platform or product is unsupported.</source>
        <translation>Платформа или продукт не поддерживается.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="649"/>
        <source>No package matches this platform.</source>
        <translation>Нет пакета для этой платформы.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="653"/>
        <source>More than one package matches this platform.</source>
        <translation>Для этой платформы найдено несколько пакетов.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="685"/>
        <source>The release has no SHA-256 checksum asset.</source>
        <translation>В релизе нет файла с контрольными суммами SHA-256.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="731"/>
        <source>The package metadata response is invalid.</source>
        <translation>Ответ с метаданными пакета некорректен.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="751"/>
        <source>The package name is %1, expected %2.</source>
        <translation>Имя пакета: %1, ожидалось: %2.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="761"/>
        <source>The package version is %1, expected %2.</source>
        <translation>Версия пакета: %1, ожидалась: %2.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="784"/>
        <source>The package architecture is %1, expected %2.</source>
        <translation>Архитектура пакета: %1, ожидалась: %2.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="822"/>
        <source>Invalid SHA-256 manifest line: %1</source>
        <translation>Некорректная строка списка SHA-256: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="829"/>
        <source>SHA-256 manifest contains an invalid file name.</source>
        <translation>Список SHA-256 содержит некорректное имя файла.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="836"/>
        <source>SHA-256 manifest has conflicting entries for %1.</source>
        <translation>Список SHA-256 содержит противоречивые записи для %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="844"/>
        <source>SHA-256 manifest is empty.</source>
        <translation>Список SHA-256 пуст.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="871"/>
        <source>SHA-256 manifest has ambiguous file names.</source>
        <translation>Список SHA-256 содержит неоднозначные имена файлов.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="882"/>
        <source>SHA-256 manifest has no entry for %1.</source>
        <translation>В списке SHA-256 нет записи для %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="893"/>
        <source>Expected SHA-256 checksum is invalid.</source>
        <translation>Ожидаемая контрольная сумма SHA-256 некорректна.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="924"/>
        <source>The package has not passed SHA-256 verification.</source>
        <translation>Пакет не прошёл проверку SHA-256.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="934"/>
        <source>No supported system package installer is available.</source>
        <translation>Поддерживаемый системный установщик пакетов недоступен.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="941"/>
        <source>Package path is not an absolute %1 file.</source>
        <translation>Путь к пакету не является абсолютным путём к файлу %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpdaterCore.cpp" line="959"/>
        <source>Unsupported system package installer: %1</source>
        <translation>Неподдерживаемый системный установщик пакетов: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="127"/>
        <source>The network request timed out.</source>
        <translation>Истекло время ожидания сетевого запроса.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="131"/>
        <source>The server response is unexpectedly large.</source>
        <translation>Ответ сервера оказался неожиданно большим.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="136"/>
        <location filename="../src/update/UpgradeRunner.cpp" line="235"/>
        <source>The server returned HTTP %1.</source>
        <translation>Сервер вернул HTTP %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="152"/>
        <source>The package size reported by GitHub is invalid.</source>
        <translation>GitHub сообщил некорректный размер пакета.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="224"/>
        <source>The package download timed out.</source>
        <translation>Истекло время ожидания загрузки пакета.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="231"/>
        <source>The downloaded package could not be written.</source>
        <translation>Не удалось записать загруженный пакет.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="228"/>
        <location filename="../src/update/UpgradeRunner.cpp" line="237"/>
        <source>The downloaded package size does not match the release metadata.</source>
        <translation>Размер загруженного пакета не совпадает с метаданными релиза.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="81"/>
        <source>The network request URL is not trusted.</source>
        <translation>Адрес сетевого запроса не является доверенным.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="129"/>
        <location filename="../src/update/UpgradeRunner.cpp" line="226"/>
        <source>The server redirected to an untrusted URL.</source>
        <translation>Сервер перенаправил запрос на недоверенный адрес.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="158"/>
        <source>The package download URL is not trusted.</source>
        <translation>Адрес загрузки пакета не является доверенным.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="350"/>
        <source>A required trusted system utility is not available for the upgrade.</source>
        <translation>Для обновления недоступна необходимая доверенная системная утилита.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="411"/>
        <source>A system utility returned unexpectedly large output.</source>
        <translation>Системная утилита вернула неожиданно большой объём данных.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="416"/>
        <source>A system utility process crashed.</source>
        <translation>Процесс системной утилиты аварийно завершился.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="425"/>
        <source>A system utility exited with status %1.</source>
        <translation>Системная утилита завершилась со статусом %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="509"/>
        <source>The protected package copy failed SHA-256 verification.</source>
        <translation>Защищённая копия пакета не прошла проверку SHA-256.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="556"/>
        <source>The following command will request administrator privileges:</source>
        <translation>Следующая команда запросит права администратора:</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="558"/>
        <source>The following package manager command will be run:</source>
        <translation>Будет выполнена следующая команда пакетного менеджера:</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="561"/>
        <source>Install the update? [y/N] </source>
        <translation>Установить обновление? [д/Н] </translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="576"/>
        <source>Checking GitHub for a stable update...</source>
        <translation>Проверка стабильного обновления на GitHub...</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="583"/>
        <source>This Linux distribution or CPU architecture is not supported.</source>
        <translation>Этот дистрибутив Linux или архитектура процессора не поддерживается.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="598"/>
        <source>No stable GitHub release has been published yet.</source>
        <translation>Стабильный релиз на GitHub ещё не опубликован.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="604"/>
        <source>Could not check for updates: %1</source>
        <translation>Не удалось проверить обновления: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="612"/>
        <source>Could not read the latest release: %1</source>
        <translation>Не удалось прочитать последний релиз: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="620"/>
        <source>Could not compare release versions: %1</source>
        <translation>Не удалось сравнить версии релизов: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="626"/>
        <source>MSI Keyboard %1 is already up to date.</source>
        <translation>MSI Keyboard %1 уже обновлён до последней версии.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="638"/>
        <source>No verified package is available for this system: %1</source>
        <translation>Для этой системы нет проверенного пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="644"/>
        <source>Update %1 is available.</source>
        <translation>Доступно обновление %1.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="645"/>
        <source>Downloading checksums...</source>
        <translation>Загрузка контрольных сумм...</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="654"/>
        <source>Could not download SHA-256 checksums: %1</source>
        <translation>Не удалось загрузить контрольные суммы SHA-256: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="662"/>
        <source>Could not parse SHA-256 checksums: %1</source>
        <translation>Не удалось разобрать контрольные суммы SHA-256: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="669"/>
        <source>Could not find the package checksum: %1</source>
        <translation>Не удалось найти контрольную сумму пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="678"/>
        <source>Could not create a temporary download directory.</source>
        <translation>Не удалось создать временный каталог для загрузки.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="685"/>
        <source>Downloading %1...</source>
        <translation>Загрузка %1...</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="694"/>
        <source>Could not download the package: %1</source>
        <translation>Не удалось загрузить пакет: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="703"/>
        <source>The downloaded package SHA-256 checksum does not match.</source>
        <translation>Контрольная сумма SHA-256 загруженного пакета не совпадает.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="705"/>
        <source>Package verification failed: %1</source>
        <translation>Проверка пакета не пройдена: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="708"/>
        <source>SHA-256 verification passed.</source>
        <translation>Проверка SHA-256 пройдена.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="716"/>
        <source>Could not prepare package installation: %1</source>
        <translation>Не удалось подготовить установку пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="725"/>
        <source>Could not prepare protected installation: %1</source>
        <translation>Не удалось подготовить защищённую установку: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="738"/>
        <source>Could not prepare protected installation.</source>
        <translation>Не удалось подготовить защищённую установку.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="765"/>
        <source>Could not create a protected package copy: %1</source>
        <translation>Не удалось создать защищённую копию пакета: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="784"/>
        <source>Package metadata verification failed: %1</source>
        <translation>Проверка метаданных пакета не пройдена: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="789"/>
        <source>Protected package verification passed.</source>
        <translation>Защищённая копия пакета успешно проверена.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="747"/>
        <source>This copy is not managed by the system package manager; the package will be installed system-wide.</source>
        <translation>Эта копия не управляется системным пакетным менеджером; пакет будет установлен для всей системы.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="754"/>
        <source>Update cancelled.</source>
        <translation>Обновление отменено.</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="802"/>
        <source>The package manager failed: %1</source>
        <translation>Ошибка пакетного менеджера: %1</translation>
    </message>
    <message>
        <location filename="../src/update/UpgradeRunner.cpp" line="806"/>
        <source>MSI Keyboard was upgraded to %1.</source>
        <translation>MSI Keyboard обновлён до версии %1.</translation>
    </message>
</context>
<context>
    <name>strikepro::BatteryGauge</name>
    <message>
        <location filename="../src/gui/BatteryGauge.cpp" line="174"/>
        <source>BATTERY</source>
        <translation>БАТАРЕЯ</translation>
    </message>
</context>
<context>
    <name>strikepro::CliRunner</name>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="66"/>
        <source>Battery profile loaded</source>
        <translation>Профиль батареи загружен</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="72"/>
        <source>The battery decoder is not configured</source>
        <translation>Декодер батареи не настроен</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="83"/>
        <source>MSI Strike Pro 0db0:1620/b231 was not found</source>
        <translation>MSI Strike Pro 0db0:1620/b231 не найдена</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="118"/>
        <source>MSI Strike Pro found, interfaces %1</source>
        <translation>Найдена MSI Strike Pro, интерфейсы %1</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="125"/>
        <source>No access to vendor hidraw. Install the rule with scripts/linux/install-udev-rule.sh</source>
        <translation>Нет доступа к vendor hidraw. Установите правило через scripts/linux/install-udev-rule.sh</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="143"/>
        <source>Confirmed MSI Center battery query sent</source>
        <translation>Отправлен подтверждённый запрос батареи MSI Center</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="148"/>
        <source>The keyboard did not answer the battery query</source>
        <translation>Клавиатура не ответила на запрос батареи</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="180"/>
        <source>request=%1 response=%2</source>
        <translation>запрос=%1 ответ=%2</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="181"/>
        <source>id=%1</source>
        <translation>идентификатор=%1</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="184"/>
        <source>%1 if%2 %3 size=%4 data=%5</source>
        <translation>%1 интерфейс%2 %3 размер=%4 данные=%5</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="212"/>
        <source>Battery: %1%</source>
        <translation>Заряд: %1%</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="215"/>
        <source>, charging</source>
        <translation>, зарядка</translation>
    </message>
    <message>
        <location filename="../src/cli/CliRunner.cpp" line="215"/>
        <source>, on battery</source>
        <translation>, от батареи</translation>
    </message>
</context>
<context>
    <name>strikepro::DebugWindow</name>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="221"/>
        <source>Debug · MSI Keyboard</source>
        <translation>Отладка · MSI Keyboard</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="222"/>
        <source>Application logs</source>
        <translation>Логи приложения</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="223"/>
        <location filename="../src/gui/DebugWindow.cpp" line="229"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="224"/>
        <source>Logs</source>
        <translation>Логи</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="225"/>
        <source>HID telemetry</source>
        <translation>Телеметрия HID</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="226"/>
        <source>Raw reports and protocol research tools</source>
        <translation>Сырые отчёты и инструменты исследования протокола</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="227"/>
        <source>Take snapshot</source>
        <translation>Снять снимок</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="228"/>
        <source>Reload profile</source>
        <translation>Перечитать профиль</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="230"/>
        <source>Export JSON</source>
        <translation>Экспорт JSON</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="232"/>
        <source>Time</source>
        <translation>Время</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="232"/>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="232"/>
        <source>Report</source>
        <translation>Отчёт</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="232"/>
        <source>HEX data</source>
        <translation>HEX-данные</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="233"/>
        <source>Telemetry</source>
        <translation>Телеметрия</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="236"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="37"/>
        <source>INPUT</source>
        <translation>INPUT</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="39"/>
        <source>FEATURE</source>
        <translation>FEATURE</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="41"/>
        <source>OUTPUT</source>
        <translation>OUTPUT</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="43"/>
        <source>UNKNOWN</source>
        <translation>НЕИЗВЕСТНО</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="364"/>
        <source>Export diagnostics</source>
        <translation>Экспорт диагностики</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="366"/>
        <source>JSON (*.json)</source>
        <translation>JSON (*.json)</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="398"/>
        <source>Could not open the export file</source>
        <translation>Не удалось открыть файл экспорта</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="403"/>
        <source>Could not finish the export</source>
        <translation>Не удалось завершить экспорт</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="406"/>
        <source>Diagnostics exported: %1</source>
        <translation>Диагностика экспортирована: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="286"/>
        <source>%1:if%2  ·  req 0x%3 / resp 0x%4  ·  %5 B</source>
        <translation>%1:if%2  ·  запрос 0x%3 / ответ 0x%4  ·  %5 Б</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/gui/DebugWindow.cpp" line="246"/>
        <source>%n packet(s)</source>
        <translation>
            <numerusform>%n пакет</numerusform>
            <numerusform>%n пакета</numerusform>
            <numerusform>%n пакетов</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/gui/DebugWindow.cpp" line="292"/>
        <source>%1:if%2  ·  id 0x%3  ·  %4 B</source>
        <translation>%1:if%2  ·  id 0x%3  ·  %4 Б</translation>
    </message>
</context>
<context>
    <name>strikepro::HidMonitor</name>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="169"/>
        <source>HID interface 1 was not found</source>
        <translation>HID-интерфейс 1 не найден</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="130"/>
        <source>No access to interface 1 of device 0db0:%1. Reinstall the udev rule.</source>
        <translation>Нет доступа к интерфейсу 1 устройства 0db0:%1. Переустановите правило udev.</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="141"/>
        <source>No write access to %1: %2</source>
        <translation>Нет доступа на запись к %1: %2</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="157"/>
        <source>Could not send the battery query: %1</source>
        <translation>Не удалось отправить запрос батареи: %1</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="159"/>
        <source>Battery query was incomplete: %1 of %2 bytes</source>
        <translation>Запрос батареи отправлен не полностью: %1 из %2 байт</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="171"/>
        <source>No accessible HID interface 1 was found</source>
        <translation>Не найден доступный HID-интерфейс 1</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="262"/>
        <source>USB event monitor failed: %1</source>
        <translation>Ошибка отслеживания событий USB: %1</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="292"/>
        <source>Could not open %1: %2</source>
        <translation>Не удалось открыть %1: %2</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="348"/>
        <source>Read error on %1: %2</source>
        <translation>Ошибка чтения %1: %2</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="380"/>
        <source>HID interface %1 was not found</source>
        <translation>HID-интерфейс %1 не найден</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="395"/>
        <source>No access to %1. Install the udev rule from packaging.</source>
        <translation>Нет доступа к %1. Установите правило udev из packaging.</translation>
    </message>
    <message>
        <location filename="../src/device/HidMonitor.cpp" line="414"/>
        <source>%1 if%2 0x%3: %4</source>
        <translation>%1 if%2 0x%3: %4</translation>
    </message>
</context>
<context>
    <name>strikepro::MainWindow</name>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="544"/>
        <source>MSI Keyboard</source>
        <translation>MSI Keyboard</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="545"/>
        <source>Settings</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="546"/>
        <source>Language</source>
        <translation>Язык</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="547"/>
        <source>English</source>
        <translation>Английский</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="548"/>
        <source>Russian</source>
        <translation>Русский</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="549"/>
        <source>Debug</source>
        <translation>Отладка</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="550"/>
        <source>Logs</source>
        <translation>Логи</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="551"/>
        <source>Telemetry</source>
        <translation>Телеметрия</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="559"/>
        <source>MSI Keyboard manager for Linux</source>
        <translation>Менеджер клавиатур MSI для Linux</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="560"/>
        <source>BATTERY</source>
        <translation>АККУМУЛЯТОР</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="564"/>
        <source>DEVICE</source>
        <translation>УСТРОЙСТВО</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="611"/>
        <source>Battery profile unavailable</source>
        <translation>Профиль батареи недоступен</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="606"/>
        <source>Protocol profile loaded: %1</source>
        <translation>Профиль протокола загружен: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="565"/>
        <source>Status</source>
        <translation>Состояние</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="612"/>
        <source>Profile error: %1</source>
        <translation>Ошибка профиля: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="708"/>
        <source>MSI Strike Pro disconnected</source>
        <translation>MSI Strike Pro отключена</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="637"/>
        <source>No device</source>
        <translation>Нет устройства</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="659"/>
        <source>MSI Strike Pro · USB</source>
        <translation>MSI Strike Pro · USB</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="660"/>
        <source>MSI Strike Pro · 2.4 GHz</source>
        <translation>MSI Strike Pro · 2,4 ГГц</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="667"/>
        <source>Reading battery…</source>
        <translation>Получение заряда…</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="674"/>
        <source>HID access required</source>
        <translation>Нужен доступ к HID</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="764"/>
        <source>Battery query: %1</source>
        <translation>Запрос батареи: %1</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="685"/>
        <source>No response</source>
        <translation>Нет ответа</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="636"/>
        <source>No MSI keyboard detected</source>
        <translation>Клавиатура MSI не обнаружена</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="638"/>
        <source>Connect a keyboard or its USB receiver.</source>
        <translation>Подключите клавиатуру или её USB-приёмник.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="643"/>
        <source>No supported keyboard detected</source>
        <translation>Поддерживаемая клавиатура не обнаружена</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="646"/>
        <location filename="../src/gui/MainWindow.cpp" line="673"/>
        <source>MSI Strike Pro</source>
        <translation>MSI Strike Pro</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="647"/>
        <source>Checking…</source>
        <translation>Проверка…</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="649"/>
        <source>Waiting for a response from the keyboard.</source>
        <translation>Ожидание ответа от клавиатуры.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="654"/>
        <source>Checking connection</source>
        <translation>Проверка подключения</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="665"/>
        <source>Connected via USB</source>
        <translation>Подключена по USB</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="665"/>
        <source>Connected via 2.4 GHz</source>
        <translation>Подключена по 2,4 ГГц</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="668"/>
        <source>Waiting for battery data.</source>
        <translation>Ожидание данных аккумулятора.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="676"/>
        <source>The device is present, but Linux denied HID access.</source>
        <translation>Устройство найдено, но Linux запретил доступ к HID.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="681"/>
        <source>Permission problem</source>
        <translation>Проблема с правами доступа</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="684"/>
        <source>MSI Strike Pro receiver</source>
        <translation>Приёмник MSI Strike Pro</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="686"/>
        <source>The USB transport is present, but the keyboard did not answer.</source>
        <translation>USB-подключение доступно, но клавиатура не ответила.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="692"/>
        <source>Keyboard not responding</source>
        <translation>Клавиатура не отвечает</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="784"/>
        <source>The keyboard did not answer the battery query</source>
        <translation>Клавиатура не ответила на запрос батареи</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="806"/>
        <source>MSI Strike Pro answered over USB</source>
        <translation>MSI Strike Pro ответила по USB</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="807"/>
        <source>MSI Strike Pro answered through the 2.4 GHz receiver</source>
        <translation>MSI Strike Pro ответила через приёмник 2,4 ГГц</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="821"/>
        <source>The keyboard is charging.</source>
        <translation>Клавиатура подключена к зарядке.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="822"/>
        <source>The keyboard is running on battery.</source>
        <translation>Клавиатура работает от аккумулятора.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="824"/>
        <source>Battery status received over USB HID.</source>
        <translation>Заряд получен через USB HID.</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="827"/>
        <source>Battery updated: %1%</source>
        <translation>Заряд обновлён: %1%</translation>
    </message>
    <message>
        <location filename="../src/gui/MainWindow.cpp" line="348"/>
        <source>MSI STRIKE PRO</source>
        <translation>MSI STRIKE PRO</translation>
    </message>
    <message>
        <source>DEVICES</source>
        <translation>УСТРОЙСТВА</translation>
    </message>
    <message>
        <source>Connect a supported MSI keyboard.</source>
        <translation>Подключите поддерживаемую клавиатуру MSI.</translation>
    </message>
    <message>
        <source>SELECTED DEVICE</source>
        <translation>ВЫБРАННОЕ УСТРОЙСТВО</translation>
    </message>
    <message>
        <source>Supported MSI keyboard</source>
        <translation>Поддерживаемая клавиатура MSI</translation>
    </message>
    <message>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <source>2.4 GHz</source>
        <translation>2,4 ГГц</translation>
    </message>
    <message>
        <source>USB + 2.4 GHz</source>
        <translation>USB + 2,4 ГГц</translation>
    </message>
    <message>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <source>Disconnected</source>
        <translation>Отключена</translation>
    </message>
    <message>
        <source>Not responding</source>
        <translation>Не отвечает</translation>
    </message>
    <message>
        <source>Connected via %1</source>
        <translation>Подключена через %1</translation>
    </message>
    <message>
        <source>%1 detected via %2</source>
        <translation>%1 обнаружена через %2</translation>
    </message>
    <message>
        <source>%1 disconnected</source>
        <translation>%1 отключена</translation>
    </message>
</context>
<context>
    <name>strikepro::TrayIndicator</name>
    <message>
        <source>Open MSI Keyboard</source>
        <translation>Открыть MSI Keyboard</translation>
    </message>
    <message>
        <source>Quit</source>
        <translation>Выйти</translation>
    </message>
    <message>
        <source>MSI Keyboard</source>
        <translation>MSI Keyboard</translation>
    </message>
    <message>
        <source>No supported keyboard detected</source>
        <translation>Поддерживаемая клавиатура не обнаружена</translation>
    </message>
    <message>
        <source>Checking connection</source>
        <translation>Проверка подключения</translation>
    </message>
    <message>
        <source>Connection problem</source>
        <translation>Проблема подключения</translation>
    </message>
    <message>
        <source>Connected</source>
        <translation>Подключена</translation>
    </message>
    <message>
        <source>Battery: %1% · Charging</source>
        <translation>Заряд: %1% · Идёт зарядка</translation>
    </message>
    <message>
        <source>Battery: %1% · On battery</source>
        <translation>Заряд: %1% · От аккумулятора</translation>
    </message>
    <message>
        <source>Battery: %1%</source>
        <translation>Заряд: %1%</translation>
    </message>
</context>
</TS>
