#include "update/UpdaterCore.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>
#include <QTest>

using namespace msikeyboard::update;

class UpdaterCoreTest final : public QObject {
    Q_OBJECT

private slots:
    void parsesGitHubRelease();
    void rejectsInsecureReleaseAsset();
    void rejectsNonGitHubReleaseAsset();
    void rejectsAssetPath();
    void validatesGitHubUrls();
    void comparesVersions_data();
    void comparesVersions();
    void detectsDebianAndRpmPlatforms();
    void selectsNativePackageAndChecksum();
    void rejectsAmbiguousNativePackages();
    void rejectsDebugPackage();
    void rejectsPrereleaseUpgrade();
    void validatesPackageMetadata();
    void parsesAndResolvesChecksums();
    void rejectsConflictingChecksums();
    void verifiesDownloadedFile();
    void requiresConfirmationAndExistingPrivileges();
};

namespace {

ReleaseAsset asset(const QString &name)
{
    return ReleaseAsset{
        .name = name,
        .downloadUrl = QUrl(QStringLiteral("https://example.test/") + name),
        .size = 42,
    };
}

GitHubRelease stableRelease()
{
    return GitHubRelease{
        .tagName = QStringLiteral("v0.2.0"),
        .version = QStringLiteral("0.2.0"),
        .name = QStringLiteral("0.2.0"),
        .pageUrl = QUrl(QStringLiteral("https://github.com/kordax/msi-keyboard/releases/0.2.0")),
        .assets = {
            asset(QStringLiteral("msi-keyboard_0.2.0-1_amd64.deb")),
            asset(QStringLiteral("msi-keyboard_0.2.0-1_arm64.deb")),
            asset(QStringLiteral("msi-keyboard-0.2.0-1.x86_64.rpm")),
            asset(QStringLiteral("msi-keyboard-0.2.0-1.aarch64.rpm")),
            asset(QStringLiteral("SHA256SUMS")),
        },
        .draft = false,
        .prerelease = false,
    };
}

} // namespace

void UpdaterCoreTest::parsesGitHubRelease()
{
    const QByteArray json = R"json({
        "tag_name": "v1.2.3",
        "name": "MSI Keyboard 1.2.3",
        "html_url": "https://github.com/kordax/msi-keyboard/releases/tag/v1.2.3",
        "draft": false,
        "prerelease": false,
        "assets": [{
            "name": "msi-keyboard_1.2.3_amd64.deb",
            "browser_download_url": "https://github.com/kordax/msi-keyboard/releases/download/v1.2.3/msi-keyboard.deb",
            "size": 1234
        }]
    })json";

    QString error;
    const auto release = parseGitHubRelease(json, &error);
    QVERIFY2(release.has_value(), qPrintable(error));
    QCOMPARE(release->tagName, QStringLiteral("v1.2.3"));
    QCOMPARE(release->version, QStringLiteral("1.2.3"));
    QCOMPARE(release->assets.size(), 1);
    QCOMPARE(release->assets.constFirst().size, 1234);
}

void UpdaterCoreTest::rejectsInsecureReleaseAsset()
{
    const QByteArray json = R"json({
        "tag_name": "1.0.0",
        "assets": [{
            "name": "msi-keyboard_1.0.0_amd64.deb",
            "browser_download_url": "http://example.test/package.deb",
            "size": 1234
        }]
    })json";

    QVERIFY(!parseGitHubRelease(json).has_value());
}

void UpdaterCoreTest::rejectsNonGitHubReleaseAsset()
{
    const QByteArray json = R"json({
        "tag_name": "1.0.0",
        "assets": [{
            "name": "msi-keyboard_1.0.0_amd64.deb",
            "browser_download_url": "https://downloads.example.test/package.deb",
            "size": 1234
        }]
    })json";

    QVERIFY(!parseGitHubRelease(json).has_value());
}

void UpdaterCoreTest::rejectsAssetPath()
{
    const QList<QString> invalidNames{
        QStringLiteral("../msi-keyboard.deb"),
        QStringLiteral("packages/msi-keyboard.deb"),
        QStringLiteral("packages\\msi-keyboard.deb"),
        QStringLiteral("package\nname.deb"),
        QString::fromUtf8("package\u001bname.deb"),
        QString::fromUtf8("пакет.deb"),
        QStringLiteral("."),
        QStringLiteral(".."),
    };
    for (const QString &name : invalidNames) {
        const QByteArray json = QJsonDocument(QJsonObject{
                                                  {
                                                      QStringLiteral("tag_name"),
                                                      QStringLiteral("1.0.0"),
                                                  },
                                                  {
                                                      QStringLiteral("assets"),
                                                      QJsonArray{
                                                          QJsonObject{
                                                              {
                                                                  QStringLiteral("name"),
                                                                  name,
                                                              },
                                                              {
                                                                  QStringLiteral(
                                                                      "browser_download_url"),
                                                                  QStringLiteral(
                                                                      "https://github.com/kordax/"
                                                                      "msi-keyboard-app/releases/"
                                                                      "download/v1.0.0/package.deb"),
                                                              },
                                                              {
                                                                  QStringLiteral("size"),
                                                                  1234,
                                                              },
                                                          },
                                                      },
                                                  },
                                              })
                                    .toJson(QJsonDocument::Compact);
        QVERIFY2(!parseGitHubRelease(json).has_value(), qPrintable(name));
    }
}

void UpdaterCoreTest::validatesGitHubUrls()
{
    QVERIFY(isAllowedGitHubUrl(
        QUrl(QStringLiteral("https://api.github.com/repos/kordax/releases"))));
    QVERIFY(isAllowedGitHubUrl(QUrl(QStringLiteral(
        "https://release-assets.githubusercontent.com/package.deb"))));
    QVERIFY(!isAllowedGitHubUrl(
        QUrl(QStringLiteral("https://github.com.example.test/package.deb"))));
    QVERIFY(!isAllowedGitHubUrl(
        QUrl(QStringLiteral("https://user@github.com/package.deb"))));
    QVERIFY(!isAllowedGitHubUrl(
        QUrl(QStringLiteral("https://github.com:444/package.deb"))));
    QVERIFY(!isAllowedGitHubUrl(
        QUrl(QStringLiteral("http://github.com/package.deb"))));
}

void UpdaterCoreTest::comparesVersions_data()
{
    QTest::addColumn<QString>("left");
    QTest::addColumn<QString>("right");
    QTest::addColumn<int>("expected");

    QTest::newRow("minor") << QStringLiteral("1.10.0") << QStringLiteral("1.9.9") << 1;
    QTest::newRow("leading-v") << QStringLiteral("v2.0") << QStringLiteral("2.0.0") << 0;
    QTest::newRow("prerelease") << QStringLiteral("1.0.0-rc.1")
                                << QStringLiteral("1.0.0") << -1;
    QTest::newRow("numeric-prerelease") << QStringLiteral("1.0.0-rc.10")
                                        << QStringLiteral("1.0.0-rc.2") << 1;
    QTest::newRow("build-metadata") << QStringLiteral("1.2.3+linux")
                                    << QStringLiteral("1.2.3+other") << 0;
}

void UpdaterCoreTest::comparesVersions()
{
    QFETCH(QString, left);
    QFETCH(QString, right);
    QFETCH(int, expected);

    const auto comparison = compareVersions(left, right);
    QVERIFY(comparison.has_value());
    QCOMPARE(*comparison, expected);
}

void UpdaterCoreTest::detectsDebianAndRpmPlatforms()
{
    const PlatformInfo debian = detectPlatform(PlatformProbe{
        .osReleaseText = QStringLiteral("ID=linuxmint\nID_LIKE=\"ubuntu debian\"\n"),
        .cpuArchitecture = QStringLiteral("amd64"),
        .availablePrograms = {QStringLiteral("/usr/bin/apt-get"), QStringLiteral("dpkg")},
        .executablePath = QStringLiteral("/usr/bin/msi-keyboard"),
    });
    QCOMPARE(debian.packageFormat, PackageFormat::Deb);
    QCOMPARE(debian.architecture, CpuArchitecture::X86_64);
    QCOMPARE(debian.installerProgram, QStringLiteral("/usr/bin/apt-get"));
    QVERIFY(debian.systemPackageInstall);

    const PlatformInfo fedora = detectPlatform(PlatformProbe{
        .osReleaseText = QStringLiteral("ID=fedora\n"),
        .cpuArchitecture = QStringLiteral("aarch64"),
        .availablePrograms = {QStringLiteral("/usr/bin/dnf")},
        .executablePath = QStringLiteral("/home/user/msi-keyboard"),
    });
    QCOMPARE(fedora.packageFormat, PackageFormat::Rpm);
    QCOMPARE(fedora.architecture, CpuArchitecture::Arm64);
    QCOMPARE(fedora.installerProgram, QStringLiteral("/usr/bin/dnf"));
    QVERIFY(!fedora.systemPackageInstall);

    const PlatformInfo untrusted = detectPlatform(PlatformProbe{
        .osReleaseText = QStringLiteral("ID=debian\n"),
        .cpuArchitecture = QStringLiteral("amd64"),
        .availablePrograms = {QStringLiteral("/tmp/apt-get")},
        .executablePath = QStringLiteral("/tmp/msi-keyboard"),
    });
    QCOMPARE(untrusted.packageFormat, PackageFormat::Deb);
    QVERIFY(untrusted.installerProgram.isEmpty());
}

void UpdaterCoreTest::selectsNativePackageAndChecksum()
{
    const GitHubRelease release = stableRelease();
    const PlatformInfo debian{
        .packageFormat = PackageFormat::Deb,
        .architecture = CpuArchitecture::X86_64,
        .installerProgram = QStringLiteral("apt-get"),
    };
    QString error;
    const auto selection = selectUpgrade(
        release,
        QStringLiteral("0.1.0"),
        debian,
        QStringView(u"msi-keyboard"),
        &error);

    QVERIFY2(selection.has_value(), qPrintable(error));
    QCOMPARE(
        selection->packageAsset.name,
        QStringLiteral("msi-keyboard_0.2.0-1_amd64.deb"));
    QCOMPARE(selection->checksumAsset.name, QStringLiteral("SHA256SUMS"));
}

void UpdaterCoreTest::rejectsAmbiguousNativePackages()
{
    GitHubRelease release = stableRelease();
    release.assets.append(asset(QStringLiteral("msi-keyboard_0.2.0_linux_amd64.deb")));
    const PlatformInfo debian{
        .packageFormat = PackageFormat::Deb,
        .architecture = CpuArchitecture::X86_64,
        .installerProgram = QStringLiteral("apt-get"),
    };

    QVERIFY(!selectPackageAsset(release, debian).has_value());
}

void UpdaterCoreTest::rejectsDebugPackage()
{
    GitHubRelease release = stableRelease();
    release.assets = {
        asset(QStringLiteral("msi-keyboard-debuginfo-0.2.0-1.x86_64.rpm")),
        asset(QStringLiteral("SHA256SUMS")),
    };
    const PlatformInfo fedora{
        .packageFormat = PackageFormat::Rpm,
        .architecture = CpuArchitecture::X86_64,
        .installerProgram = QStringLiteral("dnf"),
    };

    QVERIFY(!selectPackageAsset(release, fedora).has_value());
}

void UpdaterCoreTest::rejectsPrereleaseUpgrade()
{
    GitHubRelease release = stableRelease();
    release.version = QStringLiteral("0.3.0-rc.1");
    release.prerelease = false;

    QVERIFY(!isNewerStableRelease(release, QStringLiteral("0.1.0")));
}

void UpdaterCoreTest::validatesPackageMetadata()
{
    QString error;
    const auto metadata = parsePackageMetadata(
        QByteArrayLiteral("msi-keyboard\n0.2.0-1\namd64\n"),
        &error);
    QVERIFY2(metadata.has_value(), qPrintable(error));

    const PlatformInfo platform{
        .packageFormat = PackageFormat::Deb,
        .architecture = CpuArchitecture::X86_64,
        .installerProgram = QStringLiteral("/usr/bin/apt-get"),
    };
    QVERIFY(packageMetadataMatches(
        *metadata,
        QStringView(u"0.2.0"),
        platform,
        QStringView(u"msi-keyboard"),
        &error));

    PackageMetadata wrong = *metadata;
    wrong.version = QStringLiteral("0.1.0-1");
    QVERIFY(!packageMetadataMatches(
        wrong,
        QStringView(u"0.2.0"),
        platform));
    wrong = *metadata;
    wrong.name = QStringLiteral("other-package");
    QVERIFY(!packageMetadataMatches(
        wrong,
        QStringView(u"0.2.0"),
        platform));
    wrong = *metadata;
    wrong.architecture = QStringLiteral("arm64");
    QVERIFY(!packageMetadataMatches(
        wrong,
        QStringView(u"0.2.0"),
        platform));
    QVERIFY(!parsePackageMetadata(
                 QByteArrayLiteral("msi-keyboard\n0.2.0-1\n"))
                 .has_value());
}

void UpdaterCoreTest::parsesAndResolvesChecksums()
{
    const QByteArray first(64, 'a');
    const QByteArray second(64, 'b');
    const QByteArray contents =
        first + "  msi-keyboard_0.2.0_amd64.deb\nSHA256 (packages/other.rpm) = "
        + second + '\n';

    QString error;
    const auto manifest = parseSha256Manifest(contents, &error);
    QVERIFY2(manifest.has_value(), qPrintable(error));
    QCOMPARE(
        checksumForAsset(*manifest, QStringLiteral("msi-keyboard_0.2.0_amd64.deb")),
        std::optional<QByteArray>(first));
    QCOMPARE(
        checksumForAsset(*manifest, QStringLiteral("other.rpm")),
        std::optional<QByteArray>(second));
}

void UpdaterCoreTest::rejectsConflictingChecksums()
{
    const QByteArray contents =
        QByteArray(64, 'a') + "  package.deb\n" + QByteArray(64, 'b')
        + "  package.deb\n";

    QVERIFY(!parseSha256Manifest(contents).has_value());
}

void UpdaterCoreTest::verifiesDownloadedFile()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    QCOMPARE(file.write("verified package"), 16);
    file.close();

    const QByteArray expected =
        QCryptographicHash::hash("verified package", QCryptographicHash::Sha256).toHex();
    QByteArray actual;
    QCOMPARE(
        verifyFileSha256(file.fileName(), expected, &actual),
        VerificationResult::Match);
    QCOMPARE(actual, expected);
    QCOMPARE(
        verifyFileSha256(file.fileName(), QByteArray(64, '0')),
        VerificationResult::Mismatch);
}

void UpdaterCoreTest::requiresConfirmationAndExistingPrivileges()
{
    const PlatformInfo platform{
        .packageFormat = PackageFormat::Deb,
        .architecture = CpuArchitecture::X86_64,
        .installerProgram = QStringLiteral("/usr/bin/apt-get"),
        .systemPackageInstall = true,
    };
    QString error;
    QVERIFY(!createInstallPlan(
                 platform,
                 QStringLiteral("/tmp/MSI Keyboard.deb"),
                 VerificationResult::Mismatch)
                 .has_value());
    PlatformInfo untrustedPlatform = platform;
    untrustedPlatform.installerProgram = QStringLiteral("/tmp/apt-get");
    QVERIFY(!createInstallPlan(
                 untrustedPlatform,
                 QStringLiteral("/tmp/MSI Keyboard.deb"),
                 VerificationResult::Match)
                 .has_value());
    const auto plan = createInstallPlan(
        platform,
        QStringLiteral("/tmp/MSI Keyboard.deb"),
        VerificationResult::Match,
        &error);
    QVERIFY2(plan.has_value(), qPrintable(error));
    QVERIFY(plan->explicitConfirmationRequired);
    QVERIFY(plan->requiresAdministrator);
    QVERIFY(plan->packageManagedInstall);
    QVERIFY(!plan->program.contains(QStringLiteral("sudo")));
    QVERIFY(!plan->program.contains(QStringLiteral("pkexec")));
    QVERIFY(!plan->arguments.contains(QStringLiteral("-y")));
    QCOMPARE(
        assessInstallReadiness(*plan, false, false),
        InstallReadiness::AwaitingConfirmation);
    QCOMPARE(
        assessInstallReadiness(*plan, true, false),
        InstallReadiness::RequiresAdministrator);
    QCOMPARE(
        assessInstallReadiness(*plan, true, true),
        InstallReadiness::Ready);
    QCOMPARE(
        displayInstallCommand(*plan),
        QStringLiteral("/usr/bin/apt-get install '/tmp/MSI Keyboard.deb'"));
}

QTEST_MAIN(UpdaterCoreTest)
#include "UpdaterCoreTest.moc"
