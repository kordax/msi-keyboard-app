#pragma once

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QUrl>

#include <optional>

namespace msikeyboard::update {

enum class PackageFormat {
    Unknown,
    Deb,
    Rpm,
};

enum class CpuArchitecture {
    Unknown,
    X86_64,
    Arm64,
};

struct ReleaseAsset {
    QString name;
    QUrl downloadUrl;
    qint64 size = 0;
};

struct GitHubRelease {
    QString tagName;
    QString version;
    QString name;
    QUrl pageUrl;
    QList<ReleaseAsset> assets;
    bool draft = false;
    bool prerelease = false;
};

struct PlatformProbe {
    QString osReleaseText;
    QString cpuArchitecture;
    QStringList availablePrograms;
    QString executablePath;
};

struct PlatformInfo {
    PackageFormat packageFormat = PackageFormat::Unknown;
    CpuArchitecture architecture = CpuArchitecture::Unknown;
    QString installerProgram;
    bool systemPackageInstall = false;
};

struct UpgradeSelection {
    GitHubRelease release;
    ReleaseAsset packageAsset;
    ReleaseAsset checksumAsset;
};

struct PackageMetadata {
    QString name;
    QString version;
    QString architecture;
};

using Sha256Manifest = QHash<QString, QByteArray>;

enum class VerificationResult {
    Match,
    Mismatch,
    InvalidChecksum,
    FileError,
};

struct InstallPlan {
    QString program;
    QStringList arguments;
    QString packagePath;
    PackageFormat packageFormat = PackageFormat::Unknown;
    bool requiresAdministrator = true;
    bool explicitConfirmationRequired = true;
    bool packageManagedInstall = false;
};

enum class InstallReadiness {
    AwaitingConfirmation,
    RequiresAdministrator,
    Ready,
};

std::optional<GitHubRelease> parseGitHubRelease(
    const QByteArray &json,
    QString *error = nullptr);

std::optional<int> compareVersions(
    QStringView left,
    QStringView right,
    QString *error = nullptr);

bool isAllowedGitHubUrl(const QUrl &url);

bool isNewerStableRelease(
    const GitHubRelease &release,
    QStringView currentVersion,
    QString *error = nullptr);

CpuArchitecture normalizeArchitecture(QStringView architecture);
PlatformInfo detectPlatform(const PlatformProbe &probe);
PlatformProbe probeCurrentPlatform();

std::optional<ReleaseAsset> selectPackageAsset(
    const GitHubRelease &release,
    const PlatformInfo &platform,
    QStringView productName = QStringView(u"msi-keyboard"),
    QString *error = nullptr);

std::optional<ReleaseAsset> selectChecksumAsset(
    const GitHubRelease &release,
    QStringView packageAssetName,
    QString *error = nullptr);

std::optional<UpgradeSelection> selectUpgrade(
    const GitHubRelease &release,
    QStringView currentVersion,
    const PlatformInfo &platform,
    QStringView productName = QStringView(u"msi-keyboard"),
    QString *error = nullptr);

std::optional<PackageMetadata> parsePackageMetadata(
    const QByteArray &output,
    QString *error = nullptr);

bool packageMetadataMatches(
    const PackageMetadata &metadata,
    QStringView releaseVersion,
    const PlatformInfo &platform,
    QStringView productName = QStringView(u"msi-keyboard"),
    QString *error = nullptr);

std::optional<Sha256Manifest> parseSha256Manifest(
    const QByteArray &contents,
    QString *error = nullptr);

std::optional<QByteArray> checksumForAsset(
    const Sha256Manifest &manifest,
    QStringView assetName,
    QString *error = nullptr);

VerificationResult verifyFileSha256(
    const QString &path,
    const QByteArray &expectedHex,
    QByteArray *actualHex = nullptr,
    QString *error = nullptr);

std::optional<InstallPlan> createInstallPlan(
    const PlatformInfo &platform,
    const QString &packagePath,
    VerificationResult verification,
    QString *error = nullptr);

InstallReadiness assessInstallReadiness(
    const InstallPlan &plan,
    bool explicitlyConfirmed,
    bool hasAdministratorPrivileges);

QString displayInstallCommand(const InstallPlan &plan);

} // namespace msikeyboard::update
