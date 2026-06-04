/**
 * @file QssThemeGenerator.cpp
 * @brief QSS主题生成器实现 - 主入口与工具方法
 *
 * 各控件样式生成方法见 QssThemeGeneratorWidgets.cpp。
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
// 工具方法
// ============================================================

/** @brief 将QColor转换为十六进制字符串(如 #ff0000) @param color Qt颜色对象 @return 十六进制颜色字符串 */
QString QssThemeGenerator::toHex(const QColor& color)
{
    return color.name(QColor::HexRgb);
}
