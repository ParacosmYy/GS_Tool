/**
 * @file FrameVisualEditorFields.cpp
 * @brief 帧格式可视化编辑器 - 字段操作与表格管理方法
 *
 * 从 FrameVisualEditor.cpp 拆分，包含:
 *   - onAddField / onRemoveField: 字段增删
 *   - onMoveFieldUp / onMoveFieldDown: 字段上下移动(行交换)
 *   - onFieldChanged: 表格单元格变更回调
 *   - updateFieldTable: 从FrameDefinition填充字段表格
 *
 * UI构建方法见 FrameVisualEditorUI.cpp
 * 数据读写与预览方法见 FrameVisualEditor.cpp
 */

#include "protocol/editor/FrameVisualEditor.h"

/** @brief 将当前选中行上移一行(交换所有列数据和控件) */
void FrameVisualEditor::onMoveFieldUp()
{
    ++m_totalFieldRearranges;
    int row = m_fieldTable->currentRow();
    if (row <= 0) return;
    for (int col = 0; col < m_fieldTable->columnCount(); ++col) {
        auto* a = m_fieldTable->takeItem(row, col);
        auto* b = m_fieldTable->takeItem(row - 1, col);
        m_fieldTable->setItem(row, col, b);
        m_fieldTable->setItem(row - 1, col, a);
    }
    for (int col : {1, 4}) {
        auto* w1 = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(row, col));
        auto* w2 = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(row - 1, col));
        if (w1 && w2) { int t = w1->currentIndex(); w1->setCurrentIndex(w2->currentIndex()); w2->setCurrentIndex(t); }
    }
    m_fieldTable->selectRow(row - 1);
}

/** @brief 将当前选中行下移一行(交换所有列数据和控件) */
void FrameVisualEditor::onMoveFieldDown()
{
    ++m_totalFieldRearranges;
    int row = m_fieldTable->currentRow();
    if (row < 0 || row >= m_fieldTable->rowCount() - 1) return;
    for (int col = 0; col < m_fieldTable->columnCount(); ++col) {
        auto* a = m_fieldTable->takeItem(row, col);
        auto* b = m_fieldTable->takeItem(row + 1, col);
        m_fieldTable->setItem(row, col, b);
        m_fieldTable->setItem(row + 1, col, a);
    }
    for (int col : {1, 4}) {
        auto* w1 = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(row, col));
        auto* w2 = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(row + 1, col));
        if (w1 && w2) { int t = w1->currentIndex(); w1->setCurrentIndex(w2->currentIndex()); w2->setCurrentIndex(t); }
    }
    m_fieldTable->selectRow(row + 1);
}

// setupPreviewGroup() 和 setupConnections() 见 FrameVisualEditorUI.cpp

// ---- 字段操作 ----

/** @brief 添加新字段(默认UInt8, 偏移0, 大小1, LE, 缩放1.0) */
void FrameVisualEditor::onAddField()
{
    ++m_totalFieldAdds; ///< 统计: 字段添加
    int row = m_fieldTable->rowCount();
    m_fieldTable->insertRow(row);
    m_fieldTable->setItem(row, 0, new QTableWidgetItem(tr("field_%1").arg(row)));

    // 类型ComboBox(10种类型)
    auto* typeCombo = new QComboBox;
    typeCombo->setObjectName("fieldTypeCombo");  // QSS 选择器需要
    typeCombo->addItems(fieldTypeNames());
    m_fieldTable->setCellWidget(row, 1, typeCombo);
    // 注意: 不捕获row，因为removeRow/drag-drop会改变行号
    // 通过遍历cellWidget动态查找当前行，保证删除/移动字段后索引始终正确
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, typeCombo](int) {
        if (m_updating) return;
        // 动态查找typeCombo所在行(避免removeRow后captured row失效)
        int currentRow = -1;
        for (int r = 0; r < m_fieldTable->rowCount(); ++r) {
            if (m_fieldTable->cellWidget(r, 1) == typeCombo) { currentRow = r; break; }
        }
        if (currentRow < 0) return;
        auto* sizeItem = m_fieldTable->item(currentRow, 3);
        if (sizeItem) sizeItem->setText(QString::number(typeSizeFromIndex(typeCombo->currentIndex())));
        updateBinaryPreview();
    });

    m_fieldTable->setItem(row, 2, new QTableWidgetItem("0"));
    m_fieldTable->setItem(row, 3, new QTableWidgetItem("1"));

    // 字节序ComboBox(LE/BE/-)
    auto* endianCombo = new QComboBox;
    endianCombo->setObjectName("fieldEndianCombo");  // QSS 选择器需要
    endianCombo->addItems({tr("LE"), tr("BE"), tr("-")});
    endianCombo->setToolTip(tr("字节序: LE=小端, BE=大端, -=不适用"));
    m_fieldTable->setCellWidget(row, 4, endianCombo);

    m_fieldTable->setItem(row, 5, new QTableWidgetItem("1.0"));
}

/** @brief 删除当前选中行的字段 */
void FrameVisualEditor::onRemoveField()
{
    int row = m_fieldTable->currentRow();
    if (row >= 0) {
        ++m_totalFieldRemoves; ///< 统计: 字段删除
        m_fieldTable->removeRow(row);
        if (!m_updating) updateBinaryPreview();
    }
}

/** @brief 字段表格单元格变更回调 @param row 行号 @param col 列号 */
void FrameVisualEditor::onFieldChanged(int row, int col)
{
    Q_UNUSED(row)
    Q_UNUSED(col)
    if (!m_updating) { ++m_totalEdits; updateBinaryPreview(); }
}

/** @brief 从FrameDefinition填充字段表格(含类型/字节序ComboBox) */
void FrameVisualEditor::updateFieldTable()
{
    m_fieldTable->setRowCount(0);
    for (const auto& field : m_def.fields) {
        int row = m_fieldTable->rowCount();
        m_fieldTable->insertRow(row);
        m_fieldTable->setItem(row, 0, new QTableWidgetItem(field.name));

        auto* typeCombo = new QComboBox;
        typeCombo->setObjectName("fieldTypeCombo");  // QSS 选择器需要
        typeCombo->addItems(fieldTypeNames());
        typeCombo->setCurrentIndex(static_cast<int>(field.type));
        m_fieldTable->setCellWidget(row, 1, typeCombo);
        connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int) { if (!m_updating) updateBinaryPreview(); });

        m_fieldTable->setItem(row, 2, new QTableWidgetItem(QString::number(field.offset)));
        m_fieldTable->setItem(row, 3, new QTableWidgetItem(QString::number(field.size)));

        auto* endianCombo = new QComboBox;
        endianCombo->setObjectName("fieldEndianCombo");  // QSS 选择器需要
        endianCombo->addItems({tr("LE"), tr("BE"), tr("-")});
        bool isBE = (field.type == FieldDef::UInt16BE || field.type == FieldDef::Int16BE);
        bool noEnd = (field.type == FieldDef::UInt8 || field.type == FieldDef::Int8 ||
                      field.type == FieldDef::Float || field.type == FieldDef::Raw);
        endianCombo->setCurrentIndex(noEnd ? 2 : (isBE ? 1 : 0));
        m_fieldTable->setCellWidget(row, 4, endianCombo);
        m_fieldTable->setItem(row, 5, new QTableWidgetItem(QString::number(field.scale)));
    }
}
