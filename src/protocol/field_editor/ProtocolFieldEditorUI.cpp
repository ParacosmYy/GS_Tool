/**
 * @file ProtocolFieldEditorUI.cpp
 * @brief 协议字段结构编辑器 — UI构建、信号连接、槽函数、字段编辑对话框
 *
 * 从 ProtocolFieldEditor.cpp 拆分而来，包含:
 *   - setupUI(): 工具栏 + 字段表格 + 解析树 + 状态标签
 *   - setupConnections(): 信号/槽连接
 *   - 槽函数: onAddFieldClicked/onRemoveFieldClicked/onEditFieldClicked等
 *   - showFieldDialog(): 字段编辑对话框(名称/类型/偏移/位宽/枚举)
 *
 * 核心逻辑(字段CRUD/数据操作/JSON/解析)见 ProtocolFieldEditor.cpp
 */

#include "protocol/field_editor/ProtocolFieldEditor.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

// ============================================================
// UI 构建
// ============================================================

void ProtocolFieldEditor::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // ---- 工具栏 ----
    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(6);

    m_addBtn = new QPushButton(tr("添加字段"), this);
    m_addBtn->setObjectName("pfeAddBtn");
    m_addBtn->setToolTip(tr("添加新的协议字段定义"));

    m_removeBtn = new QPushButton(tr("删除字段"), this);
    m_removeBtn->setObjectName("pfeRemoveBtn");
    m_removeBtn->setEnabled(false);
    m_removeBtn->setToolTip(tr("删除选中的字段"));

    m_editBtn = new QPushButton(tr("编辑字段"), this);
    m_editBtn->setObjectName("pfeEditBtn");
    m_editBtn->setEnabled(false);
    m_editBtn->setToolTip(tr("编辑选中的字段属性"));

    toolbar->addWidget(m_addBtn);
    toolbar->addWidget(m_removeBtn);
    toolbar->addWidget(m_editBtn);
    toolbar->addStretch();

    m_importBtn = new QPushButton(tr("导入JSON"), this);
    m_importBtn->setObjectName("pfeImportBtn");
    m_importBtn->setToolTip(tr("从JSON文件导入字段定义模板"));

    m_exportBtn = new QPushButton(tr("导出JSON"), this);
    m_exportBtn->setObjectName("pfeExportBtn");
    m_exportBtn->setToolTip(tr("将当前字段定义导出为JSON模板"));

    toolbar->addWidget(m_importBtn);
    toolbar->addWidget(m_exportBtn);
    mainLayout->addLayout(toolbar);

    // ---- 分割器: 字段表格 / 解析树 ----
    auto* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setObjectName("pfeSplitter");

    m_fieldTable = new QTableWidget(0, 5, splitter);
    m_fieldTable->setObjectName("pfeFieldTable");
    m_fieldTable->setHorizontalHeaderLabels(
        {tr("名称"), tr("类型"), tr("位偏移"), tr("位宽"), tr("枚举")});
    m_fieldTable->horizontalHeader()->setStretchLastSection(true);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_fieldTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fieldTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_fieldTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_fieldTable->setAlternatingRowColors(true);

    m_interpTree = new QTreeWidget(splitter);
    m_interpTree->setObjectName("pfeInterpTree");
    m_interpTree->setHeaderLabels(
        {tr("字段"), tr("类型"), tr("偏移"), tr("值"), tr("原始HEX")});
    m_interpTree->header()->setStretchLastSection(true);
    m_interpTree->setAlternatingRowColors(true);
    m_interpTree->setRootIsDecorated(false);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    mainLayout->addWidget(splitter, 1);

    m_dataSizeLabel = new QLabel(tr("数据大小: 0 字节"), this);
    m_dataSizeLabel->setObjectName("pfeDataSizeLabel");
    mainLayout->addWidget(m_dataSizeLabel);
}

// ============================================================
// 信号连接
// ============================================================

void ProtocolFieldEditor::setupConnections()
{
    connect(m_addBtn, &QPushButton::clicked,
            this, &ProtocolFieldEditor::onAddFieldClicked);
    connect(m_removeBtn, &QPushButton::clicked,
            this, &ProtocolFieldEditor::onRemoveFieldClicked);
    connect(m_editBtn, &QPushButton::clicked,
            this, &ProtocolFieldEditor::onEditFieldClicked);
    connect(m_importBtn, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this, tr("导入字段定义"), QString(),
            tr("JSON文件 (*.json);;所有文件 (*)"));
        if (!path.isEmpty()) importFromJson(path);
    });
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getSaveFileName(
            this, tr("导出字段定义"), QString(),
            tr("JSON文件 (*.json);;所有文件 (*)"));
        if (!path.isEmpty()) exportToJson(path);
    });
    connect(m_fieldTable, &QTableWidget::itemSelectionChanged,
            this, &ProtocolFieldEditor::onFieldSelectionChanged);
    connect(m_fieldTable, &QTableWidget::cellDoubleClicked,
            this, &ProtocolFieldEditor::onFieldDoubleClicked);
}

// ============================================================
// 槽函数
// ============================================================

void ProtocolFieldEditor::onAddFieldClicked()
{
    bool ok = false;
    FieldDef def;
    def.name = tr("field_%1").arg(m_fields.size());
    def.type = FieldType::UInt8;
    def.bitOffset = 0;
    def.bitWidth = kDefaultBitWidth;
    const FieldDef result = showFieldDialog(def, tr("添加字段"), &ok);
    if (ok) addField(result);
}

void ProtocolFieldEditor::onRemoveFieldClicked()
{
    const int row = m_fieldTable->currentRow();
    if (row >= 0) removeField(row);
}

void ProtocolFieldEditor::onEditFieldClicked()
{
    const int row = m_fieldTable->currentRow();
    if (row < 0 || row >= m_fields.size()) return;
    bool ok = false;
    const FieldDef result = showFieldDialog(m_fields[row], tr("编辑字段"), &ok);
    if (ok) updateField(row, result);
}

void ProtocolFieldEditor::onFieldSelectionChanged()
{
    const bool has = (m_fieldTable->currentRow() >= 0);
    m_removeBtn->setEnabled(has);
    m_editBtn->setEnabled(has);
}

void ProtocolFieldEditor::onFieldDoubleClicked(int row, int /*column*/)
{
    if (row < 0 || row >= m_fields.size()) return;
    bool ok = false;
    const FieldDef result = showFieldDialog(m_fields[row], tr("编辑字段"), &ok);
    if (ok) updateField(row, result);
}

// ============================================================
// 字段编辑对话框
// ============================================================

ProtocolFieldEditor::FieldDef ProtocolFieldEditor::showFieldDialog(
    const FieldDef& field, const QString& title, bool* ok)
{
    QDialog dlg(this);
    dlg.setWindowTitle(title);
    dlg.setObjectName("pfeFieldDialog");

    auto* form = new QFormLayout(&dlg);
    form->setContentsMargins(16, 16, 16, 16);
    form->setSpacing(8);

    auto* nameEdit = new QLineEdit(field.name, &dlg);
    nameEdit->setObjectName("pfeDlgNameEdit");
    nameEdit->setPlaceholderText(tr("字段名称"));
    form->addRow(tr("名称:"), nameEdit);

    auto* typeCombo = new QComboBox(&dlg);
    typeCombo->setObjectName("pfeDlgTypeCombo");
    typeCombo->addItems({"UInt8", "Int8", "UInt16LE", "UInt16BE",
                         "UInt32LE", "UInt32BE", "Float32", "Float64",
                         "String", "Bytes", "Bool"});
    typeCombo->setCurrentIndex(static_cast<int>(field.type));
    form->addRow(tr("类型:"), typeCombo);

    auto* offsetSpin = new QSpinBox(&dlg);
    offsetSpin->setObjectName("pfeDlgOffsetSpin");
    offsetSpin->setRange(0, 65535);
    offsetSpin->setValue(field.bitOffset);
    offsetSpin->setSuffix(tr(" bit"));
    form->addRow(tr("位偏移:"), offsetSpin);

    auto* widthSpin = new QSpinBox(&dlg);
    widthSpin->setObjectName("pfeDlgWidthSpin");
    widthSpin->setRange(1, 256);
    widthSpin->setValue(field.bitWidth);
    widthSpin->setSuffix(tr(" bit"));
    form->addRow(tr("位宽:"), widthSpin);

    auto* enumEdit = new QLineEdit(field.enumName, &dlg);
    enumEdit->setObjectName("pfeDlgEnumEdit");
    enumEdit->setPlaceholderText(tr("可选枚举映射名称"));
    form->addRow(tr("枚举:"), enumEdit);

    auto* btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    btns->setObjectName("pfeDlgBtns");
    form->addRow(btns);

    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    FieldDef result;
    if (dlg.exec() == QDialog::Accepted && ok) {
        *ok = true;
        result.name = nameEdit->text().trimmed();
        if (result.name.isEmpty()) result.name = tr("未命名");
        result.type = static_cast<FieldType>(typeCombo->currentIndex());
        result.bitOffset = offsetSpin->value();
        result.bitWidth = widthSpin->value();
        result.enumName = enumEdit->text().trimmed();
    } else {
        *ok = false;
    }
    return result;
}
