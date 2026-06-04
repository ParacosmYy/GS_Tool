/**
 * @file DataInspectorWidget.cpp
 * @brief 字节级数据检查器面板实现 — 构造、UI、数据刷新、多字节解释
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 统计接口见 @see DataInspectorWidgetStats.cpp
 */

#include "utils/data_inspector/DataInspectorWidget.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// 构造函数
// ---------------------------------------------------------------------------

/**
 * @brief 构造函数，初始化 UI 布局
 */
DataInspectorWidget::DataInspectorWidget(QWidget *parent)
    : QWidget(parent)
    , m_byteTable(new QTableWidget(this))
    , m_multiByteTable(new QTableWidget(this))
    , m_byteOrderCombo(new QComboBox(this))
    , m_signedCheck(new QCheckBox(tr("有符号"), this))
    , m_summaryLabel(new QLabel(tr("就绪 — 请设置数据"), this))
{
    setupUI();
}

// ---------------------------------------------------------------------------
// UI 构建
// ---------------------------------------------------------------------------

/**
 * @brief 构建完整 UI 布局：工具栏 + 逐字节表 + 多字节表 + 摘要
 */
void DataInspectorWidget::setupUI()
{
    setObjectName(QStringLiteral("DataInspectorWidget"));
    auto *mainLayout = new QVBoxLayout(this);

    // ---- 工具栏：字节序 + 有符号开关 + 复制按钮 ----
    auto *toolbar = new QHBoxLayout();

    auto *orderLabel = new QLabel(tr("字节序："), this);
    orderLabel->setObjectName("inspectorOrderLabel");
    toolbar->addWidget(orderLabel);

    m_byteOrderCombo->setObjectName("inspectorByteOrderCombo");
    m_byteOrderCombo->addItem(tr("小端 (Little-Endian)"),
                              static_cast<int>(LittleEndian));
    m_byteOrderCombo->addItem(tr("大端 (Big-Endian)"),
                              static_cast<int>(BigEndian));
    toolbar->addWidget(m_byteOrderCombo);

    m_signedCheck->setObjectName("inspectorSignedCheck");
    toolbar->addWidget(m_signedCheck);

    toolbar->addStretch();

    auto *copyBtn = new QPushButton(tr("复制选中"), this);
    copyBtn->setObjectName("inspectorCopyBtn");
    toolbar->addWidget(copyBtn);

    mainLayout->addLayout(toolbar);

    // ---- 逐字节明细表 ----
    m_byteTable->setObjectName("inspectorByteTable");
    m_byteTable->setColumnCount(ColCount);
    m_byteTable->setHorizontalHeaderLabels(
        {tr("偏移"), tr("Hex"), tr("Dec"), tr("Oct"), tr("Bin"), tr("ASCII")});
    m_byteTable->horizontalHeader()->setStretchLastSection(true);
    m_byteTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_byteTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_byteTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(m_byteTable, /*stretch=*/3);

    // ---- 多字节解释表 ----
    m_multiByteTable->setObjectName("inspectorMultiByteTable");
    m_multiByteTable->setColumnCount(2);
    m_multiByteTable->setHorizontalHeaderLabels({tr("类型"), tr("值")});
    m_multiByteTable->horizontalHeader()->setStretchLastSection(true);
    m_multiByteTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_multiByteTable->verticalHeader()->setVisible(false);
    m_multiByteTable->setMaximumHeight(160);
    mainLayout->addWidget(m_multiByteTable, /*stretch=*/1);

    // ---- 摘要标签 ----
    m_summaryLabel->setObjectName("inspectorSummaryLabel");
    mainLayout->addWidget(m_summaryLabel);

    // ---- 信号连接 ----
    connect(m_byteOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataInspectorWidget::onByteOrderChanged);
    connect(m_signedCheck, &QCheckBox::toggled,
            this, [this]() { ++m_stats.formatChanges; updateMultiByteView(); });
    connect(m_byteTable, &QTableWidget::cellClicked,
            this, &DataInspectorWidget::onCellClicked);
    connect(copyBtn, &QPushButton::clicked,
            this, &DataInspectorWidget::onCopySelected);
}

// ---------------------------------------------------------------------------
// 公开接口
// ---------------------------------------------------------------------------

/**
 * @brief 设置待检查数据并刷新表格
 * @param data 原始字节数组
 */
void DataInspectorWidget::setData(const QByteArray &data)
{
    m_data = data;
    refreshTable();
    updateMultiByteView();

    ++m_stats.totalInspections;
    m_stats.totalBytesInspected += static_cast<quint64>(data.size());
    const auto sz = static_cast<quint64>(data.size());
    if (sz > m_stats.peakBytesPerInspect) {
        m_stats.peakBytesPerInspect = sz;
    }
    m_summaryLabel->setText(
        tr("%1 字节 | 偏移 0x%2 | 检查 #%3")
            .arg(data.size())
            .arg(0, 4, 16, QChar('0'))
            .arg(m_stats.totalInspections));
    emit dataInspected(data.size());
}

/**
 * @brief 获取当前数据
 * @return 字节数组副本
 */
QByteArray DataInspectorWidget::data() const
{
    return m_data;
}

/**
 * @brief 设置字节序
 * @param order LittleEndian 或 BigEndian
 */
void DataInspectorWidget::setByteOrder(ByteOrder order)
{
    m_byteOrder = order;
    m_byteOrderCombo->setCurrentIndex(static_cast<int>(order));
}

/**
 * @brief 获取当前字节序
 * @return 字节序枚举
 */
DataInspectorWidget::ByteOrder DataInspectorWidget::byteOrder() const
{
    return m_byteOrder;
}

// ---------------------------------------------------------------------------
// 表格刷新
// ---------------------------------------------------------------------------

/**
 * @brief 刷新逐字节明细表，为每个字节填写六列数据
 */
void DataInspectorWidget::refreshTable()
{
    m_byteTable->setRowCount(m_data.size());
    for (int i = 0; i < m_data.size(); ++i) {
        const auto byte = static_cast<unsigned char>(m_data[i]);

        auto *itemOffset = new QTableWidgetItem(
            QStringLiteral("0x%1").arg(i, 4, 16, QChar('0')));
        itemOffset->setTextAlignment(Qt::AlignCenter);
        m_byteTable->setItem(i, ColOffset, itemOffset);

        auto *itemHex = new QTableWidgetItem(
            QStringLiteral("%1").arg(byte, 2, 16, QChar('0')).toUpper());
        itemHex->setTextAlignment(Qt::AlignCenter);
        m_byteTable->setItem(i, ColHex, itemHex);

        auto *itemDec = new QTableWidgetItem(QString::number(byte));
        itemDec->setTextAlignment(Qt::AlignCenter);
        m_byteTable->setItem(i, ColDecimal, itemDec);

        auto *itemOct = new QTableWidgetItem(
            QStringLiteral("%1").arg(byte, 3, 8, QChar('0')));
        itemOct->setTextAlignment(Qt::AlignCenter);
        m_byteTable->setItem(i, ColOctal, itemOct);

        auto *itemBin = new QTableWidgetItem(byteToBinary(byte));
        itemBin->setTextAlignment(Qt::AlignCenter);
        m_byteTable->setItem(i, ColBinary, itemBin);

        auto *itemAscii = new QTableWidgetItem(QChar(byteToAscii(byte)));
        itemAscii->setTextAlignment(Qt::AlignCenter);
        m_byteTable->setItem(i, ColAscii, itemAscii);
    }
    m_byteTable->resizeColumnsToContents();
}

/**
 * @brief 根据当前选中行，从该偏移开始解析 int16/uint16/int32/uint32/float
 */
void DataInspectorWidget::updateMultiByteView()
{
    const int row = m_byteTable->currentRow();
    if (row < 0 || row >= m_data.size()) {
        m_multiByteTable->setRowCount(0);
        return;
    }
    const int pos = row;
    const int remain = m_data.size() - pos;
    const bool le = (m_byteOrder == LittleEndian);
    const bool sgn = m_signedCheck->isChecked();

    m_multiByteTable->setRowCount(5);
    const auto types = {
        std::make_pair(tr("int16_t"), 2),
        std::make_pair(tr("uint16_t"), 2),
        std::make_pair(tr("int32_t"), 4),
        std::make_pair(tr("uint32_t"), 4),
        std::make_pair(tr("float"),   4),
    };

    int r = 0;
    for (const auto &[name, need] : types) {
        auto *typeItem = new QTableWidgetItem(name);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        m_multiByteTable->setItem(r, 0, typeItem);

        QString val;
        if (remain >= need) {
            QByteArray slice = m_data.mid(pos, need);
            if (le) {
                // QByteArray 本身就是字节序无关的，小端无需翻转
            } else {
                std::reverse(slice.begin(), slice.end());
            }
            if (need == 2) {
                val = sgn
                    ? QString::number(qFromLittleEndian<qint16>(slice.constData()))
                    : QString::number(qFromLittleEndian<quint16>(slice.constData()));
            } else if (name == tr("float")) {
                const auto bits = qFromLittleEndian<quint32>(slice.constData());
                float f;
                std::memcpy(&f, &bits, sizeof(f));
                val = QString::number(f, 'g', 6);
            } else {
                val = sgn
                    ? QString::number(qFromLittleEndian<qint32>(slice.constData()))
                    : QString::number(qFromLittleEndian<quint32>(slice.constData()));
            }
        } else {
            val = QStringLiteral("-");
        }
        auto *valItem = new QTableWidgetItem(val);
        valItem->setTextAlignment(Qt::AlignCenter);
        m_multiByteTable->setItem(r, 1, valItem);
        ++r;
    }
    m_multiByteTable->resizeColumnsToContents();
}

// ---------------------------------------------------------------------------
// 工具函数
// ---------------------------------------------------------------------------

/**
 * @brief 判断字节是否可显示 ASCII，不可显示则返回 '.'
 */
char DataInspectorWidget::byteToAscii(unsigned char byte)
{
    return (byte >= 0x20 && byte <= 0x7E) ? static_cast<char>(byte) : '.';
}

/**
 * @brief 将单字节转为 8 位二进制字符串（前导零补齐）
 */
QString DataInspectorWidget::byteToBinary(unsigned char byte)
{
    return QStringLiteral("%1%2%3%4%5%6%7%8")
        .arg((byte >> 7) & 1).arg((byte >> 6) & 1)
        .arg((byte >> 5) & 1).arg((byte >> 4) & 1)
        .arg((byte >> 3) & 1).arg((byte >> 2) & 1)
        .arg((byte >> 1) & 1).arg(byte & 1);
}

// ---------------------------------------------------------------------------
// 槽函数
// ---------------------------------------------------------------------------

/**
 * @brief 字节序下拉框变更处理
 */
void DataInspectorWidget::onByteOrderChanged(int index)
{
    m_byteOrder = static_cast<ByteOrder>(index);
    ++m_stats.formatChanges;
    updateMultiByteView();
}

/**
 * @brief 字节表单元格点击 — 刷新多字节视图
 */
void DataInspectorWidget::onCellClicked(int row, int /*column*/)
{
    m_summaryLabel->setText(
        tr("%1 字节 | 偏移 0x%2 | 检查 #%3")
            .arg(m_data.size())
            .arg(row, 4, 16, QChar('0'))
            .arg(m_stats.totalInspections));
    updateMultiByteView();
}

/**
 * @brief 复制选中行的所有单元格内容到剪贴板
 */
void DataInspectorWidget::onCopySelected()
{
    const int row = m_byteTable->currentRow();
    if (row < 0) return;

    QStringList parts;
    for (int c = 0; c < ColCount; ++c) {
        if (auto *item = m_byteTable->item(row, c)) {
            parts << item->text();
        }
    }
    const QString text = parts.join(QStringLiteral("\t"));
    QApplication::clipboard()->setText(text);
    ++m_stats.totalCopies;
    emit dataCopied(text);
}
