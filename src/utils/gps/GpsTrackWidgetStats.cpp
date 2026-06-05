/**
 * @file GpsTrackWidgetStats.cpp
 * @brief GPS 轨迹控件统计方法实现
 *
 * 从 GpsTrackWidget.cpp 拆分而来，包含 resetTrackStats 方法。
 */

#include "utils/gps/GpsTrackWidget.h"

/** @brief 重置轨迹控件统计计数器 */
void GpsTrackWidget::resetTrackStats()
{
    m_stats = GpsTrackWidgetStats();
}
