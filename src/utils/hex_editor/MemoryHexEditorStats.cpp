/**
 * @file MemoryHexEditorStats.cpp
 * @brief 十六进制内存编辑器统计重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 独立编译单元，避免主实现文件超过500行限制。
 */

#include "utils/hex_editor/MemoryHexEditor.h"

/**
 * @brief 重置所有运行时统计计数器为零
 *
 * 调用后 stats() 返回的各字段全部归零，不影响编辑器当前数据/光标/选区状态。
 */
void MemoryHexEditor::resetStatistics()
{
    m_stats = Stats{};
}
