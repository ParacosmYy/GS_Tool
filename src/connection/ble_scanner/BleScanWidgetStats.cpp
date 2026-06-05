/**
 * @file BleScanWidgetStats.cpp
 * @brief BLE扫描控件 — UI统计计数器重置接口实现
 *
 * 从 BleScanWidget.cpp 拆分，包含 resetWidgetStatistics()。
 */

#include "connection/ble_scanner/BleScanWidget.h"

/** @brief 重置所有UI统计计数器 */
void BleScanWidget::resetWidgetStatistics()
{
    m_totalUiRefreshes = 0;
    m_totalDeviceSelections = 0;
    m_totalSortChanges = 0;
    m_totalScanTriggers = 0;
}
