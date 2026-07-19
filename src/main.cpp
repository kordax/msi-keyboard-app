#include "cli/CliRunner.h"
#include "device/AppPaths.h"
#include "gui/MainWindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>

#include <memory>

namespace {

bool isCliRequested(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index) {
        if (QString::fromLocal8Bit(argv[index]) == QStringLiteral("--cli")) {
            return true;
        }
    }
    return false;
}

} // namespace

int main(int argc, char *argv[])
{
    const bool cliRequested = isCliRequested(argc, argv);
    std::unique_ptr<QCoreApplication> application;
    if (cliRequested) {
        application = std::make_unique<QCoreApplication>(argc, argv);
    } else {
        application = std::make_unique<QApplication>(argc, argv);
    }

    QCoreApplication::setOrganizationName(QStringLiteral("kordax"));
    QCoreApplication::setApplicationName(QStringLiteral("msi-keyboard"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Linux application for MSI keyboards"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption cliOption(
        QStringLiteral("cli"),
        QStringLiteral("Запустить без GUI."));
    const QCommandLineOption logsOption(
        QStringLiteral("logs"),
        QStringLiteral("В CLI непрерывно выводить события HID и диагностику."));
    const QCommandLineOption onceOption(
        QStringLiteral("once"),
        QStringLiteral("В CLI сделать один scan и read-only снимок, затем завершиться."));
    const QCommandLineOption jsonOption(
        QStringLiteral("json"),
        QStringLiteral("В CLI выводить newline-delimited JSON."));
    const QCommandLineOption batteryOption(
        QStringLiteral("battery"),
        QStringLiteral("В CLI один раз запросить заряд подтверждённой командой MSI Center."));
    const QCommandLineOption profileOption(
        QStringLiteral("profile"),
        QStringLiteral("Путь к JSON-профилю декодера батареи."),
        QStringLiteral("path"));
    parser.addOptions(
        {cliOption, logsOption, onceOption, jsonOption, batteryOption, profileOption});
    parser.process(*application);

    if (cliRequested) {
        QString profilePath = parser.value(profileOption);
        if (profilePath.isEmpty()) {
            profilePath = strikepro::defaultProtocolProfilePath();
        }
        strikepro::CliRunner runner(
            strikepro::CliOptions{
                .json = parser.isSet(jsonOption),
                .once = parser.isSet(onceOption),
                .battery = parser.isSet(batteryOption),
                .logs = parser.isSet(logsOption),
                .profilePath = profilePath,
            });
        return application->exec();
    }

    if (parser.isSet(logsOption) || parser.isSet(onceOption)
        || parser.isSet(jsonOption)
        || parser.isSet(batteryOption)
        || parser.isSet(profileOption)) {
        parser.showHelp(1);
    }

    strikepro::MainWindow window;
    window.show();
    return application->exec();
}
