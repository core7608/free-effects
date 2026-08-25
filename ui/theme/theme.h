#pragma once

#include <QString>

class QApplication;

namespace FreeEffect {
namespace Theme {

QString getStyleSheet();
void applyTheme(QApplication* app);

} // namespace Theme
} // namespace FreeEffect
