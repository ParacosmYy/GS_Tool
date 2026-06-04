/**
 * @file SignalLineWidgetStats.cpp
 * @brief 信号线状态显示控件 — 统计计数器查询与重置实现
 *
 * 从 SignalLineWidget.cpp 拆分而来，包含信号线更新/
 * DTR/RTS切换次数的统计 getter 和 resetSignalWidgetStatistics 方法。
 */

#include "serial/signals/SignalLineWidget.h"

/** @brief 获取累计信号线状态更新次数 @return 更新总次数 */
quint64 SignalLineWidget::totalSignalUpdates() const
{
    return m_totalSignalUpdates;
}

/** @brief 获取累计DTR切换请求次数 @return DTR切换总次数 */
quint64 SignalLineWidget::totalDtrToggles() const
{
    return m_totalDtrToggles;
}

/** @brief 获取累计RTS切换请求次数 @return RTS切换总次数 */
quint64 SignalLineWidget::totalRtsToggles() const
{
    return m_totalRtsToggles;
}

/** @brief 重置所有统计计数器归零 */
void SignalLineWidget::resetSignalWidgetStatistics()
{
    m_totalSignalUpdates = 0;
    m_totalDtrToggles = 0;
    m_totalRtsToggles = 0;
}
