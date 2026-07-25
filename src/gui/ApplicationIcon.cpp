#include "ApplicationIcon.h"

#include <QList>
#include <Qt>

namespace strikepro {
namespace {

constexpr auto defaultIcon = ":/assets/icons/io.github.kordax.MsiKeyboard.svg";
constexpr auto cinnamonIcon =
    ":/assets/icons/io.github.kordax.MsiKeyboard-cinnamon.svg";
constexpr auto defaultDesktopFile = "io.github.kordax.MsiKeyboard";
constexpr auto cinnamonDesktopFile = "io.github.kordax.MsiKeyboard.Cinnamon";

bool isCinnamonDesktop(const QStringView desktop)
{
    for (const QStringView component :
         desktop.split(u':', Qt::SkipEmptyParts)) {
        const QStringView name = component.trimmed();
        if (name.compare(QStringView(u"X-Cinnamon"), Qt::CaseInsensitive) == 0
            || name.compare(QStringView(u"Cinnamon"), Qt::CaseInsensitive)
                   == 0) {
            return true;
        }
    }
    return false;
}

} // namespace

QString applicationIconResource(const QStringView currentDesktop)
{
    return QString::fromLatin1(
        isCinnamonDesktop(currentDesktop) ? cinnamonIcon : defaultIcon);
}

QString applicationDesktopFileName(const QStringView currentDesktop)
{
    return QString::fromLatin1(
        isCinnamonDesktop(currentDesktop) ? cinnamonDesktopFile
                                          : defaultDesktopFile);
}

} // namespace strikepro
