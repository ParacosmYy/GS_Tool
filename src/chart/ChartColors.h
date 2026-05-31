#ifndef CHARTCOLORS_H
#define CHARTCOLORS_H

#include <QColor>
#include <QVector>

/**
 * @file ChartColors.h
 * @brief 波形图通道默认颜色表（全局唯一定义点）
 *
 * ChartWidget 和 ChannelConfigSet 共享此颜色表，
 * 避免在两处重复定义导致颜色演化不一致（DRY 原则）。
 *
 * 颜色来源: Catppuccin Mocha 调色板，确保在暗色主题下有良好辨识度。
 */

namespace ChartColors {

/** @brief 默认通道颜色表，最多支持 10 条波形线 */
inline const QVector<QColor>& defaultColors()
{
    static const QVector<QColor> colors = {
        QColor("#89b4fa"),  // 蓝色 (Blue)
        QColor("#a6e3a1"),  // 绿色 (Green)
        QColor("#f9e2af"),  // 黄色 (Yellow)
        QColor("#f38ba8"),  // 红色 (Red)
        QColor("#94e2d5"),  // 青色 (Teal)
        QColor("#cba6f7"),  // 紫色 (Mauve)
        QColor("#fab387"),  // 橙色 (Peach)
        QColor("#74c7ec"),  // 天蓝 (Sapphire)
        QColor("#f5c2e7"),  // 粉色 (Pink)
        QColor("#b4befe")   // 薰衣草 (Lavender)
    };
    return colors;
}

} // namespace ChartColors

#endif // CHARTCOLORS_H
