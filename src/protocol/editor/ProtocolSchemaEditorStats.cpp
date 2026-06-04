/**
 * @file ProtocolSchemaEditorStats.cpp
 * @brief 协议帧结构编辑器 — 统计计数器重置实现
 *
 * 从 ProtocolSchemaEditor.cpp 拆分而来，包含协议加载/
 * 保存/验证统计的 resetSchemaEditorStatistics 方法。
 */

#include "protocol/editor/ProtocolSchemaEditor.h"

/** @brief 重置协议编辑器统计计数器(加载/保存/验证/验证失败/编辑/导入/导出) */
void ProtocolSchemaEditor::resetSchemaEditorStatistics()
{
    m_totalSchemasLoaded = 0;
    m_totalSchemasSaved = 0;
    m_totalValidations = 0;
    m_totalValidationFailures = 0;
    m_totalJsonEdits = 0;
    m_totalSchemaImports = 0;
    m_totalSchemaExports = 0;
}
