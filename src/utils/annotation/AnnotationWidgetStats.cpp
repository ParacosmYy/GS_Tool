/**
 * @file AnnotationWidgetStats.cpp
 * @brief 数据标注工具控件统计方法实现
 */

#include "utils/annotation/AnnotationWidget.h"

/** @brief 重置所有统计计数器 */
void AnnotationWidget::resetStatistics()
{
    m_stats = Stats{};
}
