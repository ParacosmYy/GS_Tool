/**
 * @file TerminalSelectionManagerStats.cpp
 * @brief 终端选区管理器 - 统计查询与重置接口实现
 *
 * 从 TerminalSelectionManager.cpp 拆分而来，包含所有统计getter和resetStats方法。
 */

#include "terminal/selection/TerminalSelectionManager.h"

/** @brief 获取总选择次数 @return 用户完成选区的总次数 */
quint64 TerminalSelectionManager::totalSelections() const
{
    return m_totalSelections;
}

/** @brief 获取总复制次数 @return 用户执行复制操作的总次数 */
quint64 TerminalSelectionManager::totalCopies() const
{
    return m_totalCopies;
}

/** @brief 获取总选择字符数 @return 历史所有选区字符数之和 */
quint64 TerminalSelectionManager::totalSelectionChars() const
{
    return m_totalSelectionChars;
}

/** @brief 获取最大单次选区长度(字符数) @return 历史最大选区的字符数 */
quint64 TerminalSelectionManager::maxSelectionLength() const
{
    return m_maxSelectionLength;
}

/** @brief 获取选区变更总次数 @return 选区范围发生变化的累计次数 */
quint64 TerminalSelectionManager::totalSelectionsChanged() const
{
    return m_totalSelectionsChanged;
}

/** @brief 获取总点击选择次数(单行点击) @return 点击选择计数 */
quint64 TerminalSelectionManager::totalClickSelects() const
{
    return m_totalClickSelects;
}

/** @brief 获取总拖拽选择次数(多行拖拽) @return 拖拽选择计数 */
quint64 TerminalSelectionManager::totalDragSelects() const
{
    return m_totalDragSelects;
}

/** @brief 获取全选操作总次数 @return 全选次数 */
quint64 TerminalSelectionManager::totalSelectAllCalls() const
{
    return m_totalSelectAllCalls;
}

/** @brief 获取选区被取消总次数 @return 取消次数 */
quint64 TerminalSelectionManager::totalSelectionCancels() const
{
    return m_totalSelectionCancels;
}

/** @brief 获取双击选词总次数 @return 双击选词次数 */
quint64 TerminalSelectionManager::totalDoubleClickSelects() const
{
    return m_totalDoubleClickSelects;
}

/** @brief 重置所有统计计数器为零(选区状态不受影响) */
void TerminalSelectionManager::resetStats()
{
    m_totalSelections = 0;
    m_totalCopies = 0;
    m_totalSelectionChars = 0;
    m_maxSelectionLength = 0;
    m_totalSelectionsChanged = 0;
    m_totalClickSelects = 0;
    m_totalDragSelects = 0;
    m_totalSelectAllCalls = 0;
    m_totalSelectionCancels = 0;
    m_totalDoubleClickSelects = 0;
}
