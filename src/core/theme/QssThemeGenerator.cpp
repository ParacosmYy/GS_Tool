/**
 * @file QssThemeGenerator.cpp
 * @brief QSS主题生成器实现 - 从语义色自动生成完整QSS
 */

#include "core/theme/QssThemeGenerator.h"

#include <QColor>

// ============================================================
// 暗色主题语义色
// ============================================================

QssThemeGenerator::ColorMap QssThemeGenerator::darkColors()
{
    return {
        // 背景色系
        {"bg_primary",      "#1e1f26"},
        {"bg_secondary",    "#282a33"},
        {"bg_tertiary",     "#2a2d35"},
        {"bg_elevated",     "#353840"},
        {"bg_hover",        "#3a3d45"},
        {"bg_pressed",      "#42454d"},
        {"bg_input",        "#22242c"},

        // 文本色系
        {"text_primary",    "#e0e0e0"},
        {"text_secondary",  "#a0a0a0"},
        {"text_disabled",   "#5a5a5a"},
        {"text_accent",     "#4a9eff"},

        // 边框色系
        {"border_default",  "#3a3d45"},
        {"border_focus",    "#4a9eff"},
        {"border_hover",    "#4a4d55"},

        // 强调色
        {"accent_primary",  "#4a9eff"},
        {"accent_success",  "#4ade80"},
        {"accent_warning",  "#f59e0b"},
        {"accent_error",    "#ef4444"},
        {"accent_info",     "#3b82f6"},

        // 滚动条
        {"scrollbar_bg",    "#282a33"},
        {"scrollbar_handle","#4a4d55"},
        {"scrollbar_hover", "#5a5d65"},

        // 选项卡
        {"tab_active",      "#4a9eff"},
        {"tab_inactive",    "transparent"},

        // 终端
        {"terminal_bg",     "#1a1b22"},
        {"terminal_fg",     "#c8c8c8"},
        {"terminal_cursor", "#4a9eff"},
        {"terminal_sel",    "#264f78"},

        // 图表
        {"chart_bg",        "#1e1f26"},
        {"chart_grid",      "#2a2d35"},
        {"chart_axis",      "#5a5a5a"},
    };
}

// ============================================================
// 亮色主题语义色
// ============================================================

QssThemeGenerator::ColorMap QssThemeGenerator::lightColors()
{
    return {
        {"bg_primary",      "#ffffff"},
        {"bg_secondary",    "#f5f5f5"},
        {"bg_tertiary",     "#ebebeb"},
        {"bg_elevated",     "#ffffff"},
        {"bg_hover",        "#e8e8e8"},
        {"bg_pressed",      "#d8d8d8"},
        {"bg_input",        "#ffffff"},

        {"text_primary",    "#1a1a1a"},
        {"text_secondary",  "#666666"},
        {"text_disabled",   "#b0b0b0"},
        {"text_accent",     "#1a73e8"},

        {"border_default",  "#d0d0d0"},
        {"border_focus",    "#1a73e8"},
        {"border_hover",    "#b0b0b0"},

        {"accent_primary",  "#1a73e8"},
        {"accent_success",  "#0d9f4f"},
        {"accent_warning",  "#e09100"},
        {"accent_error",    "#d93025"},
        {"accent_info",     "#1a73e8"},

        {"scrollbar_bg",    "#f0f0f0"},
        {"scrollbar_handle","#c0c0c0"},
        {"scrollbar_hover", "#a0a0a0"},

        {"tab_active",      "#1a73e8"},
        {"tab_inactive",    "transparent"},

        {"terminal_bg",     "#fafafa"},
        {"terminal_fg",     "#333333"},
        {"terminal_cursor", "#1a73e8"},
        {"terminal_sel",    "#cce0f5"},

        {"chart_bg",        "#ffffff"},
        {"chart_grid",      "#f0f0f0"},
        {"chart_axis",      "#999999"},
    };
}

// ============================================================
// 高对比度主题语义色
// ============================================================

QssThemeGenerator::ColorMap QssThemeGenerator::highContrastColors()
{
    return {
        {"bg_primary",      "#000000"},
        {"bg_secondary",    "#1a1a1a"},
        {"bg_tertiary",     "#2a2a2a"},
        {"bg_elevated",     "#333333"},
        {"bg_hover",        "#444444"},
        {"bg_pressed",      "#555555"},
        {"bg_input",        "#111111"},

        {"text_primary",    "#ffffff"},
        {"text_secondary",  "#cccccc"},
        {"text_disabled",   "#777777"},
        {"text_accent",     "#00ccff"},

        {"border_default",  "#666666"},
        {"border_focus",    "#00ccff"},
        {"border_hover",    "#888888"},

        {"accent_primary",  "#00ccff"},
        {"accent_success",  "#00ff88"},
        {"accent_warning",  "#ffcc00"},
        {"accent_error",    "#ff4444"},
        {"accent_info",     "#00ccff"},

        {"scrollbar_bg",    "#1a1a1a"},
        {"scrollbar_handle","#888888"},
        {"scrollbar_hover", "#aaaaaa"},

        {"tab_active",      "#00ccff"},
        {"tab_inactive",    "transparent"},

        {"terminal_bg",     "#000000"},
        {"terminal_fg",     "#ffffff"},
        {"terminal_cursor", "#00ccff"},
        {"terminal_sel",    "#003355"},

        {"chart_bg",        "#000000"},
        {"chart_grid",      "#333333"},
        {"chart_axis",      "#cccccc"},
    };
}

// ============================================================
// 生成方法
// ============================================================

QssThemeGenerator::ColorMap QssThemeGenerator::semanticColors(ThemeType type)
{
    switch (type) {
    case ThemeType::Dark:         return darkColors();
    case ThemeType::Light:        return lightColors();
    case ThemeType::HighContrast: return highContrastColors();
    }
    return darkColors();
}

QString QssThemeGenerator::generate(ThemeType type)
{
    auto colors = semanticColors(type);

    QString qss;
    qss += generateBaseWidgets(colors);
    qss += generatePanels(colors);
    qss += generateNavigation(colors);
    qss += generateTerminal(colors);
    qss += generateCharts(colors);
    qss += generateDialogs(colors);
    qss += generateToolbar(colors);

    return qss;
}

// ============================================================
// 基础控件样式
// ============================================================

QString QssThemeGenerator::generateBaseWidgets(const ColorMap& c)
{
    return QString(
        // QPushButton
        "QPushButton {"
        "  background: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 6px 16px;"
        "  font-size: 13px;"
        "}\n"
        "QPushButton:hover { background: %4; border-color: %5; }\n"
        "QPushButton:pressed { background: %6; }\n"
        "QPushButton:disabled { color: %7; background: %1; }\n\n"

        // QLineEdit
        "QLineEdit {"
        "  background: %8;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  selection-background-color: %9;"
        "}\n"
        "QLineEdit:focus { border-color: %10; }\n"
        "QLineEdit:disabled { color: %7; }\n\n"

        // QTextEdit / QPlainTextEdit
        "QPlainTextEdit, QTextEdit {"
        "  background: %8;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  selection-background-color: %9;"
        "}\n\n"

        // QComboBox
        "QComboBox {"
        "  background: %8;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  padding: 5px 10px;"
        "}\n"
        "QComboBox:focus { border-color: %10; }\n"
        "QComboBox::drop-down { border: none; width: 20px; }\n\n"

        // QSpinBox
        "QSpinBox {"
        "  background: %8;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}\n\n"

        // QCheckBox
        "QCheckBox { color: %2; spacing: 6px; }\n"
        "QCheckBox::indicator { width: 16px; height: 16px; "
        "  border: 2px solid %3; border-radius: 3px; }\n"
        "QCheckBox::indicator:checked { background: %10; "
        "  border-color: %10; }\n\n"

        // QRadioButton
        "QRadioButton { color: %2; spacing: 6px; }\n\n"

        // QSlider
        "QSlider::groove:horizontal { height: 4px; background: %3; "
        "  border-radius: 2px; }\n"
        "QSlider::handle:horizontal { width: 14px; height: 14px; "
        "  background: %10; border-radius: 7px; margin: -5px 0; }\n\n"

        // QProgressBar
        "QProgressBar { background: %3; border-radius: 3px; "
        "  height: 6px; text-align: center; }\n"
        "QProgressBar::chunk { background: %10; border-radius: 3px; }\n\n"

        // QScrollBar
        "QScrollBar:vertical { background: %11; width: 10px; "
        "  border-radius: 5px; }\n"
        "QScrollBar::handle:vertical { background: %12; "
        "  min-height: 30px; border-radius: 5px; }\n"
        "QScrollBar::handle:vertical:hover { background: %13; }\n"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical "
        "  { height: 0; }\n"
        "QScrollBar:horizontal { background: %11; height: 10px; "
        "  border-radius: 5px; }\n"
        "QScrollBar::handle:horizontal { background: %12; "
        "  min-width: 30px; border-radius: 5px; }\n"
        "QScrollBar::handle:horizontal:hover { background: %13; }\n"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal "
        "  { width: 0; }\n\n"

        // QLabel
        "QLabel { color: %2; }\n\n"

        // QToolTip
        "QToolTip { background: %1; color: %2; "
        "  border: 1px solid %3; border-radius: 4px; "
        "  padding: 4px 8px; }\n\n"

    ).arg(
        c["bg_secondary"],    // %1  QPushButton bg
        c["text_primary"],    // %2  text color
        c["border_default"],  // %3  border
        c["bg_hover"],        // %4  btn hover
        c["border_hover"],    // %5  btn hover border
        c["bg_pressed"],      // %6  btn pressed
        c["text_disabled"],   // %7  disabled text
        c["bg_input"],        // %8  input bg
        c["accent_primary"],  // %9  selection
        c["accent_primary"],  // %10 focus/accent
        c["scrollbar_bg"],    // %11 scrollbar bg
        c["scrollbar_handle"],// %12 scrollbar handle
        c["scrollbar_hover"]  // %13 scrollbar hover
    );
}

// ============================================================
// 面板/容器样式
// ============================================================

QString QssThemeGenerator::generatePanels(const ColorMap& c)
{
    return QString(
        // QGroupBox
        "QGroupBox {"
        "  color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "  margin-top: 12px;"
        "  padding: 12px 8px 8px 8px;"
        "  font-weight: bold;"
        "}\n"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  padding: 0 6px;"
        "  color: %1;"
        "}\n\n"

        // QTabWidget / QTabBar
        "QTabWidget::pane { border: 1px solid %2; border-radius: 4px; }\n"
        "QTabBar::tab {"
        "  background: %3;"
        "  color: %4;"
        "  border: 1px solid %2;"
        "  border-bottom: none;"
        "  padding: 6px 16px;"
        "  margin-right: 2px;"
        "}\n"
        "QTabBar::tab:selected {"
        "  background: %5;"
        "  color: %1;"
        "  border-bottom: 2px solid %6;"
        "}\n"
        "QTabBar::tab:hover { background: %7; }\n\n"

        // QSplitter
        "QSplitter::handle { background: %2; }\n"
        "QSplitter::handle:horizontal { width: 2px; }\n"
        "QSplitter::handle:vertical { height: 2px; }\n\n"

        // BasePanel
        "#basePanel {"
        "  background: %5;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}\n"
        "#panelTitle { color: %1; font-weight: bold; font-size: 13px; }\n"
        "#panelContent { background: transparent; }\n\n"

    ).arg(
        c["text_primary"],    // %1
        c["border_default"],  // %2
        c["bg_secondary"],    // %3 tab bg
        c["text_secondary"],  // %4 tab text
        c["bg_primary"],      // %5 panel bg
        c["accent_primary"],  // %6 selected border
        c["bg_hover"]         // %7 tab hover
    );
}

// ============================================================
// 导航栏样式
// ============================================================

QString QssThemeGenerator::generateNavigation(const ColorMap& c)
{
    return QString(
        // 侧边导航
        "#iconNavBar { background: %1; border-right: 1px solid %2; }\n"
        "#navButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px;"
        "  color: %3;"
        "}\n"
        "#navButton:hover { background: %4; color: %5; }\n"
        "#navButton:checked { background: %4; color: %5; }\n\n"

        // 导航指示器
        "#navIndicator { background: %5; border-radius: 2px; }\n\n"

    ).arg(
        c["bg_secondary"],    // %1 nav bg
        c["border_default"],  // %2
        c["text_secondary"],  // %3
        c["bg_hover"],        // %4
        c["accent_primary"]   // %5
    );
}

// ============================================================
// 终端样式
// ============================================================

QString QssThemeGenerator::generateTerminal(const ColorMap& c)
{
    return QString(
        "#terminalWidget { background: %1; color: %2; }\n"
        "#terminalSearchBar {"
        "  background: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}\n"
        "#terminalFilterBar { background: %3; }\n\n"

    ).arg(
        c["terminal_bg"],
        c["terminal_fg"],
        c["bg_elevated"],
        c["border_default"]
    );
}

// ============================================================
// 图表样式
// ============================================================

QString QssThemeGenerator::generateCharts(const ColorMap& c)
{
    return QString(
        "#chartWidget { background: %1; }\n"
        "#chartGrid { color: %2; }\n"
        "#chartAxis { color: %3; }\n\n"

    ).arg(
        c["chart_bg"],
        c["chart_grid"],
        c["chart_axis"]
    );
}

// ============================================================
// 对话框样式
// ============================================================

QString QssThemeGenerator::generateDialogs(const ColorMap& c)
{
    return QString(
        "#appDialog {"
        "  background: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 12px;"
        "}\n"
        "#dialogTitle { color: %3; font-size: 16px; font-weight: bold; }\n"
        "#dialogMessage { color: %4; font-size: 14px; }\n"
        "#dialogConfirmBtn {"
        "  background: %5; color: #fff; border: none;"
        "  border-radius: 6px; padding: 0 16px;"
        "}\n"
        "#dialogConfirmBtn:hover { background: %6; }\n"
        "#dialogCancelBtn {"
        "  background: %7; color: %4;"
        "  border: 1px solid %2;"
        "  border-radius: 6px; padding: 0 16px;"
        "}\n"
        "#dialogCancelBtn:hover { background: %8; color: %3; }\n\n"

    ).arg(
        c["bg_tertiary"],     // %1
        c["border_default"],  // %2
        c["text_primary"],    // %3
        c["text_secondary"],  // %4
        c["accent_primary"],  // %5 confirm
        c["accent_info"],     // %6 confirm hover
        c["bg_elevated"],     // %7 cancel
        c["bg_hover"]         // %8 cancel hover
    );
}

// ============================================================
// 工具栏样式
// ============================================================

QString QssThemeGenerator::generateToolbar(const ColorMap& c)
{
    return QString(
        "#mainToolbar { background: %1; border-bottom: 1px solid %2; }\n"
        "#toolButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  color: %3;"
        "}\n"
        "#toolButton:hover { background: %4; color: %5; }\n"
        "#toolButton:pressed { background: %6; }\n\n"

        "#quickCommandBar {"
        "  background: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 6px;"
        "}\n\n"

    ).arg(
        c["bg_secondary"],
        c["border_default"],
        c["text_secondary"],
        c["bg_hover"],
        c["text_primary"],
        c["bg_pressed"]
    );
}

// ============================================================
// 工具方法
// ============================================================

QString QssThemeGenerator::toHex(const QColor& color)
{
    return color.name(QColor::HexRgb);
}
