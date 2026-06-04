/**
 * @file CursorOverlayInteraction.cpp
 * @brief 波形游标测量叠加层 -- 游标值追踪、统计计数器接口实现
 *
 * 本文件从 CursorOverlay.cpp 拆分而来，专注于统计与交互查询:
 *   - totalCursorCreations: 游标创建总次数
 *   - totalCursorDeletions: 游标删除总次数
 *   - totalCursorDrags:     游标拖拽总次数
 *   - totalCursorMoves:     游标移动总次数(含放置和拖拽)
 *   - totalDeltaMeasurements: 差值测量总次数
 *   - totalMeasurements:    测量显示总次数(兼容旧接口)
 *   - totalSnapToPeak:      峰值吸附总次数
 *   - averageDeltaX/Y:      历次测量平均值
 *   - resetCursorStatistics: 重置所有计数器
 *
 * 拆分原因:
 *   CursorOverlay.cpp 包含构造、游标管理、事件处理和统计接口，
 *   将统计/查询逻辑独立成文件可降低单文件复杂度，便于维护。
 */

#include "chart/overlay/CursorOverlay.h"

// ============================================================
// 统计计数器接口
// ============================================================

/** @brief 返回游标创建总次数（双击/右键放置游标） @return 创建总次数 */
quint64 CursorOverlay::totalCursorCreations() const
{
    return m_totalCursorCreations;
}

/** @brief 返回游标删除总次数（clearCursors调用次数） @return 删除总次数 */
quint64 CursorOverlay::totalCursorDeletions() const
{
    return m_totalCursorDeletions;
}

/** @brief 返回游标拖拽移动总次数（鼠标拖拽游标） @return 拖拽总次数 */
quint64 CursorOverlay::totalCursorDrags() const
{
    return m_totalCursorDrags;
}

/** @brief 返回游标移动总次数（含放置和拖拽） @return 移动总次数 */
quint64 CursorOverlay::totalCursorMoves() const
{
    return m_totalCursorMoves;
}

/** @brief 返回差值测量总次数（双游标差值面板绘制） @return 差值测量总次数 */
quint64 CursorOverlay::totalDeltaMeasurements() const
{
    return m_totalDeltaMeasurements;
}

/** @brief 返回测量显示总次数（双游标差值面板绘制，兼容旧接口） @return 测量总次数 */
quint64 CursorOverlay::totalMeasurements() const
{
    return m_totalMeasurements;
}

/** @brief 返回峰值吸附总次数（游标自动吸附到最近峰值点） @return 峰值吸附总次数 */
quint64 CursorOverlay::totalSnapToPeak() const
{
    return m_totalSnapToPeak;
}

/** @brief 返回历次测量的平均ΔX值 @return 平均ΔX */
double CursorOverlay::averageDeltaX() const
{
    if (m_totalMeasurements == 0) return 0.0;
    return m_sumDeltaX / static_cast<double>(m_totalMeasurements);
}

/** @brief 返回历次测量的平均ΔY值(首个通道) @return 平均ΔY */
double CursorOverlay::averageDeltaY() const
{
    if (m_totalMeasurements == 0) return 0.0;
    return m_sumDeltaY / static_cast<double>(m_totalMeasurements);
}

/** @brief 重置所有游标统计计数器为初始值 */
void CursorOverlay::resetCursorStatistics()
{
    m_totalCursorCreations = 0;
    m_totalCursorDeletions = 0;
    m_totalCursorDrags = 0;
    m_totalCursorMoves = 0;
    m_totalDeltaMeasurements = 0;
    m_totalMeasurements = 0;
    m_totalSnapToPeak = 0;
    m_sumDeltaX = 0.0;
    m_sumDeltaY = 0.0;
}
