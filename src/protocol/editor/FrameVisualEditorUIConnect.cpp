/**
 * @file FrameVisualEditorUIConnect.cpp
 * @brief 帧格式可视化编辑器 - 信号/槽连接方法
 *
 * 从FrameVisualEditorUI.cpp拆分而来，集中管理所有信号/槽连接:
 *   - setupConnections(): 应用/添加/删除/字段变更/配置变更/实时预览连接
 *
 * UI构建方法见FrameVisualEditorUI.cpp。
 * 数据读写与预览逻辑见FrameVisualEditor.cpp。
 * 字段操作方法见FrameVisualEditorFields.cpp。
 */

#include "protocol/editor/FrameVisualEditor.h"

/** @brief 连接所有信号/槽(应用/添加/删除/字段变更/实时预览) */
void FrameVisualEditor::setupConnections()
{
    connect(m_applyBtn, &QPushButton::clicked, this, &FrameVisualEditor::onApply);
    connect(m_addFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onAddField);
    connect(m_removeFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onRemoveField);
    connect(m_fieldTable, &QTableWidget::cellChanged, this, &FrameVisualEditor::onFieldChanged);

    /* 帧头/帧尾实时预览 */
    connect(m_headerEdit, &QLineEdit::textChanged,
            this, &FrameVisualEditor::onHeaderChanged);
    connect(m_footerEdit, &QLineEdit::textChanged,
            this, &FrameVisualEditor::onFooterChanged);

    /* 长度字段配置变更 → 实时预览 */
    connect(m_lengthOffsetSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onLengthConfigChanged);
    connect(m_lengthSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FrameVisualEditor::onLengthConfigChanged);
    connect(m_lengthBEndianCheck, &QCheckBox::checkStateChanged,
            this, &FrameVisualEditor::onLengthConfigChanged);
    connect(m_lengthAdjustSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onLengthConfigChanged);

    /* 校验配置变更 → 实时预览 */
    connect(m_checksumTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FrameVisualEditor::onChecksumConfigChanged);
    connect(m_checksumOffsetSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onChecksumConfigChanged);
    connect(m_checksumStartSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onChecksumConfigChanged);
}
