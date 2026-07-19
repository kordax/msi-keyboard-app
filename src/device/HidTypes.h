#pragma once

#include <QByteArray>
#include <QString>

namespace strikepro {

constexpr quint16 kMsiVendorId = 0x0db0;
constexpr quint16 kStrikeProWirelessProductId = 0x1620;
constexpr quint16 kStrikeProWiredProductId = 0xb231;

[[nodiscard]] constexpr bool isStrikeProProduct(quint16 productId)
{
    return productId == kStrikeProWirelessProductId
           || productId == kStrikeProWiredProductId;
}

struct HidInterface {
    QString devNode;
    QString sysfsPath;
    QString name;
    quint16 vendorId = 0;
    quint16 productId = 0;
    int interfaceNumber = -1;
    bool readable = false;
    bool writable = false;
    QByteArray reportDescriptor;

    [[nodiscard]] bool isTarget() const
    {
        return vendorId == kMsiVendorId && isStrikeProProduct(productId);
    }
};

enum class ReportSource {
    Input,
    Feature,
    Output,
};

struct HidReport {
    QString devNode;
    int interfaceNumber = -1;
    quint16 productId = 0;
    ReportSource source = ReportSource::Input;
    int requestedReportId = -1;
    QByteArray data;
};

} // namespace strikepro
