/**
 * @file ProtocolFieldEditorStats.cpp
 * @brief 协议字段结构编辑器 — 统计计数器重置
 *
 * 从 ProtocolFieldEditor.cpp 拆分而来，仅包含:
 *   - resetStatistics(): 重置全部运行统计计数器
 */

#include "protocol/field_editor/ProtocolFieldEditor.h"

/** @brief 重置所有统计计数器归零 */
void ProtocolFieldEditor::resetStatistics()
{
    m_stats.totalFieldAdds = 0;
    m_stats.totalFieldRemoves = 0;
    m_stats.totalFieldUpdates = 0;
    m_stats.totalDataUpdates = 0;
    m_stats.totalJsonImports = 0;
    m_stats.totalJsonExports = 0;
    m_stats.totalInterpretations = 0;
    m_stats.interpretationErrors = 0;
}
