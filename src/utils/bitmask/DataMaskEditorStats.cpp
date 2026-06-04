/**
 * @file DataMaskEditorStats.cpp
 * @brief 数据掩码编辑器统计查询与重置方法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从 DataMaskEditor.cpp 拆分而来，包含 stats() 和 resetStatistics()。
 */

#include "utils/bitmask/DataMaskEditor.h"

/** @brief 获取统计信息快照 @return 当前统计数据的只读引用 */
const DataMaskEditor::Stats &DataMaskEditor::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器(掩码变更/字段编辑/添加/删除/导出/峰值/活跃归零) */
void DataMaskEditor::resetStatistics()
{
    m_stats.totalMaskChanges = 0;
    m_stats.totalFieldEdits = 0;
    m_stats.totalFieldAdds = 0;
    m_stats.totalFieldRemoves = 0;
    m_stats.totalExports = 0;
    m_stats.peakFields = m_fields.size();
    m_stats.activeFields = m_fields.size();
}
