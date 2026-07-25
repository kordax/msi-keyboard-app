#pragma once

#include <QString>
#include <QStringView>

namespace strikepro {

[[nodiscard]] QString applicationIconResource(QStringView currentDesktop);
[[nodiscard]] QString applicationDesktopFileName(QStringView currentDesktop);

} // namespace strikepro
