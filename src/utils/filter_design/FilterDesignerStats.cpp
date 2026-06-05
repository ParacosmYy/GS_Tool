/**
 * @file FilterDesignerStats.cpp
 * @brief 滤波器设计器统计接口实现 — 计数器查询与重置
 *
 * 从 FilterDesigner.cpp 拆分，仅包含统计 getter 和 resetStatistics()。
 */

#include "utils/filter_design/FilterDesigner.h"

// ============================================================
// 统计 getter
// ============================================================

/** @brief 获取累计设计IIR滤波器次数 */
quint64 FilterDesigner::totalIIRDesigns() const
{
    return m_totalIIRDesigns;
}

/** @brief 获取累计设计FIR滤波器次数 */
quint64 FilterDesigner::totalFIRDesigns() const
{
    return m_totalFIRDesigns;
}

/** @brief 获取累计计算频率响应次数 */
quint64 FilterDesigner::totalResponsesComputed() const
{
    return m_totalResponsesComputed;
}

// ============================================================
// 重置
// ============================================================

/** @brief 重置所有统计计数器为初始值 */
void FilterDesigner::resetStatistics()
{
    m_totalIIRDesigns       = 0;
    m_totalFIRDesigns       = 0;
    m_totalResponsesComputed = 0;
}
