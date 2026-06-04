/**
 * @file FrameVisualEditorPreview.cpp
 * @brief 帧格式可视化编辑器 - 二进制预览生成与统计重置
 *
 * 从 FrameVisualEditor.cpp 拆分而来，包含:
 *   - updateBinaryPreview(): 根据当前字段配置生成二进制布局预览
 *   - onPreviewTimerTick():  预览定时器回调
 *   - resetEditorStatistics(): 统计计数器重置
 *
 * 核心编辑逻辑(数据读写/配置回调/定义重建)见 FrameVisualEditor.cpp。
 * UI构建方法见 FrameVisualEditorUI.cpp。
 * 字段操作方法见 FrameVisualEditorFields.cpp。
 */

#include "protocol/editor/FrameVisualEditor.h"

// ---- 二进制预览生成 ----

/**
 * @brief 更新二进制布局预览
 *
 * 格式示例: [帧头:0-1] [温度(u16LE):2-3] [电压(u32LE):4-7] [CRC16:8-9]
 * 自动根据当前帧头/长度/字段/校验/帧尾配置拼接完整的字节偏移布局图。
 * 对于未覆盖的偏移区域插入[预留:x-y]占位。
 */
void FrameVisualEditor::updateBinaryPreview()
{
    rebuildDefinition();
    QStringList parts;
    int bytePos = 0;

    if (!m_def.header.isEmpty()) {
        int len = m_def.header.size();
        parts << tr("[帧头:%1-%2]").arg(bytePos).arg(bytePos + len - 1);
        bytePos += len;
    }
    if (m_def.lengthFieldOffset >= 0) {
        int start = m_def.lengthFieldOffset;
        if (start > bytePos) parts << tr("[预留:%1-%2]").arg(bytePos).arg(start - 1);
        parts << tr("[长度:%1-%2]").arg(start).arg(start + m_def.lengthFieldSize - 1);
        bytePos = start + m_def.lengthFieldSize;
    }
    for (const auto& f : m_def.fields) {
        int start = f.offset;
        if (start > bytePos) parts << tr("[预留:%1-%2]").arg(bytePos).arg(start - 1);
        const char* ts[] = {"u8","u16LE","u16BE","u32LE","u32BE","i8","i16LE","i16BE","f32","raw"};
        int typeIdx = qBound(0, static_cast<int>(f.type), 9);
        parts << tr("[%1(%2):%3-%4]").arg(f.name)
                     .arg(ts[typeIdx]).arg(start).arg(start + f.size - 1);
        bytePos = start + f.size;
    }
    if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
        int start = m_def.checksumOffset;
        if (start > bytePos) parts << tr("[预留:%1-%2]").arg(bytePos).arg(start - 1);
        const char* cn[] = {"","Sum8","CRC8","CRC16","CRC16M","CRC32"};
        int csIdx = qBound(0, static_cast<int>(m_def.checksumType), 5);
        parts << tr("[%1:%2-%3]").arg(cn[csIdx])
                     .arg(start).arg(start + m_def.checksumSize - 1);
        bytePos = start + m_def.checksumSize;
    }
    if (!m_def.footer.isEmpty()) {
        parts << tr("[帧尾:%1-%2]").arg(bytePos).arg(bytePos + m_def.footer.size() - 1);
    }
    m_previewLabel->setText(parts.isEmpty() ? tr("未定义字段") : parts.join(" "));
}

/** @brief 预览定时器回调，触发二进制预览刷新 */
void FrameVisualEditor::onPreviewTimerTick() { updateBinaryPreview(); }

/** @brief 重置帧编辑器统计计数器 */
void FrameVisualEditor::resetEditorStatistics()
{
    m_totalFramesBuilt = 0;
    m_totalSends = 0;
    m_totalEdits = 0;
    m_totalFieldAdds = 0;
    m_totalFieldRemoves = 0;
    m_totalFrameValidations = 0;
    m_totalProtocolLoads = 0;
    m_totalProtocolSaves = 0;
    m_totalFieldRearranges = 0;
    m_validationErrors = 0;
}
