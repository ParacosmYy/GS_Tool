/**
 * @file TerminalLayoutManagerStats.cpp
 * @brief 终端布局管理器统计方法实现
 *
 * 从 TerminalLayoutManager.cpp 拆分而来，包含所有统计getter、
 * resetStats和统计通知方法。
 */

#include "terminal/layout/TerminalLayoutManager.h"

/** @brief 获取布局切换总次数 @return 累计切换总次数 */
quint64 TerminalLayoutManager::totalSwitches() const
{
    return m_totalSwitches;
}

/** @brief 获取清除行数累计 @return 清除行总数 */
quint64 TerminalLayoutManager::totalLinesCleared() const
{
    return m_totalLinesCleared;
}

/** @brief 获取历史最大可见行数 @return 最大可见行数峰值 */
quint64 TerminalLayoutManager::maxVisibleLines() const
{
    return m_maxVisibleLines;
}

/** @brief 重置所有统计计数器为零 */
void TerminalLayoutManager::resetStats()
{
    m_totalSwitches = 0;
    m_totalLinesCleared = 0;
    m_maxVisibleLines = 0;
    m_totalSplits = 0;
    m_totalTabSwitches = 0;
    m_totalLayoutChanges = 0;
    m_totalTabAdds = 0;
    m_totalTabRemoves = 0;
}

/** @brief 通知行清除事件，累加清除行数到统计计数器 @param lines 本次清除的行数 */
void TerminalLayoutManager::notifyLinesCleared(quint64 lines)
{
    m_totalLinesCleared += lines;
}

/** @brief 更新最大可见行数记录，仅当当前值超过历史峰值时更新 @param currentVisible 当前可见行数 */
void TerminalLayoutManager::updateMaxVisibleLines(quint64 currentVisible)
{
    if (currentVisible > m_maxVisibleLines) {
        m_maxVisibleLines = currentVisible;
    }
}

/** @brief 获取分栏创建总次数 @return 累计分栏次数 */
quint64 TerminalLayoutManager::totalSplits() const
{
    return m_totalSplits;
}

/** @brief 获取Tab切换总次数 @return 累计Tab切换次数 */
quint64 TerminalLayoutManager::totalTabSwitches() const
{
    return m_totalTabSwitches;
}

/** @brief 获取布局变更总次数 @return 累计布局变更次数 */
quint64 TerminalLayoutManager::totalLayoutChanges() const
{
    return m_totalLayoutChanges;
}

/** @brief 获取Tab添加总次数 @return 累计添加次数 */
quint64 TerminalLayoutManager::totalTabAdds() const
{
    return m_totalTabAdds;
}

/** @brief 获取Tab移除总次数 @return 累计移除次数 */
quint64 TerminalLayoutManager::totalTabRemoves() const
{
    return m_totalTabRemoves;
}
