#include "gui/SingleInstance.h"

#include <QSignalSpy>
#include <QUuid>
#include <QtTest>

using strikepro::SingleInstance;

namespace {

QString uniqueServerName()
{
    return QStringLiteral("msi-keyboard-test-%1")
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

} // namespace

class SingleInstanceTest final : public QObject {
    Q_OBJECT

  private slots:
    void secondInstanceActivatesPrimary()
    {
        const QString serverName = uniqueServerName();
        SingleInstance primary(serverName);
        QCOMPARE(primary.start(), SingleInstance::Role::Primary);
        QSignalSpy activationRequested(
            &primary,
            &SingleInstance::activationRequested);

        SingleInstance secondary(serverName);
        QCOMPARE(secondary.start(), SingleInstance::Role::Secondary);
        QTRY_COMPARE(activationRequested.count(), 1);

        SingleInstance third(serverName);
        QCOMPARE(third.start(), SingleInstance::Role::Secondary);
        QTRY_COMPARE(activationRequested.count(), 2);
    }

    void releasesServerOnShutdown()
    {
        const QString serverName = uniqueServerName();
        {
            SingleInstance primary(serverName);
            QCOMPARE(primary.start(), SingleInstance::Role::Primary);
        }

        SingleInstance replacement(serverName);
        QCOMPARE(replacement.start(), SingleInstance::Role::Primary);
    }
};

QTEST_GUILESS_MAIN(SingleInstanceTest)

#include "SingleInstanceTest.moc"
