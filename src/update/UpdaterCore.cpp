#include "UpdaterCore.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSysInfo>

#include <algorithm>
#include <limits>

namespace msikeyboard::update {
namespace {

QString tr(const char *sourceText)
{
    return QCoreApplication::translate("msikeyboard::update", sourceText);
}

void setError(QString *error, const QString &message)
{
    if (error != nullptr) {
        *error = message;
    }
}

QString normalizedVersionText(QStringView input)
{
    QString value = input.trimmed().toString();
    if (value.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        value.remove(0, 1);
    }
    return value;
}

struct VersionIdentifier {
    QString text;
    bool numeric = false;
    quint64 number = 0;
};

struct ParsedVersion {
    QList<quint64> core;
    QList<VersionIdentifier> prerelease;
};

std::optional<ParsedVersion> parseVersion(QStringView input, QString *error)
{
    QString value = normalizedVersionText(input);
    if (value.isEmpty()) {
        setError(error, tr("Version is empty."));
        return std::nullopt;
    }

    const qsizetype buildSeparator = value.indexOf(QLatin1Char('+'));
    if (buildSeparator >= 0) {
        value.truncate(buildSeparator);
    }

    QString prerelease;
    const qsizetype prereleaseSeparator = value.indexOf(QLatin1Char('-'));
    if (prereleaseSeparator >= 0) {
        prerelease = value.mid(prereleaseSeparator + 1);
        value.truncate(prereleaseSeparator);
        if (prerelease.isEmpty()) {
            setError(error, tr("Version has an empty prerelease suffix."));
            return std::nullopt;
        }
    }

    ParsedVersion parsed;
    const QStringList coreParts =
        value.split(QLatin1Char('.'), Qt::KeepEmptyParts);
    if (coreParts.isEmpty()) {
        setError(error, tr("Version has no numeric components."));
        return std::nullopt;
    }
    for (const QString &part : coreParts) {
        bool ok = false;
        const quint64 number = part.toULongLong(&ok);
        if (!ok || part.isEmpty()) {
            setError(
                error,
                tr("Version component is not numeric: %1").arg(part));
            return std::nullopt;
        }
        parsed.core.append(number);
    }

    if (!prerelease.isEmpty()) {
        const QStringList identifiers =
            prerelease.split(QLatin1Char('.'), Qt::KeepEmptyParts);
        for (const QString &identifier : identifiers) {
            if (identifier.isEmpty()
                || !QRegularExpression(QStringLiteral("^[0-9A-Za-z-]+$"))
                        .match(identifier)
                        .hasMatch()) {
                setError(
                    error,
                    tr("Invalid prerelease identifier: %1").arg(identifier));
                return std::nullopt;
            }

            bool numeric = false;
            const quint64 number = identifier.toULongLong(&numeric);
            parsed.prerelease.append(VersionIdentifier{
                .text = identifier,
                .numeric = numeric,
                .number = numeric ? number : 0,
            });
        }
    }

    return parsed;
}

int compareParsedVersions(const ParsedVersion &left, const ParsedVersion &right)
{
    const qsizetype coreCount = std::max(left.core.size(), right.core.size());
    for (qsizetype index = 0; index < coreCount; ++index) {
        const quint64 leftPart =
            index < left.core.size() ? left.core.at(index) : 0;
        const quint64 rightPart =
            index < right.core.size() ? right.core.at(index) : 0;
        if (leftPart != rightPart) {
            return leftPart < rightPart ? -1 : 1;
        }
    }

    if (left.prerelease.isEmpty() != right.prerelease.isEmpty()) {
        return left.prerelease.isEmpty() ? 1 : -1;
    }

    const qsizetype prereleaseCount =
        std::min(left.prerelease.size(), right.prerelease.size());
    for (qsizetype index = 0; index < prereleaseCount; ++index) {
        const VersionIdentifier &leftPart = left.prerelease.at(index);
        const VersionIdentifier &rightPart = right.prerelease.at(index);
        if (leftPart.numeric && rightPart.numeric) {
            if (leftPart.number != rightPart.number) {
                return leftPart.number < rightPart.number ? -1 : 1;
            }
            continue;
        }
        if (leftPart.numeric != rightPart.numeric) {
            return leftPart.numeric ? -1 : 1;
        }

        const int comparison = QString::compare(
            leftPart.text,
            rightPart.text,
            Qt::CaseInsensitive);
        if (comparison != 0) {
            return comparison < 0 ? -1 : 1;
        }
    }

    if (left.prerelease.size() == right.prerelease.size()) {
        return 0;
    }
    return left.prerelease.size() < right.prerelease.size() ? -1 : 1;
}

QStringList normalizedPrograms(const QStringList &programs)
{
    QStringList normalized;
    normalized.reserve(programs.size());
    for (const QString &program : programs) {
        const QString name = QFileInfo(program).fileName().toLower();
        if (!name.isEmpty() && !normalized.contains(name)) {
            normalized.append(name);
        }
    }
    return normalized;
}

QString unquoteOsReleaseValue(QString value)
{
    value = value.trimmed();
    if (value.size() >= 2
        && ((value.startsWith(QLatin1Char('"'))
             && value.endsWith(QLatin1Char('"')))
            || (value.startsWith(QLatin1Char('\''))
                && value.endsWith(QLatin1Char('\''))))) {
        value = value.mid(1, value.size() - 2);
    }
    return value.toLower();
}

QStringList osReleaseFamilies(QStringView contents)
{
    QStringList values;
    const QStringList lines = contents.toString().split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const qsizetype separator = line.indexOf(QLatin1Char('='));
        if (separator <= 0) {
            continue;
        }
        const QString key = line.left(separator).trimmed();
        if (key != QStringLiteral("ID") && key != QStringLiteral("ID_LIKE")) {
            continue;
        }
        values.append(unquoteOsReleaseValue(line.mid(separator + 1))
                          .split(
                              QRegularExpression(QStringLiteral("\\s+")),
                              Qt::SkipEmptyParts));
    }
    return values;
}

QString
firstAvailable(const QStringList &available, const QStringList &preferred)
{
    for (const QString &candidate : preferred) {
        if (available.contains(candidate)) {
            return candidate;
        }
    }
    return {};
}

bool isSystemExecutablePath(QStringView path)
{
    const QString cleanPath = QDir::cleanPath(path.toString());
    return cleanPath.startsWith(QStringLiteral("/usr/bin/"))
           || cleanPath.startsWith(QStringLiteral("/bin/"))
           || cleanPath.startsWith(QStringLiteral("/usr/sbin/"))
           || cleanPath.startsWith(QStringLiteral("/sbin/"));
}

QString originalProgramPath(
    const QStringList &availablePrograms, const QString &programName)
{
    for (const QString &candidate : availablePrograms) {
        if (QFileInfo(candidate).fileName().compare(
                programName,
                Qt::CaseInsensitive)
                == 0
            && isSystemExecutablePath(candidate)) {
            return QDir::cleanPath(candidate);
        }
    }
    return {};
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

QString packageExtension(PackageFormat format)
{
    switch (format) {
    case PackageFormat::Deb:
        return QStringLiteral(".deb");
    case PackageFormat::Rpm:
        return QStringLiteral(".rpm");
    case PackageFormat::Unknown:
        return {};
    }
    return {};
}

QString architecturePattern(CpuArchitecture architecture)
{
    switch (architecture) {
    case CpuArchitecture::X86_64:
        return QStringLiteral("(?:amd64|x86[_-]64)");
    case CpuArchitecture::Arm64:
        return QStringLiteral("(?:arm64|aarch64)");
    case CpuArchitecture::Unknown:
        return {};
    }
    return {};
}

int architectureConventionScore(
    QStringView name, PackageFormat format, CpuArchitecture architecture)
{
    const QString lower = name.toString().toLower();
    if (architecture == CpuArchitecture::X86_64) {
        if (format == PackageFormat::Deb
            && lower.contains(QStringLiteral("amd64"))) {
            return 2;
        }
        if (format == PackageFormat::Rpm
            && lower.contains(QStringLiteral("x86_64"))) {
            return 2;
        }
    } else if (architecture == CpuArchitecture::Arm64) {
        if (format == PackageFormat::Deb
            && lower.contains(QStringLiteral("arm64"))) {
            return 2;
        }
        if (format == PackageFormat::Rpm
            && lower.contains(QStringLiteral("aarch64"))) {
            return 2;
        }
    }
    return 1;
}

QString normalizedProductName(QStringView value)
{
    QString result = value.toString().toLower();
    result.remove(QRegularExpression(QStringLiteral("[^a-z0-9]")));
    return result;
}

QString shellQuote(QString value)
{
    if (value.isEmpty()) {
        return QStringLiteral("''");
    }
    if (QRegularExpression(QStringLiteral("^[A-Za-z0-9_@%+=:,./-]+$"))
            .match(value)
            .hasMatch()) {
        return value;
    }
    value.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QLatin1Char('\'') + value + QLatin1Char('\'');
}

bool isValidSha256(const QByteArray &value)
{
    static const QRegularExpression sha256Pattern(
        QStringLiteral("^[0-9a-fA-F]{64}$"));
    return sha256Pattern.match(QString::fromLatin1(value)).hasMatch();
}

bool isPlainAssetName(QStringView name)
{
    static const QRegularExpression safeName(
        QStringLiteral("^[A-Za-z0-9][A-Za-z0-9._+-]{0,199}$"));
    return safeName.match(name.toString()).hasMatch();
}

} // namespace

bool isAllowedGitHubUrl(const QUrl &url)
{
    if (!url.isValid() || url.scheme() != QStringLiteral("https")
        || !url.userInfo().isEmpty()
        || (url.port() != -1 && url.port() != 443)) {
        return false;
    }
    const QString host = url.host().toLower();
    return host == QStringLiteral("api.github.com")
           || host == QStringLiteral("github.com")
           || host == QStringLiteral("objects.githubusercontent.com")
           || host == QStringLiteral("release-assets.githubusercontent.com");
}

std::optional<GitHubRelease>
parseGitHubRelease(const QByteArray &json, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setError(
            error,
            tr("Invalid GitHub release JSON: %1")
                .arg(parseError.errorString()));
        return std::nullopt;
    }

    const QJsonObject object = document.object();
    const QString tagName =
        object.value(QStringLiteral("tag_name")).toString().trimmed();
    if (tagName.isEmpty()) {
        setError(error, tr("GitHub release has no tag_name."));
        return std::nullopt;
    }
    if (!parseVersion(tagName, error).has_value()) {
        return std::nullopt;
    }

    GitHubRelease release{
        .tagName = tagName,
        .version = normalizedVersionText(tagName),
        .name = object.value(QStringLiteral("name")).toString(),
        .pageUrl = QUrl(object.value(QStringLiteral("html_url")).toString()),
        .assets = {},
        .draft = object.value(QStringLiteral("draft")).toBool(),
        .prerelease = object.value(QStringLiteral("prerelease")).toBool(),
    };

    const QJsonValue assetsValue = object.value(QStringLiteral("assets"));
    if (!assetsValue.isArray()) {
        setError(error, tr("GitHub release has no assets array."));
        return std::nullopt;
    }

    for (const QJsonValue &assetValue : assetsValue.toArray()) {
        if (!assetValue.isObject()) {
            setError(error, tr("GitHub release contains an invalid asset."));
            return std::nullopt;
        }
        const QJsonObject assetObject = assetValue.toObject();
        const QString name =
            assetObject.value(QStringLiteral("name")).toString().trimmed();
        const QUrl downloadUrl(
            assetObject.value(QStringLiteral("browser_download_url"))
                .toString());
        const qint64 size =
            assetObject.value(QStringLiteral("size")).toInteger(-1);
        if (!isPlainAssetName(name) || !isAllowedGitHubUrl(downloadUrl)
            || size < 0) {
            setError(error, tr("GitHub release contains an invalid asset."));
            return std::nullopt;
        }
        release.assets.append(ReleaseAsset{
            .name = name,
            .downloadUrl = downloadUrl,
            .size = size,
        });
    }

    return release;
}

std::optional<int>
compareVersions(QStringView left, QStringView right, QString *error)
{
    const auto parsedLeft = parseVersion(left, error);
    if (!parsedLeft.has_value()) {
        return std::nullopt;
    }
    const auto parsedRight = parseVersion(right, error);
    if (!parsedRight.has_value()) {
        return std::nullopt;
    }
    return compareParsedVersions(*parsedLeft, *parsedRight);
}

bool isNewerStableRelease(
    const GitHubRelease &release, QStringView currentVersion, QString *error)
{
    if (release.draft || release.prerelease) {
        setError(error, tr("The release is not a stable published release."));
        return false;
    }
    const auto parsedReleaseVersion = parseVersion(release.version, error);
    if (!parsedReleaseVersion.has_value()) {
        return false;
    }
    if (!parsedReleaseVersion->prerelease.isEmpty()) {
        setError(error, tr("The release version is a prerelease."));
        return false;
    }
    const auto comparison =
        compareVersions(release.version, currentVersion, error);
    return comparison.has_value() && *comparison > 0;
}

CpuArchitecture normalizeArchitecture(QStringView architecture)
{
    const QString value = architecture.trimmed().toString().toLower();
    if (value == QStringLiteral("x86_64") || value == QStringLiteral("amd64")
        || value == QStringLiteral("x64")) {
        return CpuArchitecture::X86_64;
    }
    if (value == QStringLiteral("aarch64")
        || value == QStringLiteral("arm64")) {
        return CpuArchitecture::Arm64;
    }
    return CpuArchitecture::Unknown;
}

PlatformInfo detectPlatform(const PlatformProbe &probe)
{
    const QStringList programs = normalizedPrograms(probe.availablePrograms);
    const QStringList families = osReleaseFamilies(probe.osReleaseText);

    const QStringList debianFamilies{
        QStringLiteral("debian"),
        QStringLiteral("ubuntu"),
        QStringLiteral("linuxmint"),
        QStringLiteral("pop"),
    };
    const QStringList rpmFamilies{
        QStringLiteral("fedora"),
        QStringLiteral("rhel"),
        QStringLiteral("centos"),
        QStringLiteral("rocky"),
        QStringLiteral("almalinux"),
        QStringLiteral("suse"),
        QStringLiteral("opensuse"),
    };

    const bool debianFamily = std::any_of(
        families.cbegin(),
        families.cend(),
        [&debianFamilies](const QString &family) {
            return debianFamilies.contains(family);
        });
    const bool rpmFamily = std::any_of(
        families.cbegin(),
        families.cend(),
        [&rpmFamilies](const QString &family) {
            return rpmFamilies.contains(family);
        });

    PlatformInfo result{
        .packageFormat = PackageFormat::Unknown,
        .architecture = normalizeArchitecture(probe.cpuArchitecture),
        .installerProgram = {},
        .systemPackageInstall = isSystemExecutablePath(probe.executablePath),
    };

    if (debianFamily
        || (!rpmFamily
            && (programs.contains(QStringLiteral("apt-get"))
                || programs.contains(QStringLiteral("dpkg"))))) {
        result.packageFormat = PackageFormat::Deb;
        const QString installerName = firstAvailable(
            programs,
            {QStringLiteral("apt-get"),
             QStringLiteral("apt"),
             QStringLiteral("dpkg")});
        result.installerProgram =
            originalProgramPath(probe.availablePrograms, installerName);
    } else if (
        rpmFamily || programs.contains(QStringLiteral("dnf5"))
        || programs.contains(QStringLiteral("dnf"))
        || programs.contains(QStringLiteral("zypper"))
        || programs.contains(QStringLiteral("rpm"))) {
        result.packageFormat = PackageFormat::Rpm;
        const QString installerName = firstAvailable(
            programs,
            {
                QStringLiteral("dnf5"),
                QStringLiteral("dnf"),
                QStringLiteral("zypper"),
                QStringLiteral("rpm"),
            });
        result.installerProgram =
            originalProgramPath(probe.availablePrograms, installerName);
    }

    return result;
}

PlatformProbe probeCurrentPlatform()
{
    QFile osRelease(QStringLiteral("/etc/os-release"));
    QByteArray osReleaseContents;
    if (osRelease.open(QIODevice::ReadOnly)) {
        osReleaseContents = osRelease.readAll();
    }

    const QStringList candidates{
        QStringLiteral("apt-get"),
        QStringLiteral("apt"),
        QStringLiteral("dpkg"),
        QStringLiteral("dnf5"),
        QStringLiteral("dnf"),
        QStringLiteral("zypper"),
        QStringLiteral("rpm"),
    };
    QStringList programs;
    for (const QString &candidate : candidates) {
        const QString path = findSystemExecutable(candidate);
        if (!path.isEmpty()) {
            programs.append(path);
        }
    }

    return PlatformProbe{
        .osReleaseText = QString::fromUtf8(osReleaseContents),
        .cpuArchitecture = QSysInfo::currentCpuArchitecture(),
        .availablePrograms = programs,
        .executablePath = QCoreApplication::applicationFilePath(),
    };
}

std::optional<ReleaseAsset> selectPackageAsset(
    const GitHubRelease &release,
    const PlatformInfo &platform,
    QStringView productName,
    QString *error)
{
    const QString extension = packageExtension(platform.packageFormat);
    const QString architecture = architecturePattern(platform.architecture);
    const QString product = normalizedProductName(productName);
    if (extension.isEmpty() || architecture.isEmpty() || product.isEmpty()) {
        setError(error, tr("The platform or product is unsupported."));
        return std::nullopt;
    }

    const QRegularExpression architectureExpression(
        QStringLiteral("(^|[^a-z0-9])%1([^a-z0-9]|$)").arg(architecture),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression nonRuntimePackageExpression(
        QStringLiteral(
            "(^|[^a-z0-9])(?:debug|debuginfo|dbgsym|devel|source|src)"
            "([^a-z0-9]|$)"),
        QRegularExpression::CaseInsensitiveOption);

    QList<ReleaseAsset> bestMatches;
    int bestScore = 0;
    for (const ReleaseAsset &asset : release.assets) {
        const QString lowerName = asset.name.toLower();
        if (!lowerName.endsWith(extension)
            || (platform.packageFormat == PackageFormat::Rpm
                && lowerName.endsWith(QStringLiteral(".src.rpm")))) {
            continue;
        }
        QString normalizedName = lowerName;
        normalizedName.remove(QRegularExpression(QStringLiteral("[^a-z0-9]")));
        if (!normalizedName.startsWith(product)
            || nonRuntimePackageExpression.match(lowerName).hasMatch()
            || !architectureExpression.match(lowerName).hasMatch()) {
            continue;
        }

        const int score = architectureConventionScore(
            lowerName,
            platform.packageFormat,
            platform.architecture);
        if (score > bestScore) {
            bestScore = score;
            bestMatches = {asset};
        } else if (score == bestScore) {
            bestMatches.append(asset);
        }
    }

    if (bestMatches.isEmpty()) {
        setError(error, tr("No package matches this platform."));
        return std::nullopt;
    }
    if (bestMatches.size() != 1) {
        setError(error, tr("More than one package matches this platform."));
        return std::nullopt;
    }
    return bestMatches.constFirst();
}

std::optional<ReleaseAsset> selectChecksumAsset(
    const GitHubRelease &release, QStringView packageAssetName, QString *error)
{
    const QString packageName = packageAssetName.toString();
    const QString exactName = packageName + QStringLiteral(".sha256");

    for (const ReleaseAsset &asset : release.assets) {
        if (asset.name.compare(exactName, Qt::CaseInsensitive) == 0) {
            return asset;
        }
    }

    const QStringList preferred{
        QStringLiteral("SHA256SUMS"),
        QStringLiteral("SHA256SUMS.txt"),
        QStringLiteral("checksums.sha256"),
        QStringLiteral("checksums.txt"),
    };
    for (const QString &candidate : preferred) {
        for (const ReleaseAsset &asset : release.assets) {
            if (asset.name.compare(candidate, Qt::CaseInsensitive) == 0) {
                return asset;
            }
        }
    }

    setError(error, tr("The release has no SHA-256 checksum asset."));
    return std::nullopt;
}

std::optional<UpgradeSelection> selectUpgrade(
    const GitHubRelease &release,
    QStringView currentVersion,
    const PlatformInfo &platform,
    QStringView productName,
    QString *error)
{
    if (!isNewerStableRelease(release, currentVersion, error)) {
        return std::nullopt;
    }
    const auto package =
        selectPackageAsset(release, platform, productName, error);
    if (!package.has_value()) {
        return std::nullopt;
    }
    const auto checksum = selectChecksumAsset(release, package->name, error);
    if (!checksum.has_value()) {
        return std::nullopt;
    }
    return UpgradeSelection{
        .release = release,
        .packageAsset = *package,
        .checksumAsset = *checksum,
    };
}

std::optional<PackageMetadata>
parsePackageMetadata(const QByteArray &output, QString *error)
{
    QStringList lines = QString::fromUtf8(output).split(QLatin1Char('\n'));
    while (!lines.isEmpty() && lines.constLast().isEmpty()) {
        lines.removeLast();
    }
    for (QString &line : lines) {
        if (line.endsWith(QLatin1Char('\r'))) {
            line.chop(1);
        }
    }
    if (lines.size() != 3
        || std::ranges::any_of(lines, [](const QString &line) {
               return line.trimmed().isEmpty() || line != line.trimmed();
           })) {
        setError(error, tr("The package metadata response is invalid."));
        return std::nullopt;
    }
    return PackageMetadata{
        .name = lines.at(0),
        .version = lines.at(1),
        .architecture = lines.at(2),
    };
}

bool packageMetadataMatches(
    const PackageMetadata &metadata,
    QStringView releaseVersion,
    const PlatformInfo &platform,
    QStringView productName,
    QString *error)
{
    if (metadata.name != productName) {
        setError(
            error,
            tr("The package name is %1, expected %2.")
                .arg(metadata.name, productName));
        return false;
    }

    const QString version = normalizedVersionText(releaseVersion);
    if (metadata.version != version
        && !metadata.version.startsWith(version + QLatin1Char('-'))) {
        setError(
            error,
            tr("The package version is %1, expected %2.")
                .arg(metadata.version, version));
        return false;
    }

    QString expectedArchitecture;
    if (platform.packageFormat == PackageFormat::Deb) {
        expectedArchitecture = platform.architecture == CpuArchitecture::X86_64
                                   ? QStringLiteral("amd64")
                               : platform.architecture == CpuArchitecture::Arm64
                                   ? QStringLiteral("arm64")
                                   : QString();
    } else if (platform.packageFormat == PackageFormat::Rpm) {
        expectedArchitecture = platform.architecture == CpuArchitecture::X86_64
                                   ? QStringLiteral("x86_64")
                               : platform.architecture == CpuArchitecture::Arm64
                                   ? QStringLiteral("aarch64")
                                   : QString();
    }
    if (expectedArchitecture.isEmpty()
        || metadata.architecture != expectedArchitecture) {
        setError(
            error,
            tr("The package architecture is %1, expected %2.")
                .arg(metadata.architecture, expectedArchitecture));
        return false;
    }
    return true;
}

std::optional<Sha256Manifest>
parseSha256Manifest(const QByteArray &contents, QString *error)
{
    static const QRegularExpression gnuPattern(
        QStringLiteral("^([0-9a-fA-F]{64})(?:[ \\t]+[* ]?(.+))?$"));
    static const QRegularExpression bsdPattern(
        QStringLiteral("^SHA256 \\((.+)\\) = ([0-9a-fA-F]{64})$"),
        QRegularExpression::CaseInsensitiveOption);

    Sha256Manifest manifest;
    const QList<QByteArray> lines = contents.split('\n');
    for (QByteArray lineBytes : lines) {
        if (lineBytes.endsWith('\r')) {
            lineBytes.chop(1);
        }
        const QString line = QString::fromUtf8(lineBytes).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }

        QString fileName;
        QByteArray checksum;
        const QRegularExpressionMatch gnuMatch = gnuPattern.match(line);
        const QRegularExpressionMatch bsdMatch = bsdPattern.match(line);
        if (gnuMatch.hasMatch()) {
            checksum = gnuMatch.captured(1).toLatin1().toLower();
            fileName = gnuMatch.captured(2).trimmed();
        } else if (bsdMatch.hasMatch()) {
            fileName = bsdMatch.captured(1).trimmed();
            checksum = bsdMatch.captured(2).toLatin1().toLower();
        } else {
            setError(error, tr("Invalid SHA-256 manifest line: %1").arg(line));
            return std::nullopt;
        }

        if (fileName.contains(QLatin1Char('\0'))) {
            setError(
                error,
                tr("SHA-256 manifest contains an invalid file name."));
            return std::nullopt;
        }
        const auto existing = manifest.constFind(fileName);
        if (existing != manifest.cend() && *existing != checksum) {
            setError(
                error,
                tr("SHA-256 manifest has conflicting entries for %1.")
                    .arg(fileName));
            return std::nullopt;
        }
        manifest.insert(fileName, checksum);
    }

    if (manifest.isEmpty()) {
        setError(error, tr("SHA-256 manifest is empty."));
        return std::nullopt;
    }
    return manifest;
}

std::optional<QByteArray> checksumForAsset(
    const Sha256Manifest &manifest, QStringView assetName, QString *error)
{
    const QString requestedName = assetName.toString();
    const auto exact = manifest.constFind(requestedName);
    if (exact != manifest.cend()) {
        return *exact;
    }

    QList<QByteArray> basenameMatches;
    for (auto iterator = manifest.cbegin(); iterator != manifest.cend();
         ++iterator) {
        if (!iterator.key().isEmpty()
            && QFileInfo(iterator.key()).fileName() == requestedName) {
            basenameMatches.append(iterator.value());
        }
    }
    if (basenameMatches.size() == 1) {
        return basenameMatches.constFirst();
    }
    if (basenameMatches.size() > 1) {
        setError(error, tr("SHA-256 manifest has ambiguous file names."));
        return std::nullopt;
    }

    const auto unnamed = manifest.constFind(QString{});
    if (manifest.size() == 1 && unnamed != manifest.cend()) {
        return *unnamed;
    }

    setError(
        error,
        tr("SHA-256 manifest has no entry for %1.").arg(requestedName));
    return std::nullopt;
}

VerificationResult verifyFileSha256(
    const QString &path,
    const QByteArray &expectedHex,
    QByteArray *actualHex,
    QString *error)
{
    if (!isValidSha256(expectedHex)) {
        setError(error, tr("Expected SHA-256 checksum is invalid."));
        return VerificationResult::InvalidChecksum;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(error, file.errorString());
        return VerificationResult::FileError;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        setError(error, file.errorString());
        return VerificationResult::FileError;
    }
    const QByteArray actual = hash.result().toHex();
    if (actualHex != nullptr) {
        *actualHex = actual;
    }
    return actual.compare(expectedHex, Qt::CaseInsensitive) == 0
               ? VerificationResult::Match
               : VerificationResult::Mismatch;
}

std::optional<InstallPlan> createInstallPlan(
    const PlatformInfo &platform,
    const QString &packagePath,
    VerificationResult verification,
    QString *error)
{
    if (verification != VerificationResult::Match) {
        setError(error, tr("The package has not passed SHA-256 verification."));
        return std::nullopt;
    }

    const QString extension = packageExtension(platform.packageFormat);
    const QString cleanPath = QDir::cleanPath(packagePath);
    if (extension.isEmpty() || platform.installerProgram.isEmpty()
        || !isSystemExecutablePath(platform.installerProgram)) {
        setError(
            error,
            tr("No supported system package installer is available."));
        return std::nullopt;
    }
    if (!QDir::isAbsolutePath(cleanPath)
        || !cleanPath.endsWith(extension, Qt::CaseInsensitive)) {
        setError(
            error,
            tr("Package path is not an absolute %1 file.").arg(extension));
        return std::nullopt;
    }

    const QString program =
        QFileInfo(platform.installerProgram).fileName().toLower();
    QStringList arguments;
    if (program == QStringLiteral("apt-get") || program == QStringLiteral("apt")
        || program == QStringLiteral("dnf5") || program == QStringLiteral("dnf")
        || program == QStringLiteral("zypper")) {
        arguments = {QStringLiteral("install"), cleanPath};
    } else if (program == QStringLiteral("dpkg")) {
        arguments = {QStringLiteral("--install"), cleanPath};
    } else if (program == QStringLiteral("rpm")) {
        arguments = {QStringLiteral("--upgrade"), cleanPath};
    } else {
        setError(
            error,
            tr("Unsupported system package installer: %1").arg(program));
        return std::nullopt;
    }

    return InstallPlan{
        .program = platform.installerProgram,
        .arguments = arguments,
        .packagePath = cleanPath,
        .packageFormat = platform.packageFormat,
        .requiresAdministrator = true,
        .explicitConfirmationRequired = true,
        .packageManagedInstall = platform.systemPackageInstall,
    };
}

InstallReadiness assessInstallReadiness(
    const InstallPlan &plan,
    bool explicitlyConfirmed,
    bool hasAdministratorPrivileges)
{
    if (plan.explicitConfirmationRequired && !explicitlyConfirmed) {
        return InstallReadiness::AwaitingConfirmation;
    }
    if (plan.requiresAdministrator && !hasAdministratorPrivileges) {
        return InstallReadiness::RequiresAdministrator;
    }
    return InstallReadiness::Ready;
}

QString displayInstallCommand(const InstallPlan &plan)
{
    QStringList parts{shellQuote(plan.program)};
    for (const QString &argument : plan.arguments) {
        parts.append(shellQuote(argument));
    }
    return parts.join(QLatin1Char(' '));
}

} // namespace msikeyboard::update
