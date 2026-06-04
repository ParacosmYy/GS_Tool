/**
 * @file QssThemeGeneratorTerminal.cpp
 * @brief QSS主题生成器 - 终端控件样式生成方法
 *
 * 从QssThemeGeneratorWidgets.cpp拆分而来，包含终端相关控件的QSS样式生成:
 *   - generateTerminal(): 终端背景/搜索栏/过滤栏样式
 *
 * 基础控件/面板/导航栏样式保留在QssThemeGeneratorWidgets.cpp中。
 */

#include "core/theme/QssThemeGenerator.h"

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
