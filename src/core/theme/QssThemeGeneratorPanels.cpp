/**
 * @file QssThemeGeneratorPanels.cpp
 * @brief QSS主题生成器 — 面板/对话框/工具栏样式生成方法
 *
 * 从 QssThemeGeneratorWidgets.cpp 拆分而来，包含图表、对话框、
 * 工具栏等辅助面板控件的QSS样式生成方法。
 */

#include "core/theme/QssThemeGenerator.h"

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
