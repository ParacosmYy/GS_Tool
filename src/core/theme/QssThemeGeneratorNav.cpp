/**
 * @file QssThemeGeneratorNav.cpp
 * @brief QSS主题生成器 - 侧边导航栏样式生成方法
 *
 * 从 QssThemeGeneratorWidgets.cpp 拆分而来，包含:
 *   - generateNavigation(): 侧边导航栏QSS(导航按钮/选中状态/指示器)
 *
 * 基础控件/面板样式见 QssThemeGeneratorWidgets.cpp。
 * 终端样式见 QssThemeGeneratorTerminal.cpp。
 * 图表/对话框/工具栏样式见 QssThemeGeneratorPanels.cpp。
 */

#include "core/theme/QssThemeGenerator.h"

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
