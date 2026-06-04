/**
 * @file DataMaskEditor.cpp
 * @brief 数据掩码编辑器实现 — 构造函数、UI布局、位操作、字段管理
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * JSON 导入导出见：@see DataMaskEditorIo.cpp
 * 统计查询与重置方法见：@see DataMaskEditorStats.cpp
 */

#include "utils/bitmask/DataMaskEditor.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化掩码编辑器
 */
DataMaskEditor::DataMaskEditor(QWidget *parent)
    : QWidget(parent)
    , m_bits(8, false)
{
    setupUI();
}

// ──────────────────────────────────────────────
// UI 初始化
// ──────────────────────────────────────────────

/**
 * @brief 初始化界面布局，所有控件设置 objectName 供 QSS 匹配
 */
void DataMaskEditor::setupUI()
{
    setObjectName(QStringLiteral("DataMaskEditor"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(6);

    // ── 顶部：位宽选择 ──
    auto *widthLayout = new QHBoxLayout();
    auto *widthLabel = new QLabel(tr("位宽："), this);
    widthLabel->setObjectName("dmeWidthLabel");
    widthLayout->addWidget(widthLabel);

    m_bitWidthCombo = new QComboBox(this);
    m_bitWidthCombo->setObjectName("dmeBitWidthCombo");
    m_bitWidthCombo->addItem(tr("8 位"), 8);
    m_bitWidthCombo->addItem(tr("16 位"), 16);
    m_bitWidthCombo->addItem(tr("32 位"), 32);
    m_bitWidthCombo->addItem(tr("64 位"), 64);
    m_bitWidthCombo->setCurrentIndex(0);
    widthLayout->addWidget(m_bitWidthCombo);
    widthLayout->addStretch();

    mainLayout->addLayout(widthLayout);

    // ── 位按钮网格：按高位到低位排列，每行8个 ──
    auto *bitGridLayout = new QGridLayout();
    bitGridLayout->setSpacing(2);
    rebuildBitButtons();
    mainLayout->addLayout(bitGridLayout);

    // ── 数值显示行 ──
    auto *valueLayout = new QHBoxLayout();

    auto *hexLabel = new QLabel(tr("HEX："), this);
    hexLabel->setObjectName("dmeHexLabel");
    valueLayout->addWidget(hexLabel);

    m_hexValueEdit = new QLineEdit(this);
    m_hexValueEdit->setObjectName("dmeHexValue");
    m_hexValueEdit->setReadOnly(true);
    m_hexValueEdit->setPlaceholderText(tr("0x00"));
    valueLayout->addWidget(m_hexValueEdit);

    auto *binLabel = new QLabel(tr("BIN："), this);
    binLabel->setObjectName("dmeBinLabel");
    valueLayout->addWidget(binLabel);

    m_binValueEdit = new QLineEdit(this);
    m_binValueEdit->setObjectName("dmeBinValue");
    m_binValueEdit->setReadOnly(true);
    m_binValueEdit->setPlaceholderText(tr("00000000"));
    valueLayout->addWidget(m_binValueEdit);

    auto *decLabel = new QLabel(tr("DEC："), this);
    decLabel->setObjectName("dmeDecLabel");
    valueLayout->addWidget(decLabel);

    m_decValueEdit = new QLineEdit(this);
    m_decValueEdit->setObjectName("dmeDecValue");
    m_decValueEdit->setReadOnly(true);
    m_decValueEdit->setPlaceholderText(tr("0"));
    valueLayout->addWidget(m_decValueEdit);

    mainLayout->addLayout(valueLayout);

    // ── 字段定义表格 ──
    m_fieldTable = new QTableWidget(this);
    m_fieldTable->setObjectName("dmeFieldTable");
    m_fieldTable->setColumnCount(5);
    m_fieldTable->setHorizontalHeaderLabels(
        {tr("名称"), tr("起始位"), tr("结束位"), tr("颜色"), tr("提取值")});
    m_fieldTable->horizontalHeader()->setStretchLastSection(true);
    m_fieldTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fieldTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_fieldTable);

    // ── 底部工具栏 ──
    auto *toolbarLayout = new QHBoxLayout();

    m_addFieldBtn = new QPushButton(tr("添加字段"), this);
    m_addFieldBtn->setObjectName("dmeAddFieldBtn");
    toolbarLayout->addWidget(m_addFieldBtn);

    m_removeFieldBtn = new QPushButton(tr("删除字段"), this);
    m_removeFieldBtn->setObjectName("dmeRemoveFieldBtn");
    toolbarLayout->addWidget(m_removeFieldBtn);

    toolbarLayout->addStretch();

    m_exportBtn = new QPushButton(tr("导出 JSON"), this);
    m_exportBtn->setObjectName("dmeExportBtn");
    toolbarLayout->addWidget(m_exportBtn);

    m_importBtn = new QPushButton(tr("导入 JSON"), this);
    m_importBtn->setObjectName("dmeImportBtn");
    toolbarLayout->addWidget(m_importBtn);

    mainLayout->addLayout(toolbarLayout);

    // ── 信号连接 ──
    connect(m_bitWidthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataMaskEditor::onBitWidthChanged);
    connect(m_addFieldBtn, &QPushButton::clicked,
            this, &DataMaskEditor::onAddField);
    connect(m_removeFieldBtn, &QPushButton::clicked,
            this, &DataMaskEditor::onRemoveField);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &DataMaskEditor::onExport);
    connect(m_importBtn, &QPushButton::clicked,
            this, &DataMaskEditor::onImport);
    connect(m_fieldTable, &QTableWidget::cellChanged,
            this, &DataMaskEditor::onFieldCellChanged);

    // 初始刷新
    refreshValueDisplay();
}

// ──────────────────────────────────────────────
// 位按钮网格管理
// ──────────────────────────────────────────────

/**
 * @brief 重建位按钮网格，根据当前位宽度按高位→低位从左到右排列
 *
 * 每行显示 8 个 bit，行从高位行开始向下排列。
 * objectName 格式: dmeBitBtn_0 ~ dmeBitBtn_63
 */
void DataMaskEditor::rebuildBitButtons()
{
    // 清除旧按钮
    for (auto *btn : m_bitButtons) {
        btn->deleteLater();
    }
    m_bitButtons.clear();

    // 找到 bitGridLayout（第二个 layout，index 1）
    auto *gridLayout = qobject_cast<QGridLayout *>(layout()->itemAt(1)->layout());
    if (!gridLayout) return;

    m_bits.resize(m_bitWidth);
    m_bits.fill(false);

    const int rows = (m_bitWidth + 7) / 8;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < 8; ++col) {
            const int bitPos = (rows - 1 - row) * 8 + (7 - col);
            if (bitPos >= m_bitWidth) {
                // 空白占位
                auto *spacer = new QWidget(this);
                spacer->setFixedWidth(32);
                spacer->setFixedHeight(28);
                gridLayout->addWidget(spacer, row, col);
                continue;
            }

            auto *btn = new QPushButton(QString::number(bitPos), this);
            btn->setObjectName(QStringLiteral("dmeBitBtn_%1").arg(bitPos));
            btn->setFixedSize(32, 28);
            btn->setCheckable(true);
            btn->setToolTip(tr("位 %1").arg(bitPos));
            btn->setProperty("bitPos", bitPos);

            connect(btn, &QPushButton::clicked, this, &DataMaskEditor::onBitClicked);
            gridLayout->addWidget(btn, row, col);
            m_bitButtons.append(btn);
        }
    }
}

/**
 * @brief 刷新所有位按钮的选中状态，匹配 m_maskValue
 */
void DataMaskEditor::refreshBitDisplay()
{
    for (int i = 0; i < m_bitButtons.size(); ++i) {
        const int bitPos = m_bitButtons[i]->property("bitPos").toInt();
        const bool on = (m_maskValue >> bitPos) & 1;
        m_bitButtons[i]->setChecked(on);
        m_bits[bitPos] = on;
    }
    colorBitButtons();
}

/**
 * @brief 刷新 hex/binary/decimal 显示
 */
void DataMaskEditor::refreshValueDisplay()
{
    const int hexDigits = m_bitWidth / 4;
    m_hexValueEdit->setText(QStringLiteral("0x%1")
        .arg(m_maskValue, hexDigits, 16, QLatin1Char('0')).toUpper());

    QString binStr;
    binStr.reserve(m_bitWidth);
    for (int i = m_bitWidth - 1; i >= 0; --i) {
        binStr += ((m_maskValue >> i) & 1) ? '1' : '0';
    }
    m_binValueEdit->setText(binStr);

    m_decValueEdit->setText(QString::number(m_maskValue));
}

// ──────────────────────────────────────────────
// 公共 API
// ──────────────────────────────────────────────

/** @brief 设置位宽度，重建位按钮网格 */
void DataMaskEditor::setBitWidth(int bits)
{
    if (bits == m_bitWidth) return;
    m_bitWidth = qBound(8, bits, 64);

    if (m_bitWidth < 64) {
        const uint64_t mask = (1ULL << m_bitWidth) - 1;
        m_maskValue &= mask;
    }

    m_bits.resize(m_bitWidth);
    m_bits.fill(false);

    rebuildBitButtons();
    refreshBitDisplay();
    refreshValueDisplay();
}

/** @brief 获取当前位宽度 */
int DataMaskEditor::bitWidth() const { return m_bitWidth; }

/** @brief 设置掩码值并刷新所有显示 */
void DataMaskEditor::setMaskValue(uint64_t value)
{
    if (value == m_maskValue) return;
    m_maskValue = value;
    ++m_stats.totalMaskChanges;
    refreshBitDisplay();
    refreshValueDisplay();
    refreshFieldTable();
    emit maskChanged(m_maskValue);
}

/** @brief 获取当前掩码值 */
uint64_t DataMaskEditor::maskValue() const { return m_maskValue; }

// ──────────────────────────────────────────────
// 字段管理
// ──────────────────────────────────────────────

/** @brief 添加位域定义 */
void DataMaskEditor::addField(const BitField &field)
{
    m_fields.append(field);
    ++m_stats.totalFieldAdds;
    m_stats.activeFields = m_fields.size();
    if (m_stats.activeFields > m_stats.peakFields) {
        m_stats.peakFields = m_stats.activeFields;
    }
    colorBitButtons();
    refreshFieldTable();
    emit fieldAdded(m_fields.size() - 1);
}

/** @brief 移除指定位域 */
void DataMaskEditor::removeField(int index)
{
    if (index < 0 || index >= m_fields.size()) return;
    m_fields.removeAt(index);
    ++m_stats.totalFieldRemoves;
    m_stats.activeFields = m_fields.size();
    colorBitButtons();
    refreshFieldTable();
    emit fieldRemoved(index);
}

/** @brief 更新指定位域 */
void DataMaskEditor::updateField(int index, const BitField &field)
{
    if (index < 0 || index >= m_fields.size()) return;
    m_fields[index] = field;
    ++m_stats.totalFieldEdits;
    colorBitButtons();
    refreshFieldTable();
    emit fieldUpdated(index);
}

/** @brief 获取所有位域定义 */
QList<DataMaskEditor::BitField> DataMaskEditor::fields() const { return m_fields; }

/**
 * @brief 从给定数据中提取指定位域的值
 * @param index 位域索引
 * @param data 输入数据
 * @return 提取出的值(右对齐)
 */
uint64_t DataMaskEditor::extractField(int index, uint64_t data) const
{
    if (index < 0 || index >= m_fields.size()) return 0;
    const auto &f = m_fields[index];
    const int width = f.endBit - f.startBit + 1;
    const uint64_t fieldMask = (width >= 64) ? ~0ULL : (1ULL << width) - 1;
    return (data >> f.startBit) & fieldMask;
}

// ──────────────────────────────────────────────
// 字段表格
// ──────────────────────────────────────────────

/** @brief 刷新字段表格，显示所有位域定义及其从当前掩码值提取的值 */
void DataMaskEditor::refreshFieldTable()
{
    disconnect(m_fieldTable, &QTableWidget::cellChanged,
               this, &DataMaskEditor::onFieldCellChanged);

    m_fieldTable->setRowCount(m_fields.size());
    for (int i = 0; i < m_fields.size(); ++i) {
        const auto &f = m_fields[i];

        auto *nameItem = new QTableWidgetItem(f.name);
        nameItem->setTextAlignment(Qt::AlignCenter);
        m_fieldTable->setItem(i, 0, nameItem);

        auto *startItem = new QTableWidgetItem(QString::number(f.startBit));
        startItem->setTextAlignment(Qt::AlignCenter);
        m_fieldTable->setItem(i, 1, startItem);

        auto *endItem = new QTableWidgetItem(QString::number(f.endBit));
        endItem->setTextAlignment(Qt::AlignCenter);
        m_fieldTable->setItem(i, 2, endItem);

        auto *colorItem = new QTableWidgetItem(f.color.name());
        colorItem->setBackground(f.color);
        colorItem->setTextAlignment(Qt::AlignCenter);
        m_fieldTable->setItem(i, 3, colorItem);

        const uint64_t val = extractField(i, m_maskValue);
        m_fields[i].value = val;
        auto *valItem = new QTableWidgetItem(
            QStringLiteral("0x%1").arg(val, 0, 16).toUpper());
        valItem->setTextAlignment(Qt::AlignCenter);
        valItem->setFlags(valItem->flags() & ~Qt::ItemIsEditable);
        m_fieldTable->setItem(i, 4, valItem);
    }

    connect(m_fieldTable, &QTableWidget::cellChanged,
            this, &DataMaskEditor::onFieldCellChanged);
}

/** @brief 根据字段定义给位按钮着色(更新工具提示) */
void DataMaskEditor::colorBitButtons()
{
    for (auto *btn : m_bitButtons) {
        const int bitPos = btn->property("bitPos").toInt();
        btn->setToolTip(tr("位 %1").arg(bitPos));
    }

    for (const auto &f : m_fields) {
        for (int b = f.startBit; b <= f.endBit; ++b) {
            for (auto *btn : m_bitButtons) {
                if (btn->property("bitPos").toInt() == b) {
                    btn->setToolTip(tr("位 %1 [%2]").arg(b).arg(f.name));
                    break;
                }
            }
        }
    }
}

// ──────────────────────────────────────────────
// 槽函数
// ──────────────────────────────────────────────

/** @brief 位按钮点击：切换该位并更新掩码值 */
void DataMaskEditor::onBitClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;

    const int bitPos = btn->property("bitPos").toInt();
    const bool on = btn->isChecked();

    if (on) {
        m_maskValue |= (1ULL << bitPos);
    } else {
        m_maskValue &= ~(1ULL << bitPos);
    }

    m_bits[bitPos] = on;
    ++m_stats.totalMaskChanges;

    refreshValueDisplay();
    refreshFieldTable();
    emit bitToggled(bitPos, on);
    emit maskChanged(m_maskValue);
}

/** @brief 位宽切换：重新构建位按钮网格 */
void DataMaskEditor::onBitWidthChanged()
{
    setBitWidth(m_bitWidthCombo->currentData().toInt());
}

/** @brief 添加字段：创建默认字段定义 */
void DataMaskEditor::onAddField()
{
    BitField field;
    field.startBit = 0;
    field.endBit = qMin(m_bitWidth - 1, 7);
    field.name = tr("字段_%1").arg(m_fields.size() + 1);
    field.color = QColor::fromHsv((m_fields.size() * 47) % 360, 180, 220);
    addField(field);
}

/** @brief 删除字段：删除表格中当前选中的行 */
void DataMaskEditor::onRemoveField()
{
    const int row = m_fieldTable->currentRow();
    if (row >= 0) removeField(row);
}

/** @brief 导出字段定义到 JSON 文件 */
void DataMaskEditor::onExport()
{
    const QString path = QFileDialog::getSaveFileName(
        this, tr("导出位域定义"), QString(),
        tr("JSON 文件 (*.json);;所有文件 (*)"));
    if (path.isEmpty()) return;
    if (exportToJson(path)) ++m_stats.totalExports;
}

/** @brief 从 JSON 文件导入字段定义 */
void DataMaskEditor::onImport()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("导入位域定义"), QString(),
        tr("JSON 文件 (*.json);;所有文件 (*)"));
    if (!path.isEmpty()) importFromJson(path);
}

/** @brief 字段表格单元格编辑回调，同步到内部数据 */
void DataMaskEditor::onFieldCellChanged(int row, int col)
{
    if (row < 0 || row >= m_fields.size()) return;
    auto *item = m_fieldTable->item(row, col);
    if (!item) return;

    auto &field = m_fields[row];
    switch (col) {
    case 0: field.name = item->text(); break;
    case 1: field.startBit = qBound(0, item->text().toInt(), m_bitWidth - 1); break;
    case 2: field.endBit = qBound(0, item->text().toInt(), m_bitWidth - 1); break;
    case 3: field.color = QColor(item->text()); break;
    default: return;
    }

    ++m_stats.totalFieldEdits;
    colorBitButtons();
    refreshFieldTable();
    emit fieldUpdated(row);
}
