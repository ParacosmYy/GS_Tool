/**
 * @file DataAnnotationWidgetStats.cpp
 * @brief 数据标注控件统计方法实现
 */

#include "utils/annotation/DataAnnotationWidget.h"

/** @brief 重置所有统计计数器 */
void DataAnnotationWidget::resetStatistics()
{
    m_stats = Stats{};
    m_stats.activeAnnotations = m_annotations.size();
}
