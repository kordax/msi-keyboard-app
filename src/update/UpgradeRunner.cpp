#include "UpgradeRunner.h"

#include "UpdaterCore.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTimer>
#include <QUuid>

#include <cstdio>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace msikeyboard::update {
namespace {

constexpr qint64 kMaximumMetadataBytes = 1024 * 1024;
constexpr qint64 kMaximumPackageBytes = 512 * 1024 * 1024;
constexpr int kMetadataTimeoutMs = 30'000;
constexpr int kPackageTimeoutMs = 300'000;

const QUrl kLatestReleaseUrl(
    QStringLiteral(
        "https://api.github.com/repos/kordax/msi-keyboard-app/releases/latest"));

QString tr(const char *sourceText)
{
    return QCoreApplication::translate("msikeyboard::update", sourceText);
}

struct DownloadResult {
    QByteArray data;
    QString error;
    int httpStatus = 0;
    bool timedOut = false;

    [[nodiscard]] bool succeeded() const
    {
        return error.isEmpty() && httpStatus >= 200 && httpStatus < 300;
    }
};

QNetworkRequest makeRequest(const QUrl &url, const QString &currentVersion)
{
    QNetworkRequest request(url);
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::UserVerifiedRedirectPolicy);
    request.setMaximumRedirectsAllowed(5);
    request.setRawHeader(
        QByteArrayLiteral("User-Agent"),
        QStringLiteral("msi-keyboard/%1").arg(currentVersion).toUtf8());
    request.setRawHeader(
        QByteArrayLiteral("Accept"),
        QByteArrayLiteral("application/vnd.github+json"));
    request.setRawHeader(
        QByteArrayLiteral("X-GitHub-Api-Version"),
        QByteArrayLiteral("2026-03-10"));
    return request;
}

DownloadResult fetchBytes(
    QNetworkAccessManager &network,
    const QUrl &url,
    const QString &currentVersion,
    qint64 maximumBytes)
{
    DownloadResult result;
    if (!isAllowedGitHubUrl(url)) {
        result.error = tr("The network request URL is not trusted.");
        return result;
    }
    QNetworkReply *reply = network.get(makeRequest(url, currentVersion));
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    bool tooLarge = false;
    bool untrustedRedirect = false;
    const auto readAvailable = [&] {
        result.data.append(reply->readAll());
        if (result.data.size() > maximumBytes) {
            tooLarge = true;
            reply->abort();
        }
    };

    QObject::connect(reply, &QNetworkReply::readyRead, &loop, readAvailable);
    QObject::connect(
        reply,
        &QNetworkReply::redirected,
        &loop,
        [reply, &untrustedRedirect](const QUrl &target) {
            const QUrl resolved = reply->url().resolved(target);
            if (isAllowedGitHubUrl(resolved)) {
                reply->redirectAllowed();
            } else {
                untrustedRedirect = true;
                reply->abort();
            }
        });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, [&] {
        result.timedOut = true;
        reply->abort();
    });

    timeout.start(kMetadataTimeoutMs);
    loop.exec();
    timeout.stop();
    readAvailable();

    result.httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (result.timedOut) {
        result.error = tr("The network request timed out.");
    } else if (untrustedRedirect || !isAllowedGitHubUrl(reply->url())) {
        result.error = tr("The server redirected to an untrusted URL.");
    } else if (tooLarge) {
        result.error = tr("The server response is unexpectedly large.");
    } else if (reply->error() != QNetworkReply::NoError) {
        result.error = reply->errorString();
    } else if (result.httpStatus < 200 || result.httpStatus >= 300) {
        result.error =
            tr("The server returned HTTP %1.").arg(result.httpStatus);
    }

    reply->deleteLater();
    return result;
}

QString downloadPackage(
    QNetworkAccessManager &network,
    const ReleaseAsset &asset,
    const QString &destinationPath,
    const QString &currentVersion,
    QString *error)
{
    if (asset.size <= 0 || asset.size > kMaximumPackageBytes) {
        if (error != nullptr) {
            *error = tr("The package size reported by GitHub is invalid.");
        }
        return {};
    }
    if (!isAllowedGitHubUrl(asset.downloadUrl)) {
        if (error != nullptr) {
            *error = tr("The package download URL is not trusted.");
        }
        return {};
    }

    QSaveFile destination(destinationPath);
    if (!destination.open(QIODevice::WriteOnly)) {
        if (error != nullptr) {
            *error = destination.errorString();
        }
        return {};
    }

    QNetworkReply *reply =
        network.get(makeRequest(asset.downloadUrl, currentVersion));
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    bool timedOut = false;
    bool writeFailed = false;
    bool sizeMismatch = false;
    bool untrustedRedirect = false;
    qint64 downloadedBytes = 0;
    const auto writeAvailable = [&] {
        const QByteArray chunk = reply->readAll();
        downloadedBytes += chunk.size();
        if (downloadedBytes > asset.size) {
            sizeMismatch = true;
            reply->abort();
            return;
        }
        if (destination.write(chunk) != chunk.size()) {
            writeFailed = true;
            reply->abort();
        }
    };

    QObject::connect(reply, &QNetworkReply::readyRead, &loop, writeAvailable);
    QObject::connect(
        reply,
        &QNetworkReply::redirected,
        &loop,
        [reply, &untrustedRedirect](const QUrl &target) {
            const QUrl resolved = reply->url().resolved(target);
            if (isAllowedGitHubUrl(resolved)) {
                reply->redirectAllowed();
            } else {
                untrustedRedirect = true;
                reply->abort();
            }
        });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, [&] {
        timedOut = true;
        reply->abort();
    });

    timeout.start(kPackageTimeoutMs);
    loop.exec();
    timeout.stop();
    writeAvailable();

    const int httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (timedOut) {
        *error = tr("The package download timed out.");
    } else if (untrustedRedirect || !isAllowedGitHubUrl(reply->url())) {
        *error = tr("The server redirected to an untrusted URL.");
    } else if (sizeMismatch) {
        *error = tr("The downloaded package size does not match the release metadata.");
    } else if (writeFailed) {
        *error = tr("The downloaded package could not be written.");
    } else if (reply->error() != QNetworkReply::NoError) {
        *error = reply->errorString();
    } else if (httpStatus < 200 || httpStatus >= 300) {
        *error = tr("The server returned HTTP %1.").arg(httpStatus);
    } else if (downloadedBytes != asset.size) {
        *error = tr("The downloaded package size does not match the release metadata.");
    } else if (!destination.commit()) {
        *error = destination.errorString();
    }

    reply->deleteLater();
    return error->isEmpty() ? destinationPath : QString{};
}

bool isAdministrator()
{
#ifdef Q_OS_UNIX
    return geteuid() == 0;
#else
    return false;
#endif
}

QString findSystemExecutable(QStringView programName)
{
    const QStringList directories{
        QStringLiteral("/usr/bin"),
        QStringLiteral("/bin"),
        QStringLiteral("/usr/sbin"),
        QStringLiteral("/sbin"),
    };
    for (const QString &directory : directories) {
        const QFileInfo candidate(
            QDir(directory).filePath(programName.toString()));
        if (candidate.isFile() && candidate.isExecutable()) {
            return candidate.absoluteFilePath();
        }
    }
    return {};
}

bool executableIsPackageManaged(PackageFormat packageFormat)
{
    QString queryProgram;
    QStringList queryArguments;
    const QString executablePath =
        QFileInfo(QCoreApplication::applicationFilePath()).canonicalFilePath();
    if (executablePath.isEmpty()) {
        return false;
    }

    if (packageFormat == PackageFormat::Deb) {
        queryProgram = findSystemExecutable(QStringView(u"dpkg-query"));
        queryArguments = {
            QStringLiteral("--search"),
            executablePath,
        };
    } else if (packageFormat == PackageFormat::Rpm) {
        queryProgram = findSystemExecutable(QStringView(u"rpm"));
        queryArguments = {
            QStringLiteral("-qf"),
            executablePath,
        };
    }
    if (queryProgram.isEmpty()) {
        return false;
    }

    QProcess query;
    query.setProcessChannelMode(QProcess::MergedChannels);
    query.start(queryProgram, queryArguments);
    if (!query.waitForStarted(2'000)) {
        return false;
    }
    if (!query.waitForFinished(5'000)) {
        query.kill();
        query.waitForFinished();
        return false;
    }
    return query.exitStatus() == QProcess::NormalExit && query.exitCode() == 0;
}

struct UpgradeTools {
    QString sudoProgram;
    QString mkdirProgram;
    QString copyProgram;
    QString sha256Program;
    QString removeProgram;
    QString metadataProgram;
};

struct CommandInvocation {
    QString program;
    QStringList arguments;
};

std::optional<UpgradeTools> findUpgradeTools(
    PackageFormat format,
    bool administrator,
    QString *error)
{
    UpgradeTools tools{
        .sudoProgram =
            administrator ? QString() : findSystemExecutable(QStringView(u"sudo")),
        .mkdirProgram = findSystemExecutable(QStringView(u"mkdir")),
        .copyProgram = findSystemExecutable(QStringView(u"install")),
        .sha256Program = findSystemExecutable(QStringView(u"sha256sum")),
        .removeProgram = findSystemExecutable(QStringView(u"rm")),
        .metadataProgram =
            findSystemExecutable(
                format == PackageFormat::Deb
                    ? QStringView(u"dpkg-deb")
                    : QStringView(u"rpm")),
    };
    if ((!administrator && tools.sudoProgram.isEmpty())
        || tools.mkdirProgram.isEmpty() || tools.copyProgram.isEmpty()
        || tools.sha256Program.isEmpty() || tools.removeProgram.isEmpty()
        || tools.metadataProgram.isEmpty()) {
        if (error != nullptr) {
            *error = tr(
                "A required trusted system utility is not available for the upgrade.");
        }
        return std::nullopt;
    }
    return tools;
}

CommandInvocation elevated(
    const QString &program,
    const QStringList &arguments,
    bool administrator,
    const UpgradeTools &tools)
{
    if (administrator) {
        return CommandInvocation{
            .program = program,
            .arguments = arguments,
        };
    }
    QStringList elevatedArguments{QStringLiteral("--"), program};
    elevatedArguments.append(arguments);
    return CommandInvocation{
        .program = tools.sudoProgram,
        .arguments = elevatedArguments,
    };
}

QString displayElevatedInstallCommand(
    const InstallPlan &plan,
    bool administrator,
    const UpgradeTools &tools)
{
    const CommandInvocation invocation =
        elevated(plan.program, plan.arguments, administrator, tools);
    InstallPlan displayed = plan;
    displayed.program = invocation.program;
    displayed.arguments = invocation.arguments;
    return displayInstallCommand(displayed);
}

int executeCommand(
    const CommandInvocation &invocation,
    QByteArray *standardOutput,
    QString *error)
{
    QProcess process;
    process.setInputChannelMode(QProcess::ForwardedInputChannel);
    process.setProcessChannelMode(
        standardOutput == nullptr
            ? QProcess::ForwardedChannels
            : QProcess::SeparateChannels);
    process.start(invocation.program, invocation.arguments);
    if (!process.waitForStarted()) {
        *error = process.errorString();
        return -1;
    }
    if (!process.waitForFinished(-1)) {
        *error = process.errorString();
        return -1;
    }
    if (standardOutput != nullptr) {
        *standardOutput = process.readAllStandardOutput();
        if (standardOutput->size() > 64 * 1024) {
            *error = tr("A system utility returned unexpectedly large output.");
            return -1;
        }
    }
    if (process.exitStatus() != QProcess::NormalExit) {
        *error = tr("A system utility process crashed.");
        return -1;
    }
    const int exitCode = process.exitCode();
    if (exitCode != 0) {
        const QString details =
            QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        *error = details.isEmpty()
            ? tr("A system utility exited with status %1.").arg(exitCode)
            : details;
    }
    return exitCode;
}

void removeStagingDirectory(
    const QString &directory,
    bool administrator,
    const UpgradeTools &tools)
{
    QString ignoredError;
    executeCommand(
        elevated(
            tools.removeProgram,
            {
                QStringLiteral("--recursive"),
                QStringLiteral("--force"),
                QStringLiteral("--"),
                directory,
            },
            administrator,
            tools),
        nullptr,
        &ignoredError);
}

bool stageAndVerifyPackage(
    const QString &sourcePath,
    const QString &stagedPath,
    const QByteArray &expectedChecksum,
    bool administrator,
    const UpgradeTools &tools,
    QString *error)
{
    const QString stagingDirectory = QFileInfo(stagedPath).absolutePath();
    if (executeCommand(
            elevated(
                tools.mkdirProgram,
                {
                    QStringLiteral("--mode=0700"),
                    QStringLiteral("--"),
                    stagingDirectory,
                },
                administrator,
                tools),
            nullptr,
            error)
        != 0) {
        return false;
    }

    if (executeCommand(
            elevated(
                tools.copyProgram,
                {
                    QStringLiteral("--mode=0600"),
                    QStringLiteral("--"),
                    sourcePath,
                    stagedPath,
                },
                administrator,
                tools),
            nullptr,
            error)
        != 0) {
        removeStagingDirectory(stagingDirectory, administrator, tools);
        return false;
    }

    QByteArray checksumOutput;
    if (executeCommand(
            elevated(
                tools.sha256Program,
                {QStringLiteral("--"), stagedPath},
                administrator,
                tools),
            &checksumOutput,
            error)
        != 0) {
        removeStagingDirectory(stagingDirectory, administrator, tools);
        return false;
    }
    const QByteArray stagedChecksum =
        checksumOutput.simplified().split(' ').value(0).toLower();
    if (stagedChecksum != expectedChecksum.toLower()) {
        *error = tr(
            "The protected package copy failed SHA-256 verification.");
        removeStagingDirectory(stagingDirectory, administrator, tools);
        return false;
    }
    return true;
}

std::optional<PackageMetadata> inspectPackageMetadata(
    const QString &stagedPath,
    PackageFormat format,
    bool administrator,
    const UpgradeTools &tools,
    QString *error)
{
    const QStringList arguments =
        format == PackageFormat::Deb
        ? QStringList{
              QStringLiteral("--show"),
              QStringLiteral(
                  "--showformat=${Package}\\n${Version}\\n${Architecture}\\n"),
              QStringLiteral("--"),
              stagedPath,
          }
        : QStringList{
              QStringLiteral("-qp"),
              QStringLiteral("--queryformat"),
              QStringLiteral("%{NAME}\\n%{VERSION}-%{RELEASE}\\n%{ARCH}\\n"),
              QStringLiteral("--"),
              stagedPath,
          };
    QByteArray output;
    if (executeCommand(
            elevated(
                tools.metadataProgram,
                arguments,
                administrator,
                tools),
            &output,
            error)
        != 0) {
        return std::nullopt;
    }
    return parsePackageMetadata(output, error);
}

bool confirmInstallation(const QString &command, bool requestsPrivileges)
{
    QTextStream output(stdout);
    QTextStream input(stdin);
    output << (requestsPrivileges
                   ? tr("The following command will request administrator privileges:")
                   : tr("The following package manager command will be run:"))
           << '\n'
           << "  " << command << '\n'
           << tr("Install the update? [y/N] ");
    output.flush();

    const QString answer = input.readLine().trimmed().toLower();
    return answer == QStringLiteral("y") || answer == QStringLiteral("yes")
        || answer == QString::fromUtf8("д") || answer == QString::fromUtf8("да");
}

} // namespace

int runUpgrade(const QString &currentVersion)
{
    QTextStream output(stdout);
    QTextStream errors(stderr);
    output << tr("Checking GitHub for a stable update...") << '\n';
    output.flush();

    PlatformInfo platform = detectPlatform(probeCurrentPlatform());
    if (platform.packageFormat == PackageFormat::Unknown
        || platform.architecture == CpuArchitecture::Unknown
        || platform.installerProgram.isEmpty()) {
        errors << tr("This Linux distribution or CPU architecture is not supported.")
               << '\n';
        return 3;
    }
    platform.systemPackageInstall =
        executableIsPackageManaged(platform.packageFormat);

    QNetworkAccessManager network;
    const DownloadResult releaseResponse = fetchBytes(
        network,
        kLatestReleaseUrl,
        currentVersion,
        kMaximumMetadataBytes);
    if (releaseResponse.httpStatus == 404) {
        output << tr("No stable GitHub release has been published yet.") << '\n';
        return 0;
    }
    if (!releaseResponse.succeeded()) {
        errors << tr("Could not check for updates: %1")
                      .arg(releaseResponse.error)
               << '\n';
        return 2;
    }

    QString error;
    const auto release = parseGitHubRelease(releaseResponse.data, &error);
    if (!release.has_value()) {
        errors << tr("Could not read the latest release: %1").arg(error) << '\n';
        return 2;
    }

    const auto comparison =
        compareVersions(release->version, currentVersion, &error);
    if (!comparison.has_value()) {
        errors << tr("Could not compare release versions: %1").arg(error) << '\n';
        return 2;
    }
    if (*comparison <= 0) {
        output << tr("MSI Keyboard %1 is already up to date.")
                      .arg(currentVersion)
               << '\n';
        return 0;
    }

    const auto selection = selectUpgrade(
        *release,
        currentVersion,
        platform,
        QStringView(u"msi-keyboard"),
        &error);
    if (!selection.has_value()) {
        errors << tr("No verified package is available for this system: %1")
                      .arg(error)
               << '\n';
        return 3;
    }

    output << tr("Update %1 is available.").arg(release->version) << '\n'
           << tr("Downloading checksums...") << '\n';
    output.flush();

    const DownloadResult checksumResponse = fetchBytes(
        network,
        selection->checksumAsset.downloadUrl,
        currentVersion,
        kMaximumMetadataBytes);
    if (!checksumResponse.succeeded()) {
        errors << tr("Could not download SHA-256 checksums: %1")
                      .arg(checksumResponse.error)
               << '\n';
        return 2;
    }

    const auto manifest = parseSha256Manifest(checksumResponse.data, &error);
    if (!manifest.has_value()) {
        errors << tr("Could not parse SHA-256 checksums: %1").arg(error)
               << '\n';
        return 4;
    }
    const auto expectedChecksum =
        checksumForAsset(*manifest, selection->packageAsset.name, &error);
    if (!expectedChecksum.has_value()) {
        errors << tr("Could not find the package checksum: %1").arg(error)
               << '\n';
        return 4;
    }

    QTemporaryDir temporaryDirectory(
        QStandardPaths::writableLocation(QStandardPaths::TempLocation)
        + QStringLiteral("/msi-keyboard-upgrade-XXXXXX"));
    if (!temporaryDirectory.isValid()) {
        errors << tr("Could not create a temporary download directory.") << '\n';
        return 2;
    }

    const QString packagePath =
        temporaryDirectory.filePath(selection->packageAsset.name);
    output << tr("Downloading %1...").arg(selection->packageAsset.name) << '\n';
    output.flush();
    if (downloadPackage(
            network,
            selection->packageAsset,
            packagePath,
            currentVersion,
            &error)
        .isEmpty()) {
        errors << tr("Could not download the package: %1").arg(error) << '\n';
        return 2;
    }

    const VerificationResult verification =
        verifyFileSha256(packagePath, *expectedChecksum, nullptr, &error);
    if (verification != VerificationResult::Match) {
        if (verification == VerificationResult::Mismatch) {
            error = tr("The downloaded package SHA-256 checksum does not match.");
        }
        errors << tr("Package verification failed: %1").arg(error) << '\n';
        return 4;
    }
    output << tr("SHA-256 verification passed.") << '\n';

    const auto plan = createInstallPlan(
        platform,
        packagePath,
        VerificationResult::Match,
        &error);
    if (!plan.has_value()) {
        errors << tr("Could not prepare package installation: %1").arg(error)
               << '\n';
        return 3;
    }

    const bool administrator = isAdministrator();
    const auto tools =
        findUpgradeTools(platform.packageFormat, administrator, &error);
    if (!tools.has_value()) {
        errors << tr("Could not prepare protected installation: %1").arg(error)
               << '\n';
        return 5;
    }

    const QString stagingDirectory =
        QStringLiteral("/var/tmp/msi-keyboard-upgrade-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    const QString stagedPackagePath =
        QDir(stagingDirectory).filePath(selection->packageAsset.name);
    InstallPlan stagedPlan = *plan;
    if (stagedPlan.arguments.isEmpty()
        || stagedPlan.arguments.constLast() != stagedPlan.packagePath) {
        errors << tr("Could not prepare protected installation.") << '\n';
        return 5;
    }
    stagedPlan.packagePath = stagedPackagePath;
    stagedPlan.arguments.last() = stagedPackagePath;
    const QString command =
        displayElevatedInstallCommand(stagedPlan, administrator, *tools);

    if (!plan->packageManagedInstall) {
        output << tr(
                      "This copy is not managed by the system package manager; "
                      "the package will be installed system-wide.")
               << '\n';
    }
    output.flush();

    if (!confirmInstallation(command, !administrator)) {
        output << tr("Update cancelled.") << '\n';
        return 0;
    }

    if (!stageAndVerifyPackage(
            packagePath,
            stagedPackagePath,
            *expectedChecksum,
            administrator,
            *tools,
            &error)) {
        errors << tr("Could not create a protected package copy: %1").arg(error)
               << '\n';
        return 5;
    }

    const auto metadata = inspectPackageMetadata(
        stagedPackagePath,
        platform.packageFormat,
        administrator,
        *tools,
        &error);
    if (!metadata.has_value()
        || !packageMetadataMatches(
            *metadata,
            release->version,
            platform,
            QStringView(u"msi-keyboard"),
            &error)) {
        removeStagingDirectory(stagingDirectory, administrator, *tools);
        errors << tr("Package metadata verification failed: %1").arg(error)
               << '\n';
        return 4;
    }

    output << tr("Protected package verification passed.") << '\n';
    output.flush();

    const int exitCode = executeCommand(
        elevated(
            stagedPlan.program,
            stagedPlan.arguments,
            administrator,
            *tools),
        nullptr,
        &error);
    removeStagingDirectory(stagingDirectory, administrator, *tools);
    if (exitCode != 0) {
        errors << tr("The package manager failed: %1").arg(error) << '\n';
        return 5;
    }

    output << tr("MSI Keyboard was upgraded to %1.").arg(release->version)
           << '\n';
    return 0;
}

} // namespace msikeyboard::update
