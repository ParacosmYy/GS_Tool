/**
 * @file FrameVisualEditor.cpp
 * @brief 帧格式可视化编辑器实现 - 数据读写、定义重建、预览更新
 *
 * 实现帧格式定义的编辑逻辑:
 *   - 数据读写(currentDefinition/setDefinition)
 *   - 定义重建(rebuildDefinition)与二进制预览更新(updateBinaryPreview)
 *   - 配置变更回调(帧头/帧尾/长度/校验)
 *   - 静态辅助(fieldTypeNames/typeSizeFromIndex)
 *
 * 字段操作方法见 FrameVisualEditorFields.cpp:
 *   onAddField / onRemoveField / onMoveFieldUp / onMoveFieldDown
 *   onFieldChanged / updateFieldTable
 *
 * UI构建方法见 FrameVisualEditorUI.cpp
 */
#include "protocol/editor/FrameVisualEditor.h"
#include "utils/crypto/HexConverter.h"
#include <QHeaderView>
#include <QDebug>

// ---- 静态辅助方法 ----

/** @brief 获取所有可用的字段类型名称(UInt8/UInt16LE/Float/Raw等) @return 类型名QStringList */
QStringList FrameVisualEditor::fieldTypeNames()
{
    return {"UInt8", "UInt16LE", "UInt16BE", "UInt32LE", "UInt32BE",
            "Int8", "Int16LE", "Int16BE", "Float", "Raw"};
}

/** @brief 根据数据类型索引返回该类型占用的字节数 @param typeIndex 类型ComboBox索引 @return 字节数(1/2/4) */
int FrameVisualEditor::typeSizeFromIndex(int typeIndex) const
{
    switch (typeIndex) {
    case 0: return 1; case 1: return 2; case 2: return 2;
    case 3: return 4; case 4: return 4; case 5: return 1;
    case 6: return 2; case 7: return 2; case 8: return 4;
    case 9: return 1; default: return 1;
    }
}

// ============================================================
// 构造与UI
// ============================================================

/** @brief 构造函数，初始化UI @param parent 父控件 */
FrameVisualEditor::FrameVisualEditor(QWidget* parent) : QWidget(parent) { setupUI(); }

// UI构建方法见 FrameVisualEditorUI.cpp
// 字段操作方法见 FrameVisualEditorFields.cpp:
//   onMoveFieldUp / onMoveFieldDown / onAddField / onRemoveField
//   onFieldChanged / updateFieldTable

// ---- 配置变更回调（触发实时预览刷新） ----

/** @brief 帧头变更回调 — 实时更新二进制布局预览 */
void FrameVisualEditor::onHeaderChanged()
{
    if (!m_updating) { ++m_totalEdits; updateBinaryPreview(); }
}

/** @brief 帧尾变更回调 — 实时更新二进制布局预览 */
void FrameVisualEditor::onFooterChanged()
{
    if (!m_updating) { ++m_totalEdits; updateBinaryPreview(); }
}

/** @brief 长度字段配置变更回调 — 实时更新二进制布局预览 */
void FrameVisualEditor::onLengthConfigChanged()
{
    if (!m_updating) { ++m_totalEdits; updateBinaryPreview(); }
}

/** @brief 校验配置变更回调 — 实时更新二进制布局预览 */
void FrameVisualEditor::onChecksumConfigChanged()
{
    if (!m_updating) { ++m_totalEdits; updateBinaryPreview(); }
}

// ---- 数据读写 ----

/** @brief 返回当前编辑器中的帧定义 @return 当前FrameDefinition */
FrameDefinition FrameVisualEditor::currentDefinition() const { return m_def; }

/** @brief 从外部FrameDefinition加载到编辑器UI */
void FrameVisualEditor::setDefinition(const FrameDefinition& def)
{
    m_updating = true;
    m_def = def;
    m_headerEdit->setText(HexConverter::toHexString(def.header));
    m_footerEdit->setText(HexConverter::toHexString(def.footer));
    m_lengthOffsetSpin->setValue(def.lengthFieldOffset);
    m_lengthSizeCombo->setCurrentIndex(def.lengthFieldSize - 1);
    m_lengthBEndianCheck->setChecked(def.lengthBigEndian);
    m_lengthAdjustSpin->setValue(def.lengthAdjust);
    m_checksumTypeCombo->setCurrentIndex(static_cast<int>(def.checksumType));
    m_checksumOffsetSpin->setValue(def.checksumOffset);
    m_checksumStartSpin->setValue(def.checksumStart);
    updateFieldTable();
    updateBinaryPreview();
    m_updating = false;
}

/** @brief 应用当前编辑器配置，发射definitionChanged信号 */
void FrameVisualEditor::onApply()
{
    rebuildDefinition();
    updateBinaryPreview();
    ++m_totalFramesBuilt;  ///< 统计: 帧构建
    ++m_totalSends;        ///< 统计: 应用=发送
    emit definitionChanged(m_def);
}

// ---- 字段操作方法见 FrameVisualEditorFields.cpp ----
// onAddField / onRemoveField / onFieldChanged / updateFieldTable

// ---- 内部更新 ----

/** @brief 从UI控件收集数据重建FrameDefinition */
void FrameVisualEditor::rebuildDefinition()
{
    m_def.header = HexConverter::fromHexString(m_headerEdit->text());
    m_def.footer = HexConverter::fromHexString(m_footerEdit->text());
    m_def.lengthFieldOffset = m_lengthOffsetSpin->value();
    m_def.lengthFieldSize = m_lengthSizeCombo->currentIndex() + 1;
    m_def.lengthBigEndian = m_lengthBEndianCheck->isChecked();
    m_def.lengthAdjust = m_lengthAdjustSpin->value();
    m_def.checksumType = static_cast<ChecksumType>(m_checksumTypeCombo->currentIndex());
    m_def.checksumOffset = m_checksumOffsetSpin->value();
    m_def.checksumStart = m_checksumStartSpin->value();
    // 校验字节数: None=0, Sum8/CRC8=1, CRC16*=2, CRC32=4
    static constexpr int csSizes[] = {0, 1, 1, 2, 2, 4};
    m_def.checksumSize = (static_cast<int>(m_def.checksumType) < 6)
        ? csSizes[static_cast<int>(m_def.checksumType)] : 0;
    m_def.checksumEnd = m_def.checksumOffset;

    m_def.fields.clear();
    for (int i = 0; i < m_fieldTable->rowCount(); ++i) {
        FieldDef field;
        auto* nameItem = m_fieldTable->item(i, 0);
        field.name = nameItem ? nameItem->text() : QString("field_%1").arg(i);
        auto* typeCombo = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(i, 1));
        field.type = typeCombo ? static_cast<FieldDef::Type>(typeCombo->currentIndex()) : FieldDef::UInt8;
        // 读取字节序ComboBox，调整类型中的LE/BE标记
        if (auto* ec = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(i, 4))) {
            bool be = (ec->currentText() == "BE");
            if (be && field.type == FieldDef::UInt16LE) field.type = FieldDef::UInt16BE;
            else if (be && field.type == FieldDef::UInt32LE) field.type = FieldDef::UInt32BE;
            else if (be && field.type == FieldDef::Int16LE) field.type = FieldDef::Int16BE;
            else if (!be && field.type == FieldDef::UInt16BE) field.type = FieldDef::UInt16LE;
            else if (!be && field.type == FieldDef::UInt32BE) field.type = FieldDef::UInt32LE;
            else if (!be && field.type == FieldDef::Int16BE) field.type = FieldDef::Int16LE;
        }
        auto* offsetItem = m_fieldTable->item(i, 2);
        field.offset = offsetItem ? offsetItem->text().toInt() : 0;
        auto* sizeItem = m_fieldTable->item(i, 3);
        field.size = sizeItem ? sizeItem->text().toInt() : 1;
        auto* scaleItem = m_fieldTable->item(i, 5);
        field.scale = scaleItem ? scaleItem->text().toDouble() : 1.0;
        m_def.fields.append(field);
    }
}

/**
 * @brief 更新二进制布局预览
 * 格式: [帧头:0-1] [温度(u16LE):2-3] [电压(u32LE):4-7] [CRC16:8-9]
 */
/** @brief 更新二进制预览区，根据当前字段配置生成示例帧的HEX显示 */
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
}
