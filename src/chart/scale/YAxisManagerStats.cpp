/**
 * @file YAxisManagerStats.cpp
 * @brief Y轴管理器 - 主题应用与统计接口实现
 *
 * 从 YAxisManager.cpp 拆分而来，包含主题颜色应用、
 * 内部辅助方法和统计计数器访问/重置方法。
 */

#include "chart/scale/YAxisManager.h"

#include <QValueAxis>

/** @brief 应用当前主题颜色到所有Y轴，更新轴标签颜色、网格线颜色和轴线条颜色 @param gridColor 网格线颜色 @param labelColor 标签文字颜色 */
void YAxisManager::applyThemeColors(const QColor& gridColor, const QColor& labelColor)
{
    ++m_totalThemeApplied;  ///< 统计: 主题颜色应用次数递增
    for (auto it = m_axes.begin(); it != m_axes.end(); ++it) {
        QValueAxis* axis = it->axis;

        axis->setLabelsBrush(QBrush(it->color));
        axis->setTitleBrush(it->color);

        if (axis->isGridLineVisible()) {
            axis->setGridLineColor(gridColor);
        }

        QColor axisColor = it->color;
        axisColor.setAlpha(180);
        axis->setLinePen(QPen(axisColor, 1));
    }
}

/** @brief 获取指定侧已使用的轴数量 @param side 左/右侧 @return 该侧已有的轴数量 */
int YAxisManager::countAxesOnSide(YAxisSide side) const
{
    int count = 0;
    for (auto it = m_axes.constBegin(); it != m_axes.constEnd(); ++it) {
        if (it->side == side) {
            ++count;
        }
    }
    return count;
}

/** @brief 获取累计缩放重算次数 @return 重算次数 */
quint64 YAxisManager::totalRescales() const
{
    return m_totalRescales;
}

/** @brief 获取累计自动缩放事件次数 @return 自动缩放事件次数 */
quint64 YAxisManager::totalAutoScaleEvents() const
{
    return m_totalAutoScaleEvents;
}

/** @brief 获取累计手动范围设置次数 @return 手动设置次数 */
quint64 YAxisManager::totalManualRangeSets() const
{
    return m_totalManualRangeSets;
}

/** @brief 获取累计主题颜色应用次数 @return 主题应用次数 */
quint64 YAxisManager::totalThemeApplied() const
{
    return m_totalThemeApplied;
}

/** @brief 获取历史峰值轴数量 @return 峰值轴数量 */
quint64 YAxisManager::peakAxisCount() const
{
    return m_peakAxisCount;
}

/** @brief 重置所有Y轴统计计数器 */
void YAxisManager::resetYAxisStatistics()
{
    m_totalRescales = 0;
    m_totalAutoScaleEvents = 0;
    m_totalAxisAdds = 0;
    m_totalAxisRemoves = 0;
    m_totalManualRangeSets = 0;
    m_totalThemeApplied = 0;
    m_peakAxisCount = 0;
}
