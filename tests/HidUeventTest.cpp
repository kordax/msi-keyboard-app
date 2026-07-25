#include "device/HidUevent.h"

#include <QtTest>

namespace {

QByteArray makeEvent(std::initializer_list<QByteArray> fields)
{
    QByteArray event;
    for (const QByteArray &field : fields) {
        event.append(field);
        event.append('\0');
    }
    return event;
}

} // namespace

class HidUeventTest final : public QObject {
    Q_OBJECT

  private slots:
    void acceptsExactHidIdentifier()
    {
        const QByteArray event = makeEvent({
            QByteArrayLiteral("add@/devices/virtual/hidraw/hidraw4"),
            QByteArrayLiteral("ACTION=add"),
            QByteArrayLiteral("SUBSYSTEM=hidraw"),
            QByteArrayLiteral("HID_ID=0003:00000DB0:00001620"),
        });

        QVERIFY(strikepro::isSupportedDeviceUevent(event));
    }

    void acceptsExactUsbProductIdentifier()
    {
        const QByteArray event = makeEvent({
            QByteArrayLiteral("change@/devices/pci/usb1/1-2"),
            QByteArrayLiteral("ACTION=change"),
            QByteArrayLiteral("SUBSYSTEM=usb"),
            QByteArrayLiteral("PRODUCT=0db0/b231/0100"),
        });

        QVERIFY(strikepro::isSupportedDeviceUevent(event));
    }

    void rejectsIncidentalIdentifierText()
    {
        const QByteArray event = makeEvent({
            QByteArrayLiteral("add@/devices/mentions-0db0-1620"),
            QByteArrayLiteral("ACTION=add"),
            QByteArrayLiteral("SUBSYSTEM=hidraw"),
            QByteArrayLiteral("HID_ID=0003:00001234:00005678"),
        });

        QVERIFY(!strikepro::isSupportedDeviceUevent(event));
    }

    void rejectsUnsupportedActionAndSubsystem()
    {
        const QByteArray wrongAction = makeEvent({
            QByteArrayLiteral("move@/devices/test"),
            QByteArrayLiteral("ACTION=move"),
            QByteArrayLiteral("SUBSYSTEM=hidraw"),
            QByteArrayLiteral("HID_ID=0003:00000DB0:00001620"),
        });
        const QByteArray wrongSubsystem = makeEvent({
            QByteArrayLiteral("add@/devices/test"),
            QByteArrayLiteral("ACTION=add"),
            QByteArrayLiteral("SUBSYSTEM=input"),
            QByteArrayLiteral("HID_ID=0003:00000DB0:00001620"),
        });

        QVERIFY(!strikepro::isSupportedDeviceUevent(wrongAction));
        QVERIFY(!strikepro::isSupportedDeviceUevent(wrongSubsystem));
    }
};

QTEST_GUILESS_MAIN(HidUeventTest)

#include "HidUeventTest.moc"
