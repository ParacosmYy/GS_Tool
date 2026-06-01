/**
 * @file FrameVisualEditor.cpp
 * @brief 帧格式可视化编辑器实现
 *
 * 实现帧格式定义的完整编辑功能:
 *   - 帧头/帧尾/长度字段/校验的表单编辑
 *   - 数据字段表格管理(添加/删除/拖拽排序/上移下移)
 *   - 扩展字段类型(UInt8~Raw共10种)
 *   - 实时二进制布局预览(彩色块状图)
 *   - 字段属性编辑(名称/偏移/大小/字节序/缩放)
 */
#include "protocol/FrameVisualEditor.h"
#include "utils/HexConverter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

// ---- 静态辅助方法 ----

QStringList FrameVisualEditor::fieldTypeNames()
{
    return {"UInt8", "UInt16LE", "UInt16BE", "UInt32LE", "UInt32BE",
            "Int8", "Int16LE", "Int16BE", "Float", "Raw"};
}

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

FrameVisualEditor::FrameVisualEditor(QWidget* parent) : QWidget(parent) { setupUI(); }

/**
 * @brief 初始化UI控件和布局
 *
 * 结构:
 *   上半部分(水平): 帧头帧尾 | 长度字段 | 校验
 *   下半部分(垂直): 字段表格(可伸展) | 预览 | 应用按钮
 *
 * 三个配置组水平排列，节省垂直空间，避免控件挤到一起。
 */
void FrameVisualEditor::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ---- 上半部分: 三个配置组水平排列 ----
    auto* configLayout = new QHBoxLayout;
    configLayout->setSpacing(8);
    configLayout->addWidget(setupHeaderGroup());
    configLayout->addWidget(setupLengthGroup());
    configLayout->addWidget(setupChecksumGroup());
    mainLayout->addLayout(configLayout);

    // ---- 下半部分: 字段表格(可伸展) + 预览 + 应用 ----
    mainLayout->addWidget(setupFieldsGroup(), 1);
    mainLayout->addWidget(setupPreviewGroup());

    // ---- 应用按钮 ----
    m_applyBtn = new QPushButton(tr("应用定义"));
    m_applyBtn->setObjectName("applyDefBtn");
    m_applyBtn->setMinimumHeight(32);
    mainLayout->addWidget(m_applyBtn);

    setupConnections();
}

/**
 * @brief 创建帧头/帧尾配置分组
 * @return 帧头/帧尾GroupBox(包含帧头和帧尾HEX输入框)
 */
QGroupBox* FrameVisualEditor::setupHeaderGroup()
{
    auto* group = new QGroupBox(tr("帧头/帧尾配置"));
    group->setObjectName("frameHeaderGroup");
    auto* layout = new QFormLayout(group);
    m_headerEdit = new QLineEdit;
    m_headerEdit->setObjectName("frameHeaderEdit");
    m_headerEdit->setPlaceholderText("AA 55");
    m_headerEdit->setToolTip(tr("帧头HEX字节，如 AA 55"));
    layout->addRow(tr("帧头:"), m_headerEdit);
    m_footerEdit = new QLineEdit;
    m_footerEdit->setObjectName("frameFooterEdit");
    m_footerEdit->setPlaceholderText("0D 0A");
    m_footerEdit->setToolTip(tr("帧尾HEX字节（可选）"));
    layout->addRow(tr("帧尾:"), m_footerEdit);
    return group;
}

/**
 * @brief 创建长度字段配置分组
 * @return 长度字段GroupBox(包含偏移/大小/字节序/调整值)
 */
QGroupBox* FrameVisualEditor::setupLengthGroup()
{
    auto* group = new QGroupBox(tr("长度字段"));
    group->setObjectName("frameLengthGroup");
    auto* layout = new QFormLayout(group);
    m_lengthOffsetSpin = new QSpinBox;
    m_lengthOffsetSpin->setObjectName("frameLengthOffsetSpin");
    m_lengthOffsetSpin->setRange(-1, 255);
    m_lengthOffsetSpin->setValue(-1);
    m_lengthOffsetSpin->setSpecialValueText(tr("无"));
    layout->addRow(tr("偏移:"), m_lengthOffsetSpin);
    m_lengthSizeCombo = new QComboBox;
    m_lengthSizeCombo->setObjectName("frameLengthSizeCombo");
    m_lengthSizeCombo->addItems({"1 byte", "2 bytes"});
    layout->addRow(tr("大小:"), m_lengthSizeCombo);
    m_lengthBEndianCheck = new QCheckBox(tr("大端序"));
    m_lengthBEndianCheck->setObjectName("frameLengthEndianCheck");
    layout->addRow(m_lengthBEndianCheck);
    m_lengthAdjustSpin = new QSpinBox;
    m_lengthAdjustSpin->setObjectName("frameLengthAdjustSpin");
    m_lengthAdjustSpin->setRange(-256, 256);
    m_lengthAdjustSpin->setValue(0);
    m_lengthAdjustSpin->setToolTip(tr("实际负载 = 长度字段值 - 调整值"));
    layout->addRow(tr("调整:"), m_lengthAdjustSpin);
    return group;
}

/**
 * @brief 创建校验配置分组
 * @return 校验GroupBox(包含类型/偏移/起始偏移)
 */
QGroupBox* FrameVisualEditor::setupChecksumGroup()
{
    auto* group = new QGroupBox(tr("校验配置"));
    group->setObjectName("frameChecksumGroup");
    auto* layout = new QFormLayout(group);
    m_checksumTypeCombo = new QComboBox;
    m_checksumTypeCombo->setObjectName("frameChecksumTypeCombo");
    m_checksumTypeCombo->addItems({"None", "Sum8", "CRC8", "CRC16-CCITT", "CRC16-Modbus", "CRC32"});
    layout->addRow(tr("类型:"), m_checksumTypeCombo);
    m_checksumOffsetSpin = new QSpinBox;
    m_checksumOffsetSpin->setObjectName("frameChecksumOffsetSpin");
    m_checksumOffsetSpin->setRange(-1, 255);
    m_checksumOffsetSpin->setValue(-1);
    m_checksumOffsetSpin->setSpecialValueText(tr("自动"));
    layout->addRow(tr("偏移:"), m_checksumOffsetSpin);
    m_checksumStartSpin = new QSpinBox;
    m_checksumStartSpin->setObjectName("frameChecksumStartSpin");
    m_checksumStartSpin->setRange(0, 255);
    m_checksumStartSpin->setValue(0);
    layout->addRow(tr("起始:"), m_checksumStartSpin);
    return group;
}

/**
 * @brief 创建数据字段表格分组(含拖拽排序和上移/下移按钮)
 * @return 字段GroupBox(包含6列表格: 名称/类型/偏移/大小/字节序/缩放)
 */
QGroupBox* FrameVisualEditor::setupFieldsGroup()
{
    auto* group = new QGroupBox(tr("数据字段"));
    group->setObjectName("frameFieldGroup");
    auto* layout = new QVBoxLayout(group);

    // 字段定义表(支持拖拽排序)
    m_fieldTable = new QTableWidget(0, 6);
    m_fieldTable->setObjectName("frameFieldTable");
    m_fieldTable->setHorizontalHeaderLabels({
        tr("名称"), tr("类型"), tr("偏移"), tr("大小"), tr("字节序"), tr("缩放")
    });
    m_fieldTable->horizontalHeader()->setStretchLastSection(true);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_fieldTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fieldTable->setMinimumHeight(120);
    // 启用拖拽排序(行级拖放)
    m_fieldTable->setDragEnabled(true);
    m_fieldTable->setAcceptDrops(true);
    m_fieldTable->setDragDropMode(QAbstractItemView::InternalMove);
    m_fieldTable->setDefaultDropAction(Qt::MoveAction);
    m_fieldTable->setDragDropOverwriteMode(false);
    layout->addWidget(m_fieldTable);

    // 字段操作按钮行
    auto* btnLayout = new QHBoxLayout;
    m_addFieldBtn = new QPushButton(tr("添加字段"));
    m_addFieldBtn->setObjectName("frameAddFieldBtn");
    m_removeFieldBtn = new QPushButton(tr("删除字段"));
    m_removeFieldBtn->setObjectName("frameRemoveFieldBtn");
    auto* moveUpBtn = new QPushButton(tr("上移"));
    moveUpBtn->setObjectName("frameMoveUpBtn");
    moveUpBtn->setFixedWidth(60);
    auto* moveDownBtn = new QPushButton(tr("下移"));
    moveDownBtn->setObjectName("frameMoveDownBtn");
    moveDownBtn->setFixedWidth(60);
    btnLayout->addWidget(m_addFieldBtn);
    btnLayout->addWidget(m_removeFieldBtn);
    btnLayout->addWidget(moveUpBtn);
    btnLayout->addWidget(moveDownBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(moveUpBtn, &QPushButton::clicked, this, &FrameVisualEditor::onMoveFieldUp);
    connect(moveDownBtn, &QPushButton::clicked, this, &FrameVisualEditor::onMoveFieldDown);

    return group;
}

/** @brief 将当前选中行上移一行(交换所有列数据和控件) */
void FrameVisualEditor::onMoveFieldUp()
{
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

/**
 * @brief 创建二进制布局预览分组
 * @return 预览GroupBox(包含Consolas字体的预览标签)
 */
QGroupBox* FrameVisualEditor::setupPreviewGroup()
{
    auto* group = new QGroupBox(tr("二进制布局预览"));
    group->setObjectName("framePreviewGroup");
    auto* layout = new QVBoxLayout(group);
    m_previewLabel = new QLabel(tr("点击\"应用定义\"后显示布局"));
    m_previewLabel->setObjectName("framePreviewLabel");
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_previewLabel->setMinimumHeight(60);
    m_previewLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_previewLabel->setFont(QFont("Consolas", 10));
    layout->addWidget(m_previewLabel);
    return group;
}

/**
 * @brief 连接所有信号/槽(应用/添加/删除/字段变更/实时预览)
 */
void FrameVisualEditor::setupConnections()
{
    connect(m_applyBtn, &QPushButton::clicked, this, &FrameVisualEditor::onApply);
    connect(m_addFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onAddField);
    connect(m_removeFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onRemoveField);
    connect(m_fieldTable, &QTableWidget::cellChanged, this, &FrameVisualEditor::onFieldChanged);
    // 实时预览: 表格变化时更新
    connect(m_fieldTable, &QTableWidget::cellChanged, this, [this]() {
        if (!m_updating) updateBinaryPreview();
    });
}

// ---- 空实现(预留) ----
void FrameVisualEditor::onHeaderChanged() {}
void FrameVisualEditor::onFooterChanged() {}
void FrameVisualEditor::onLengthConfigChanged() {}
void FrameVisualEditor::onChecksumConfigChanged() {}

// ---- 数据读写 ----

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

void FrameVisualEditor::onApply()
{
    rebuildDefinition();
    updateBinaryPreview();
    emit definitionChanged(m_def);
}

// ---- 字段操作 ----

/** @brief 添加新字段(默认UInt8, 偏移0, 大小1, LE, 缩放1.0) */
void FrameVisualEditor::onAddField()
{
    int row = m_fieldTable->rowCount();
    m_fieldTable->insertRow(row);
    m_fieldTable->setItem(row, 0, new QTableWidgetItem(QString("field_%1").arg(row)));

    // 类型ComboBox(10种类型)
    auto* typeCombo = new QComboBox;
    typeCombo->setObjectName("fieldTypeCombo");  // QSS 选择器需要
    typeCombo->addItems(fieldTypeNames());
    m_fieldTable->setCellWidget(row, 1, typeCombo);
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, row](int) {
        if (m_updating) return;
        auto* sizeItem = m_fieldTable->item(row, 3);
        auto* cb = qobject_cast<QComboBox*>(m_fieldTable->cellWidget(row, 1));
        if (sizeItem && cb) sizeItem->setText(QString::number(typeSizeFromIndex(cb->currentIndex())));
        updateBinaryPreview();
    });

    m_fieldTable->setItem(row, 2, new QTableWidgetItem("0"));
    m_fieldTable->setItem(row, 3, new QTableWidgetItem("1"));

    // 字节序ComboBox(LE/BE/-)
    auto* endianCombo = new QComboBox;
    endianCombo->setObjectName("fieldEndianCombo");  // QSS 选择器需要
    endianCombo->addItems({"LE", "BE", "-"});
    endianCombo->setToolTip(tr("字节序: LE=小端, BE=大端, -=不适用"));
    m_fieldTable->setCellWidget(row, 4, endianCombo);

    m_fieldTable->setItem(row, 5, new QTableWidgetItem("1.0"));
}

void FrameVisualEditor::onRemoveField()
{
    int row = m_fieldTable->currentRow();
    if (row >= 0) {
        m_fieldTable->removeRow(row);
        if (!m_updating) updateBinaryPreview();
    }
}

void FrameVisualEditor::onFieldChanged(int row, int col)
{
    Q_UNUSED(row); Q_UNUSED(col);
}

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
        endianCombo->addItems({"LE", "BE", "-"});
        bool isBE = (field.type == FieldDef::UInt16BE || field.type == FieldDef::Int16BE);
        bool noEnd = (field.type == FieldDef::UInt8 || field.type == FieldDef::Int8 ||
                      field.type == FieldDef::Float || field.type == FieldDef::Raw);
        endianCombo->setCurrentIndex(noEnd ? 2 : (isBE ? 1 : 0));
        m_fieldTable->setCellWidget(row, 4, endianCombo);
        m_fieldTable->setItem(row, 5, new QTableWidgetItem(QString::number(field.scale)));
    }
}

/**
 * @brief 更新二进制布局预览
 * 格式: [帧头:0-1] [温度(u16LE):2-3] [电压(u32LE):4-7] [CRC16:8-9]
 */
void FrameVisualEditor::updateBinaryPreview()
{
    rebuildDefinition();
    QStringList parts;
    int bytePos = 0;

    if (!m_def.header.isEmpty()) {
        int len = m_def.header.size();
        parts << QString("[帧头:%1-%2]").arg(bytePos).arg(bytePos + len - 1);
        bytePos += len;
    }
    if (m_def.lengthFieldOffset >= 0) {
        int start = m_def.lengthFieldOffset;
        if (start > bytePos) parts << QString("[预留:%1-%2]").arg(bytePos).arg(start - 1);
        parts << QString("[长度:%1-%2]").arg(start).arg(start + m_def.lengthFieldSize - 1);
        bytePos = start + m_def.lengthFieldSize;
    }
    for (const auto& f : m_def.fields) {
        int start = f.offset;
        if (start > bytePos) parts << QString("[预留:%1-%2]").arg(bytePos).arg(start - 1);
        const char* ts[] = {"u8","u16LE","u16BE","u32LE","u32BE","i8","i16LE","i16BE","f32","raw"};
        parts << QString("[%1(%2):%3-%4]").arg(f.name)
                     .arg(ts[static_cast<int>(f.type)]).arg(start).arg(start + f.size - 1);
        bytePos = start + f.size;
    }
    if (m_def.checksumType != ChecksumType::None && m_def.checksumOffset >= 0) {
        int start = m_def.checksumOffset;
        if (start > bytePos) parts << QString("[预留:%1-%2]").arg(bytePos).arg(start - 1);
        const char* cn[] = {"","Sum8","CRC8","CRC16","CRC16M","CRC32"};
        parts << QString("[%1:%2-%3]").arg(cn[static_cast<int>(m_def.checksumType)])
                     .arg(start).arg(start + m_def.checksumSize - 1);
        bytePos = start + m_def.checksumSize;
    }
    if (!m_def.footer.isEmpty()) {
        parts << QString("[帧尾:%1-%2]").arg(bytePos).arg(bytePos + m_def.footer.size() - 1);
    }
    m_previewLabel->setText(parts.isEmpty() ? tr("未定义字段") : parts.join(" "));
}

void FrameVisualEditor::onPreviewTimerTick() { updateBinaryPreview(); }
