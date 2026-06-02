/**
 * @file ChartColors.h
 * @brief 波形图通道颜色表 -- 支持暗色/亮色主题双调色板
 *
 * 设计要点:
 *   1. 为暗色主题提供 Catppuccin Mocha 调色板（高饱和、亮色线条）
 *   2. 为亮色主题提供 Catppuccin Latte 调色板（高饱和、深色线条）
 *   3. 通过 isDark 参数由 ChartWidget 在主题切换时选择对应调色板
 *   4. ChartWidget 和 ChannelConfigSet 共享此颜色表（DRY 原则）
 *
 * 协作关系:
 *   - ChartWidget: 调用 colorsForTheme() 获取当前主题对应调色板
 *   - ChannelConfigSet: 调用 defaultColors() 获取暗色默认调色板
 *   - ThemeManager: 主题切换时通知 ChartWidget 重绘，间接驱动调色板切换
 */

#ifndef CHARTCOLORS_H
#define CHARTCOLORS_H

#include <QColor>
#include <QVector>

namespace ChartColors {

/**
 * @brief 暗色主题默认通道颜色表（Catppuccin Mocha 调色板）
 *
 * 最多支持 10 条波形线。颜色选取原则:
 *   - 高饱和度，在深色背景上辨识度高
 *   - 相邻颜色色相差距大，避免混淆
 *
 * @return 暗色调色板的常引用
 */
inline const QVector<QColor>& darkColors()
{
    static const QVector<QColor> colors = {
        QColor("#89b4fa"),  ///< 蓝色 (Blue)
        QColor("#a6e3a1"),  ///< 绿色 (Green)
        QColor("#f9e2af"),  ///< 黄色 (Yellow)
        QColor("#f38ba8"),  ///< 红色 (Red)
        QColor("#94e2d5"),  ///< 青色 (Teal)
        QColor("#cba6f7"),  ///< 紫色 (Mauve)
        QColor("#fab387"),  ///< 橙色 (Peach)
        QColor("#74c7ec"),  ///< 天蓝 (Sapphire)
        QColor("#f5c2e7"),  ///< 粉色 (Pink)
        QColor("#b4befe")   ///< 薰衣草 (Lavender)
    };
    return colors;
}

/**
 * @brief 亮色主题通道颜色表（Catppuccin Latte 调色板）
 *
 * 最多支持 10 条波形线。颜色选取原则:
 *   - 高饱和度但明度较低，在浅色背景上辨识度高
 *   - 与暗色调色板保持色相对应，方便用户跨主题识别同一通道
 *
 * @return 亮色调色板的常引用
 */
inline const QVector<QColor>& lightColors()
{
    static const QVector<QColor> colors = {
        QColor("#1e66f5"),  ///< 蓝色 (Blue) -- 深蓝，在白底上清晰
        QColor("#40a02b"),  ///< 绿色 (Green) -- 深绿
        QColor("#df8e1d"),  ///< 黄色 (Yellow) -- 深橙黄，纯黄在白底上不可读
        QColor("#d20f39"),  ///< 红色 (Red) -- 深红
        QColor("#179299"),  ///< 青色 (Teal) -- 深青
        QColor("#8839ef"),  ///< 紫色 (Mauve) -- 深紫
        QColor("#fe640b"),  ///< 橙色 (Peach) -- 深橙
        QColor("#04a5e5"),  ///< 天蓝 (Sapphire) -- 深天蓝
        QColor("#ea76cb"),  ///< 粉色 (Pink) -- 深粉
        QColor("#7287fd")   ///< 薰衣草 (Lavender) -- 深薰衣草
    };
    return colors;
}

/**
 * @brief 默认通道颜色表（向后兼容接口，返回暗色调色板）
 *
 * 保留此接口以兼容 ChannelConfigSet::generateDefaults() 等现有调用点。
 * 新代码应优先使用 colorsForTheme() 以支持主题切换。
 *
 * @return 暗色调色板的常引用
 */
inline const QVector<QColor>& defaultColors()
{
    return darkColors();
}

/**
 * @brief 根据主题模式返回对应的通道颜色表
 *
 * 由 ChartWidget::applyThemeColors() 在初始化和主题切换时调用，
 * 确保数据线颜色与当前主题背景有足够对比度。
 *
 * @param isDark true 为暗色主题（使用 Mocha 调色板），false 为亮色主题（使用 Latte 调色板）
 * @return 对应调色板的常引用
 */
inline const QVector<QColor>& colorsForTheme(bool isDark)
{
    return isDark ? darkColors() : lightColors();
}

} // namespace ChartColors

#endif // CHARTCOLORS_H
