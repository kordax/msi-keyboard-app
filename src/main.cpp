#include "cli/CliRunner.h"
#include "device/AppPaths.h"
#include "gui/MainWindow.h"
#include "i18n/LanguageManager.h"
#include "update/UpgradeRunner.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

#include <memory>
#include <optional>

namespace {

bool hasArgument(int argc, char *argv[], QStringView expected)
{
    for (int index = 1; index < argc; ++index) {
        if (QString::fromLocal8Bit(argv[index]) == expected) {
            return true;
        }
    }
    return false;
}

std::optional<QString> requestedLanguage(int argc, char *argv[])
{
    constexpr auto prefix = "--language=";
    for (int index = 1; index < argc; ++index) {
        const QString argument = QString::fromLocal8Bit(argv[index]);
        if (argument == QStringLiteral("--language")) {
            if (index + 1 >= argc) {
                return QString();
            }
            const QString value = QString::fromLocal8Bit(argv[index + 1]);
            return value.startsWith(QLatin1Char('-')) ? std::optional(QString())
                                                      : std::optional(value);
        }
        if (argument.startsWith(QString::fromLatin1(prefix))) {
            return argument.sliced(QString::fromLatin1(prefix).size());
        }
    }
    return std::nullopt;
}

} // namespace

int main(int argc, char *argv[])
{
    const bool cliRequested = hasArgument(argc, argv, QStringView(u"--cli"));
    const bool upgradeRequested =
        hasArgument(argc, argv, QStringView(u"upgrade"));
    const bool terminalOnlyRequested =
        hasArgument(argc, argv, QStringView(u"--help"))
        || hasArgument(argc, argv, QStringView(u"-h"))
        || hasArgument(argc, argv, QStringView(u"--version"))
        || hasArgument(argc, argv, QStringView(u"-v"));
    std::unique_ptr<QCoreApplication> application;
    if (cliRequested || upgradeRequested || terminalOnlyRequested) {
        application = std::make_unique<QCoreApplication>(argc, argv);
    } else {
        application = std::make_unique<QApplication>(argc, argv);
    }

    QCoreApplication::setOrganizationName(QStringLiteral("kordax"));
    QCoreApplication::setApplicationName(QStringLiteral("msi-keyboard"));
    QCoreApplication::setApplicationVersion(
        QStringLiteral(MSI_KEYBOARD_VERSION));
    if (!cliRequested && !upgradeRequested && !terminalOnlyRequested) {
        QApplication::setDesktopFileName(
            QStringLiteral("io.github.kordax.MsiKeyboard"));
    }

    strikepro::LanguageManager languageManager;
    const std::optional<QString> languageOverride =
        requestedLanguage(argc, argv);
    const QString initialLanguage =
        languageOverride.has_value() ? *languageOverride
        : (cliRequested || upgradeRequested)
            ? strikepro::LanguageManager::defaultLanguage()
            : strikepro::LanguageManager::storedLanguage();
    if (!languageManager.setLanguage(initialLanguage)) {
        QTextStream(stderr) << QCoreApplication::translate(
                                   "CommandLine",
                                   "Unsupported language \"%1\". Use en or ru.")
                                   .arg(initialLanguage)
                            << '\n';
        return 1;
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate(
        "CommandLine",
        "Linux application for MSI keyboards"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption cliOption(
        QStringLiteral("cli"),
        QCoreApplication::translate("CommandLine", "Run without the GUI."));
    const QCommandLineOption logsOption(
        QStringLiteral("logs"),
        QCoreApplication::translate(
            "CommandLine",
            "Continuously print HID events and diagnostics in CLI mode."));
    const QCommandLineOption onceOption(
        QStringLiteral("once"),
        QCoreApplication::translate(
            "CommandLine",
            "Take one read-only HID snapshot in CLI mode, then exit."));
    const QCommandLineOption jsonOption(
        QStringLiteral("json"),
        QCoreApplication::translate(
            "CommandLine",
            "Print newline-delimited JSON in CLI mode."));
    const QCommandLineOption batteryOption(
        QStringLiteral("battery"),
        QCoreApplication::translate(
            "CommandLine",
            "Query the battery once using the confirmed MSI Center command."));
    const QCommandLineOption profileOption(
        QStringLiteral("profile"),
        QCoreApplication::translate(
            "CommandLine",
            "Path to the battery decoder JSON profile."),
        QCoreApplication::translate("CommandLine", "path"));
    const QCommandLineOption languageOption(
        QStringLiteral("language"),
        QCoreApplication::translate(
            "CommandLine",
            "Use a language for this run. The default is English."),
        QStringLiteral("en|ru"));
    parser.addOptions(
        {cliOption,
         logsOption,
         onceOption,
         jsonOption,
         batteryOption,
         profileOption,
         languageOption});
    parser.addPositionalArgument(
        QStringLiteral("command"),
        QCoreApplication::translate("CommandLine", "Command to run: upgrade."),
        QStringLiteral("[upgrade]"));
    parser.process(*application);

    const QStringList positionalArguments = parser.positionalArguments();
    if (!positionalArguments.isEmpty()) {
        if (positionalArguments.size() != 1
            || positionalArguments.constFirst() != QStringLiteral("upgrade")
            || parser.isSet(logsOption) || parser.isSet(onceOption)
            || parser.isSet(jsonOption) || parser.isSet(batteryOption)
            || parser.isSet(profileOption)) {
            parser.showHelp(1);
        }
        return msikeyboard::update::runUpgrade(
            QCoreApplication::applicationVersion());
    }

    if (cliRequested) {
        QString profilePath = parser.value(profileOption);
        if (profilePath.isEmpty()) {
            profilePath = strikepro::defaultProtocolProfilePath();
        }
        strikepro::CliRunner runner(strikepro::CliOptions{
            .json = parser.isSet(jsonOption),
            .once = parser.isSet(onceOption),
            .battery = parser.isSet(batteryOption),
            .logs = parser.isSet(logsOption),
            .profilePath = profilePath,
        });
        return application->exec();
    }

    if (parser.isSet(logsOption) || parser.isSet(onceOption)
        || parser.isSet(jsonOption) || parser.isSet(batteryOption)
        || parser.isSet(profileOption)) {
        parser.showHelp(1);
    }

    strikepro::MainWindow window(&languageManager);
    window.show();
    return application->exec();
}
