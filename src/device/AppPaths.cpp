#include "AppPaths.h"

#include <QFileInfo>
#include <QStandardPaths>

namespace strikepro {

QString defaultProtocolProfilePath()
{
    const QString current =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
        + QStringLiteral("/protocol.json");
    if (QFileInfo::exists(current)) {
        return current;
    }

    const QString legacy =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        + QStringLiteral("/kordax/msi-strike-pro/protocol.json");
    return QFileInfo::exists(legacy) ? legacy : current;
}

} // namespace strikepro
