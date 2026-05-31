#include "FrameVisualEditor.h"
#include "utils/HexConverter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

FrameVisualEditor::FrameVisualEditor(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void FrameVisualEditor::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);

    // ---- 帧头/帧尾配置 ----
    auto* headerGroup = new QGroupBox(tr("帧头/帧尾配置"));
    auto* headerLayout = new QFormLayout(headerGroup);

    m_headerEdit = new QLineEdit;
    m_headerEdit->setPlaceholderText("AA 55");
    m_headerEdit->setToolTip(tr("帧头HEX字节，如 AA 55"));
    headerLayout->addRow(tr("帧头:"), m_headerEdit);

    m_footerEdit = new QLineEdit;
    m_footerEdit->setPlaceholderText("0D 0A");
    m_footerEdit->setToolTip(tr("帧尾HEX字节（可选）"));
    headerLayout->addRow(tr("帧尾:"), m_footerEdit);

    mainLayout->addWidget(headerGroup);

    // ---- 长度字段配置 ----
    auto* lengthGroup = new QGroupBox(tr("长度字段"));
    auto* lengthLayout = new QFormLayout(lengthGroup);

    m_lengthOffsetSpin = new QSpinBox;
    m_lengthOffsetSpin->setRange(-1, 255);
    m_lengthOffsetSpin->setValue(-1);
    m_lengthOffsetSpin->setSpecialValueText(tr("无"));
    lengthLayout->addRow(tr("偏移:"), m_lengthOffsetSpin);

    m_lengthSizeCombo = new QComboBox;
    m_lengthSizeCombo->addItems({"1 byte", "2 bytes"});
    lengthLayout->addRow(tr("大小:"), m_lengthSizeCombo);

    m_lengthBEndianCheck = new QCheckBox(tr("大端序"));
    lengthLayout->addRow(m_lengthBEndianCheck);

    m_lengthAdjustSpin = new QSpinBox;
    m_lengthAdjustSpin->setRange(-256, 256);
    m_lengthAdjustSpin->setValue(0);
    m_lengthAdjustSpin->setToolTip(tr("实际负载 = 长度字段值 - 调整值"));
    lengthLayout->addRow(tr("调整:"), m_lengthAdjustSpin);

    mainLayout->addWidget(lengthGroup);

    // ---- 校验配置 ----
    auto* checksumGroup = new QGroupBox(tr("校验配置"));
    auto* checksumLayout = new QFormLayout(checksumGroup);

    m_checksumTypeCombo = new QComboBox;
    m_checksumTypeCombo->addItems({"None", "Sum8", "CRC8", "CRC16-CCITT", "CRC16-Modbus", "CRC32"});
    checksumLayout->addRow(tr("类型:"), m_checksumTypeCombo);

    m_checksumOffsetSpin = new QSpinBox;
    m_checksumOffsetSpin->setRange(-1, 255);
    m_checksumOffsetSpin->setValue(-1);
    m_checksumOffsetSpin->setSpecialValueText(tr("自动"));
    checksumLayout->addRow(tr("偏移:"), m_checksumOffsetSpin);

    m_checksumStartSpin = new QSpinBox;
    m_checksumStartSpin->setRange(0, 255);
    m_checksumStartSpin->setValue(0);
    checksumLayout->addRow(tr("起始:"), m_checksumStartSpin);

    mainLayout->addWidget(checksumGroup);

    // ---- 字段定义表 ----
    auto* fieldGroup = new QGroupBox(tr("数据字段"));
    auto* fieldLayout = new QVBoxLayout(fieldGroup);

    m_fieldTable = new QTableWidget(0, 5);
    m_fieldTable->setHorizontalHeaderLabels({
        tr("名称"), tr("类型"), tr("偏移"), tr("大小"), tr("单位")
    });
    m_fieldTable->horizontalHeader()->setStretchLastSection(true);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_fieldTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fieldTable->setMinimumHeight(120);
    fieldLayout->addWidget(m_fieldTable);

    auto* fieldBtnLayout = new QHBoxLayout;
    m_addFieldBtn = new QPushButton(tr("添加字段"));
    m_removeFieldBtn = new QPushButton(tr("删除字段"));
    fieldBtnLayout->addWidget(m_addFieldBtn);
    fieldBtnLayout->addWidget(m_removeFieldBtn);
    fieldBtnLayout->addStretch();
    fieldLayout->addLayout(fieldBtnLayout);

    mainLayout->addWidget(fieldGroup, 1);

    // ---- 应用按钮 ----
    m_applyBtn = new QPushButton(tr("应用定义"));
    m_applyBtn->setObjectName("applyDefBtn");
    m_applyBtn->setMinimumHeight(32);
    mainLayout->addWidget(m_applyBtn);

    // 信号连接
    connect(m_applyBtn, &QPushButton::clicked, this, &FrameVisualEditor::onApply);
    connect(m_addFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onAddField);
    connect(m_removeFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onRemoveField);
    connect(m_fieldTable, &QTableWidget::cellChanged, this, &FrameVisualEditor::onFieldChanged);
}

void FrameVisualEditor::onHeaderChanged() {}
void FrameVisualEditor::onFooterChanged() {}
void FrameVisualEditor::onLengthConfigChanged() {}
void FrameVisualEditor::onChecksumConfigChanged() {}

FrameDefinition FrameVisualEditor::currentDefinition() const
{
    return m_def;
}

void FrameVisualEditor::setDefinition(const FrameDefinition& def)
{
    m_updating = true;
    m_def = def;

    // 填充帧头/帧尾
    m_headerEdit->setText(HexConverter::toHexString(def.header));
    m_footerEdit->setText(HexConverter::toHexString(def.footer));

    // 长度字段
    m_lengthOffsetSpin->setValue(def.lengthFieldOffset);
    m_lengthSizeCombo->setCurrentIndex(def.lengthFieldSize - 1);
    m_lengthBEndianCheck->setChecked(def.lengthBigEndian);
    m_lengthAdjustSpin->setValue(def.lengthAdjust);

    // 校验
    m_checksumTypeCombo->setCurrentIndex(static_cast<int>(def.checksumType));
    m_checksumOffsetSpin->setValue(def.checksumOffset);
    m_checksumStartSpin->setValue(def.checksumStart);

    // 字段表
    updateFieldTable();
    m_updating = false;
}

void FrameVisualEditor::onApply()
{
    rebuildDefinition();
    emit definitionChanged(m_def);
}

void FrameVisualEditor::onAddField()
{
    int row = m_fieldTable->rowCount();
    m_fieldTable->insertRow(row);
    m_fieldTable->setItem(row, 0, new QTableWidgetItem(QString("field_%1").arg(row)));
    auto* typeCombo = new QComboBox;
    typeCombo->addItems({"UInt8", "UInt16LE", "UInt16BE", "UInt32LE", "UInt32BE",
                          "Int8", "Int16LE", "Int16BE", "Float", "Raw"});
    m_fieldTable->setCellWidget(row, 1, typeCombo);
    m_fieldTable->setItem(row, 2, new QTableWidgetItem("0"));
    m_fieldTable->setItem(row, 3, new QTableWidgetItem("1"));
    m_fieldTable->setItem(row, 4, new QTableWidgetItem(""));
}

void FrameVisualEditor::onRemoveField()
{
    int row = m_fieldTable->currentRow();
    if (row >= 0) {
        m_fieldTable->removeRow(row);
    }
}

void FrameVisualEditor::onFieldChanged(int row, int col)
{
    Q_UNUSED(row);
    Q_UNUSED(col);
}

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
    switch (m_def.checksumType) {
    case ChecksumType::None: m_def.checksumSize = 0; break;
    case ChecksumType::Sum8: m_def.checksumSize = 1; break;
    case ChecksumType::CRC8: m_def.checksumSize = 1; break;
    case ChecksumType::CRC16CCITT: m_def.checksumSize = 2; break;
    case ChecksumType::CRC16Modbus: m_def.checksumSize = 2; break;
    case ChecksumType::CRC32: m_def.checksumSize = 4; break;
    }
    m_def.checksumEnd = m_def.checksumOffset;

    // 重建字段列表
    m_def.fields.clear();
    for (int i = 0; i < m_fieldTable->rowCount(); ++i) {
        FieldDef field;
        auto* nameItem = m_fieldTable->item(i, 0);
        field.name = nameItem ? nameItem->text() : QString("field_%1").arg(i);

        auto* typeCombo = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(i, 1));
        field.type = typeCombo ? static_cast<FieldDef::Type>(typeCombo->currentIndex()) : FieldDef::UInt8;

        auto* offsetItem = m_fieldTable->item(i, 2);
        field.offset = offsetItem ? offsetItem->text().toInt() : 0;

        auto* sizeItem = m_fieldTable->item(i, 3);
        field.size = sizeItem ? sizeItem->text().toInt() : 1;

        auto* unitItem = m_fieldTable->item(i, 4);
        field.unit = unitItem ? unitItem->text() : "";

        m_def.fields.append(field);
    }
}

void FrameVisualEditor::updateFieldTable()
{
    m_fieldTable->setRowCount(0);
    for (int i = 0; i < m_def.fields.size(); ++i) {
        const auto& field = m_def.fields[i];
        int row = m_fieldTable->rowCount();
        m_fieldTable->insertRow(row);

        m_fieldTable->setItem(row, 0, new QTableWidgetItem(field.name));

        auto* typeCombo = new QComboBox;
        typeCombo->addItems({"UInt8", "UInt16LE", "UInt16BE", "UInt32LE", "UInt32BE",
                              "Int8", "Int16LE", "Int16BE", "Float", "Raw"});
        typeCombo->setCurrentIndex(static_cast<int>(field.type));
        m_fieldTable->setCellWidget(row, 1, typeCombo);

        m_fieldTable->setItem(row, 2, new QTableWidgetItem(QString::number(field.offset)));
        m_fieldTable->setItem(row, 3, new QTableWidgetItem(QString::number(field.size)));
        m_fieldTable->setItem(row, 4, new QTableWidgetItem(field.unit));
    }
}
