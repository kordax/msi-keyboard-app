#include "HidUevent.h"

#include <QHash>
#include <QRegularExpression>

#include <limits>
#include <optional>

namespace strikepro {
namespace {

struct UeventDeviceId {
    quint16 vendorId = 0;
    quint16 productId = 0;
};

bool parseHexId(const QByteArray &text, quint16 *output)
{
    bool ok = false;
    const qulonglong value = text.toULongLong(&ok, 16);
    if (!ok || value > std::numeric_limits<quint16>::max()) {
        return false;
    }
    *output = static_cast<quint16>(value);
    return true;
}

std::optional<UeventDeviceId>
parseDeviceId(const QHash<QByteArray, QByteArray> &properties)
{
    const QList<QByteArray> hidId = properties.value("HID_ID").split(':');
    if (hidId.size() == 3) {
        UeventDeviceId result;
        if (parseHexId(hidId.at(1), &result.vendorId)
            && parseHexId(hidId.at(2), &result.productId)) {
            return result;
        }
    }

    const QList<QByteArray> product = properties.value("PRODUCT").split('/');
    if (product.size() >= 2) {
        UeventDeviceId result;
        if (parseHexId(product.at(0), &result.vendorId)
            && parseHexId(product.at(1), &result.productId)) {
            return result;
        }
    }

    static const QRegularExpression hidModalias(
        QStringLiteral(R"(v([0-9A-Fa-f]{8})p([0-9A-Fa-f]{8}))"));
    const QRegularExpressionMatch match =
        hidModalias.match(QString::fromLatin1(properties.value("MODALIAS")));
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    UeventDeviceId result;
    if (!parseHexId(match.captured(1).toLatin1(), &result.vendorId)
        || !parseHexId(match.captured(2).toLatin1(), &result.productId)) {
        return std::nullopt;
    }
    return result;
}

} // namespace

bool isSupportedDeviceUevent(const QByteArray &event)
{
    return isSupportedDeviceUevent(event, supportedDeviceDefinitions());
}

bool isSupportedDeviceUevent(
    const QByteArray &event,
    const std::span<const DeviceDefinition> definitions)
{
    QByteArray normalized = event;
    normalized.replace('\0', '\n');

    QHash<QByteArray, QByteArray> properties;
    QByteArray action;
    for (const QByteArray &rawLine : normalized.split('\n')) {
        const QByteArray line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        const qsizetype separator = line.indexOf('=');
        if (separator > 0) {
            properties.insert(
                line.first(separator).toUpper(),
                line.sliced(separator + 1));
            continue;
        }
        const qsizetype pathSeparator = line.indexOf('@');
        if (pathSeparator > 0 && action.isEmpty()) {
            action = line.first(pathSeparator).toLower();
        }
    }

    if (action.isEmpty()) {
        action = properties.value("ACTION").toLower();
    }
    const bool relevantAction = action == "add" || action == "remove"
                                || action == "change" || action == "bind"
                                || action == "unbind";
    if (!relevantAction) {
        return false;
    }

    const QByteArray subsystem = properties.value("SUBSYSTEM").toLower();
    if (subsystem != "hidraw" && subsystem != "hid" && subsystem != "usb") {
        return false;
    }

    const std::optional<UeventDeviceId> deviceId = parseDeviceId(properties);
    if (!deviceId.has_value()) {
        return false;
    }

    for (const DeviceDefinition &definition : definitions) {
        if (definition.matches(deviceId->vendorId, deviceId->productId)) {
            return true;
        }
    }
    return false;
}

} // namespace strikepro
