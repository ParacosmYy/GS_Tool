/**
 * @file SequenceEditorWidgetStats.cpp
 * @brief 协议序列编辑器面板 -- 统计查询与重置
 *
 * 从 SequenceEditorWidget.cpp 中拆分出的统计方法，职责:
 *   1. stats -- 委托给 ProtocolSequencer 的统计查询
 *   2. resetStatistics -- 委托给 ProtocolSequencer 的统计重置
 */

#include "protocol/sequencer/SequenceEditorWidget.h"
#include "protocol/sequencer/ProtocolSequencer.h"

// ============================================================================
// 统计查询
// ============================================================================

/** @brief 获取序列器运行统计 @return 统计结构体常引用 */
const SequencerStats& SequenceEditorWidget::stats() const
{
    return m_sequencer->stats();
}

// ============================================================================
// 统计重置
// ============================================================================

/** @brief 重置序列器累计统计(委托给 ProtocolSequencer) */
void SequenceEditorWidget::resetStatistics()
{
    m_sequencer->resetStatistics();
}
