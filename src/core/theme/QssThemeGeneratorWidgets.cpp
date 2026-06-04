/**
 * @file QssThemeGeneratorWidgets.cpp
 * @brief QSS主题生成器 - 各控件样式生成方法
 *
 * 从 QssThemeGenerator.cpp 拆分而来，包含基础控件/面板/导航栏/
 * 终端/图表/对话框/工具栏的QSS样式生成方法。
 */

#include "core/theme/QssThemeGenerator.h"

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

// ---- 图表/对话框/工具栏样式已拆分至 QssThemeGeneratorPanels.cpp ----
// generateCharts() / generateDialogs() / generateToolbar()
