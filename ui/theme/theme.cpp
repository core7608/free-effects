#include "theme.h"
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QFont>

namespace FreeEffect {
namespace Theme {

static const QString STYLE_SHEET = R"(
* {
    font-family: "SF Pro Display", "Segoe UI", "Helvetica Neue", Arial, sans-serif;
    font-size: 12px;
}

QMainWindow, QDialog {
    background-color: #0e0e0e;
    color: #e0e0e0;
}

QMenuBar {
    background-color: #0e0e0e;
    color: #cccccc;
    border-bottom: 1px solid #1e1e1e;
    padding: 1px 0;
    font-size: 12px;
}

QMenuBar::item {
    padding: 4px 8px;
    background: transparent;
}

QMenuBar::item:selected {
    background-color: #ffffff;
    color: #000000;
    border-radius: 2px;
}

QMenu {
    background-color: #141414;
    color: #cccccc;
    border: 1px solid #2a2a2a;
    padding: 4px 0;
}

QMenu::item {
    padding: 5px 30px 5px 25px;
    background: transparent;
}

QMenu::item:selected {
    background-color: #ffffff;
    color: #000000;
}

QMenu::item:disabled {
    color: #444444;
}

QMenu::separator {
    height: 1px;
    background-color: #222222;
    margin: 4px 10px;
}

QToolBar {
    background-color: #0e0e0e;
    border: none;
    spacing: 2px;
    padding: 2px;
}

QToolBar::separator {
    width: 1px;
    background-color: #222222;
    margin: 4px 4px;
}

QToolButton {
    background-color: transparent;
    color: #cccccc;
    border: 1px solid transparent;
    border-radius: 2px;
    padding: 4px;
    min-width: 28px;
    min-height: 28px;
}

QToolButton:hover {
    background-color: #222222;
    border: 1px solid #333333;
}

QToolButton:pressed {
    background-color: #ffffff;
    border: 1px solid #ffffff;
    color: #000000;
}

QToolButton:checked {
    background-color: #ffffff;
    border: 1px solid #ffffff;
    color: #000000;
}

QDockWidget {
    titlebar-close-icon: url(:/icons/ui/close.svg);
    titlebar-normal-icon: url(:/icons/ui/expand.svg);
    font-size: 11px;
    font-weight: 600;
    color: #888888;
}

QDockWidget::title {
    text-align: left;
    background-color: #111111;
    padding: 6px 8px;
    border-bottom: 1px solid #1e1e1e;
    border-top: 1px solid #1e1e1e;
}

QDockWidget::title:hover {
    background-color: #181818;
}

QDockWidget::close-button, QDockWidget::float-button {
    icon-size: 14px;
    subcontrol-position: top right;
    subcontrol-origin: padding;
    position: absolute;
    top: 4px;
    right: 4px;
    width: 18px;
    height: 18px;
}

QDockWidget::close-button { right: 24px; }

QDockWidget::close-button:hover, QDockWidget::float-button:hover {
    background-color: #333333;
}

QTabWidget::pane {
    border: 1px solid #1e1e1e;
    background-color: #0e0e0e;
}

QTabBar::tab {
    background-color: #111111;
    color: #666666;
    border: 1px solid #1e1e1e;
    padding: 6px 14px;
    margin-right: -1px;
    font-size: 11px;
}

QTabBar::tab:selected {
    background-color: #0e0e0e;
    color: #ffffff;
    border-bottom-color: #0e0e0e;
}

QTabBar::tab:hover:!selected {
    background-color: #1a1a1a;
    color: #aaaaaa;
}

QTreeWidget, QTreeView {
    background-color: #0e0e0e;
    color: #cccccc;
    border: none;
    outline: none;
    font-size: 11px;
}

QTreeWidget::item, QTreeView::item {
    padding: 2px 4px;
    border: none;
}

QTreeWidget::item:selected, QTreeView::item:selected {
    background-color: #ffffff;
    color: #000000;
}

QTreeWidget::item:hover:!selected, QTreeView::item:hover:!selected {
    background-color: #1a1a1a;
}

QHeaderView::section {
    background-color: #111111;
    color: #666666;
    border: none;
    border-right: 1px solid #1a1a1a;
    border-bottom: 1px solid #1a1a1a;
    padding: 4px 8px;
    font-size: 10px;
    font-weight: normal;
}

QHeaderView::section:hover {
    background-color: #1a1a1a;
}

QLabel {
    color: #cccccc;
    background: transparent;
}

QLineEdit {
    background-color: #1a1a1a;
    color: #e0e0e0;
    border: 1px solid #2a2a2a;
    border-radius: 2px;
    padding: 4px 6px;
    selection-background-color: #ffffff;
    selection-color: #000000;
}

QLineEdit:focus {
    border: 1px solid #ffffff;
}

QLineEdit:disabled {
    background-color: #151515;
    color: #444444;
}

QSpinBox, QDoubleSpinBox {
    background-color: #1a1a1a;
    color: #e0e0e0;
    border: 1px solid #2a2a2a;
    border-radius: 2px;
    padding: 2px 4px;
    selection-background-color: #ffffff;
}

QSpinBox:focus, QDoubleSpinBox:focus {
    border: 1px solid #ffffff;
}

QComboBox {
    background-color: #1a1a1a;
    color: #e0e0e0;
    border: 1px solid #2a2a2a;
    border-radius: 2px;
    padding: 4px 8px;
    min-width: 60px;
}

QComboBox:hover {
    border: 1px solid #444444;
}

QComboBox:focus {
    border: 1px solid #ffffff;
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;
    width: 20px;
    border-left: 1px solid #2a2a2a;
    border-top-right-radius: 2px;
    border-bottom-right-radius: 2px;
    background-color: #222222;
}

QComboBox QAbstractItemView {
    background-color: #141414;
    color: #cccccc;
    selection-background-color: #ffffff;
    selection-color: #000000;
    border: 1px solid #2a2a2a;
    outline: none;
}

QPushButton {
    background-color: #1a1a1a;
    color: #cccccc;
    border: 1px solid #2a2a2a;
    border-radius: 2px;
    padding: 5px 14px;
    min-height: 20px;
    font-size: 11px;
}

QPushButton:hover {
    background-color: #252525;
    border: 1px solid #444444;
}

QPushButton:pressed {
    background-color: #ffffff;
    color: #000000;
    border: 1px solid #ffffff;
}

QPushButton:disabled {
    background-color: #141414;
    color: #333333;
    border: 1px solid #1e1e1e;
}

QPushButton:default {
    background-color: #ffffff;
    color: #000000;
    border: 1px solid #ffffff;
    font-weight: bold;
}

QPushButton:default:hover {
    background-color: #e0e0e0;
}

QCheckBox {
    color: #cccccc;
    spacing: 6px;
}

QCheckBox::indicator {
    width: 14px;
    height: 14px;
    border: 1px solid #444444;
    border-radius: 2px;
    background-color: #1a1a1a;
}

QCheckBox::indicator:checked {
    background-color: #ffffff;
    border-color: #ffffff;
}

QCheckBox::indicator:hover {
    border: 1px solid #888888;
}

QSlider::groove:horizontal {
    height: 2px;
    background-color: #2a2a2a;
    border-radius: 1px;
}

QSlider::handle:horizontal {
    width: 12px;
    height: 12px;
    margin: -5px 0;
    background-color: #ffffff;
    border: none;
    border-radius: 6px;
}

QSlider::handle:horizontal:hover {
    background-color: #ffffff;
}

QSlider::sub-page:horizontal {
    background-color: #ffffff;
    border-radius: 1px;
}

QProgressBar {
    background-color: #1a1a1a;
    border: 1px solid #2a2a2a;
    border-radius: 2px;
    text-align: center;
    color: #cccccc;
    height: 16px;
}

QProgressBar::chunk {
    background-color: #ffffff;
    border-radius: 2px;
}

QScrollBar:vertical {
    background-color: #0e0e0e;
    width: 8px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background-color: #333333;
    border-radius: 4px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background-color: #555555;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background-color: #0e0e0e;
    height: 8px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background-color: #333333;
    border-radius: 4px;
    min-width: 30px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #555555;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

QStatusBar {
    background-color: #0e0e0e;
    color: #555555;
    border-top: 1px solid #1a1a1a;
    font-size: 11px;
}

QStatusBar::item {
    border: none;
}

QGroupBox {
    color: #cccccc;
    border: 1px solid #222222;
    border-radius: 4px;
    margin-top: 8px;
    padding-top: 12px;
    font-weight: bold;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 4px;
}

QSplitter::handle {
    background-color: #1a1a1a;
}

QSplitter::handle:horizontal {
    width: 2px;
}

QSplitter::handle:vertical {
    height: 2px;
}

QSplitter::handle:hover {
    background-color: #ffffff;
}

QListWidget {
    background-color: #0e0e0e;
    color: #cccccc;
    border: none;
    outline: none;
}

QListWidget::item {
    padding: 4px 8px;
    border: none;
}

QListWidget::item:selected {
    background-color: #ffffff;
    color: #000000;
}

QListWidget::item:hover:!selected {
    background-color: #1a1a1a;
}

QToolTip {
    background-color: #1e1e1e;
    color: #cccccc;
    border: 1px solid #333333;
    padding: 4px;
    font-size: 11px;
}

QTextEdit, QPlainTextEdit {
    background-color: #0e0e0e;
    color: #cccccc;
    border: 1px solid #2a2a2a;
    selection-background-color: #ffffff;
    selection-color: #000000;
}
)";

QString getStyleSheet() {
    return STYLE_SHEET;
}

void applyTheme(QApplication* app) {
    app->setStyle(QStyleFactory::create("Fusion"));
    
    // Fusion dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(14, 14, 14));
    darkPalette.setColor(QPalette::WindowText, QColor(224, 224, 224));
    darkPalette.setColor(QPalette::Base, QColor(14, 14, 14));
    darkPalette.setColor(QPalette::AlternateBase, QColor(17, 17, 17));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::ToolTipText, QColor(204, 204, 204));
    darkPalette.setColor(QPalette::Text, QColor(224, 224, 224));
    darkPalette.setColor(QPalette::Button, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::ButtonText, QColor(204, 204, 204));
    darkPalette.setColor(QPalette::BrightText, Qt::white);
    darkPalette.setColor(QPalette::Link, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(68, 68, 68));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(68, 68, 68));
    
    app->setPalette(darkPalette);
    app->setStyleSheet(STYLE_SHEET);
    
    QFont defaultFont("SF Pro Display", 12);
    defaultFont.setStyleStrategy(QFont::PreferAntialias);
    app->setFont(defaultFont);
}

} // namespace Theme
} // namespace FreeEffect
