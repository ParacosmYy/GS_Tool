/**
 * @file ScriptEditorWidgetStats.cpp
 * @brief Script Editor Widget 统计查询与重置
 *
 * 从 ScriptEditorWidget.cpp 拆分而来，包含 stats getter 和 resetPanelStatistics 方法。
 */

#include "utils/scripting/ScriptEditorWidget.h"

// ============================================================
// 面板统计查询与重置
// ============================================================

/** @brief 获取面板统计信息(QVariantMap)，包含运行次数/成功/失败/添加/删除 */
QVariantMap ScriptEditorWidget::stats() const
{
    QVariantMap s;
    s["totalRuns"] = m_totalRuns;
    s["totalSuccessRuns"] = m_totalSuccessRuns;
    s["totalFailedRuns"] = m_totalFailedRuns;
    s["totalScriptsAdded"] = m_totalScriptsAdded;
    s["totalScriptsRemoved"] = m_totalScriptsRemoved;
    s["currentScriptIndex"] = m_currentIndex;
    return s;
}

/** @brief 重置所有面板统计计数器(不影响编辑器内容) */
void ScriptEditorWidget::resetPanelStatistics()
{
    m_totalRuns = 0;
    m_totalSuccessRuns = 0;
    m_totalFailedRuns = 0;
    m_totalScriptsAdded = 0;
    m_totalScriptsRemoved = 0;
}
