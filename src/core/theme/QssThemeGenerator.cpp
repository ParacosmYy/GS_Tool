/**
 * @file QssThemeGenerator.cpp
 * @brief QSS主题生成器实现 - 从语义色自动生成完整QSS
 *
 * 三套主题（暗色/亮色/高对比度）的语义色定义见 QssThemeGeneratorColors.cpp。
 */

#include "core/theme/QssThemeGenerator.h"

#include <QColor>

// ============================================================
// 生成方法
// ============================================================

/** @brief 根据主题类型生成完整QSS样式表 @param type 主题类型枚举 @return 完整QSS样式字符串 */
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

/** @brief 生成基础控件QSS(按钮/输入框/复选框/滑块/进度条/滚动条/标签/提示) @param c 语义色映射表 @return 基础控件QSS字符串 */
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

/** @brief 生成面板和容器QSS(分组框/选项卡/分割器/BasePanel) @param c 语义色映射表 @return 面板容器QSS字符串 */
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

/** @brief 生成侧边导航栏QSS(导航按钮/选中状态/指示器) @param c 语义色映射表 @return 导航栏QSS字符串 */
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

/** @brief 生成终端控件QSS(终端背景/搜索栏/过滤栏) @param c 语义色映射表 @return 终端QSS字符串 */
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

/** @brief 生成图表控件QSS(图表背景/网格/轴线) @param c 语义色映射表 @return 图表QSS字符串 */
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

/** @brief 生成对话框QSS(对话框背景/标题/消息/确认按钮/取消按钮) @param c 语义色映射表 @return 对话框QSS字符串 */
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

/** @brief 生成工具栏QSS(主工具栏/工具按钮/快捷命令栏) @param c 语义色映射表 @return 工具栏QSS字符串 */
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

/** @brief 将QColor转换为十六进制字符串(如 #ff0000) @param color Qt颜色对象 @return 十六进制颜色字符串 */
QString QssThemeGenerator::toHex(const QColor& color)
{
    return color.name(QColor::HexRgb);
}
