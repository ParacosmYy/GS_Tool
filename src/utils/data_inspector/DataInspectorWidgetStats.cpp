/**
 * @file DataInspectorWidgetStats.cpp
 * @brief 字节级数据检查器 — 统计接口实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 仅包含 stats() 常量引用与 resetStatistics() 重置方法。
 */

#include "utils/data_inspector/DataInspectorWidget.h"

/**
 * @brief 获取统计数据只读引用
 * @return Stats 常量引用，外部可读取但不修改
 */
const DataInspectorWidget::Stats &DataInspectorWidget::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器（检查次数/字节数/复制次数/峰值/格式切换）
 */
void DataInspectorWidget::resetStatistics()
{
    m_stats.totalInspections    = 0;
    m_stats.totalBytesInspected = 0;
    m_stats.totalCopies         = 0;
    m_stats.peakBytesPerInspect = 0;
    m_stats.formatChanges       = 0;
}
