/**
 * @file EyeDiagramEngineStats.cpp
 * @brief 眼图引擎统计接口实现 — 计数器查询与重置
 *
 * 从 EyeDiagramEngine.cpp 拆分，仅包含统计 getter 和 resetStatistics()。
 */

#include "chart/eye/EyeDiagramEngine.h"

// ============================================================
// 统计 getter
// ============================================================

/** @brief 获取累计处理的采样点数 */
quint64 EyeDiagramEngine::totalSamplesProcessed() const
{
    return m_totalSamplesProcessed;
}

/** @brief 获取累计处理的比特数 */
quint64 EyeDiagramEngine::totalBitsProcessed() const
{
    return m_totalBitsProcessed;
}

/** @brief 获取累计叠加的眼图层数 */
quint64 EyeDiagramEngine::totalOverlays() const
{
    return m_totalOverlays;
}

/** @brief 获取累计测量次数 */
quint64 EyeDiagramEngine::totalMeasurements() const
{
    return m_totalMeasurements;
}

/** @brief 获取累计掩模违规次数 */
quint64 EyeDiagramEngine::totalMaskHits() const
{
    return m_totalMaskHits;
}

/** @brief 获取累计掩模测试次数 */
quint64 EyeDiagramEngine::totalMaskTests() const
{
    return m_totalMaskTests;
}

// ============================================================
// 重置
// ============================================================

/** @brief 重置所有统计计数器为初始值 */
void EyeDiagramEngine::resetStatistics()
{
    m_totalSamplesProcessed = 0;
    m_totalBitsProcessed   = 0;
    m_totalOverlays        = 0;
    m_totalMeasurements    = 0;
    m_totalMaskHits        = 0;
    m_totalMaskTests       = 0;
}
