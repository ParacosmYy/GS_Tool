/**
 * @file PacketBuilderPanel.cpp
 * @brief 数据包构建面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/packet/PacketBuilderPanel.h"

#include <QHeaderView>

/**
 * @brief 构造函数，初始化面板布局
 */
PacketBuilderPanel::PacketBuilderPanel(QWidget *parent)
    : QWidget(parent)
    , m_fieldTable(new QTableWidget(this))
    , m_addFieldBtn(new QPushButton(tr("添加字段"), this))
    , m_removeFieldBtn(new QPushButton(tr("删除字段"), this))
    , m_buildBtn(new QPushButton(tr("构建数据包"), this))
    , m_hexPreview(new QTextEdit(this))
{
    setObjectName(QStringLiteral("PacketBuilderPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 按钮行
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_addFieldBtn);
    btnLayout->addWidget(m_removeFieldBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_buildBtn);

    // 字段表格
    m_fieldTable->setColumnCount(5);
    m_fieldTable->setHorizontalHeaderLabels({
        tr("名称"), tr("偏移"), tr("大小"), tr("类型"), tr("值")
    });
    m_fieldTable->horizontalHeader()->setStretchLastSection(true);
    m_fieldTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 十六进制预览
    m_hexPreview->setReadOnly(true);
    m_hexPreview->setMaximumHeight(80);
    m_hexPreview->setPlaceholderText(tr("构建后的数据包将以十六进制显示..."));

    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(m_fieldTable);
    mainLayout->addWidget(m_hexPreview);

    // 连接信号
    connect(m_addFieldBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onAddField);
    connect(m_removeFieldBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onRemoveField);
    connect(m_buildBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onBuild);
}

/**
 * @brief 设置关联的构建器实例
 */
void PacketBuilderPanel::setBuilder(PacketBuilder *builder)
{
    m_builder = builder;
    refreshTable();
}

/**
 * @brief 添加新字段行
 */
void PacketBuilderPanel::onAddField()
{
    if (!m_builder) {
        return;
    }

    PacketField field;
    field.name = tr("字段%1").arg(m_builder->fields().size() + 1);
    field.offset = 0;
    field.size = 1;
    field.dataType = QStringLiteral("uint8");
    field.value = 0;

    m_builder->addField(field);
    refreshTable();
}

/**
 * @brief 删除选中行对应的字段
 */
void PacketBuilderPanel::onRemoveField()
{
    if (!m_builder) {
        return;
    }

    int row = m_fieldTable->currentRow();
    if (row >= 0) {
        m_builder->removeField(row);
        refreshTable();
    }
}

/**
 * @brief 构建数据包并更新预览
 */
void PacketBuilderPanel::onBuild()
{
    if (!m_builder) {
        return;
    }

    QByteArray packet = m_builder->buildPacket();
    m_hexPreview->setPlainText(QString::fromUtf8(packet.toHex(' ').toUpper()));
}

/**
 * @brief 刷新表格以同步构建器字段
 */
void PacketBuilderPanel::refreshTable()
{
    if (!m_builder) {
        return;
    }

    auto fields = m_builder->fields();
    m_fieldTable->setRowCount(fields.size());

    for (int i = 0; i < fields.size(); ++i) {
        const auto &f = fields[i];
        m_fieldTable->setItem(i, 0, new QTableWidgetItem(f.name));
        m_fieldTable->setItem(i, 1, new QTableWidgetItem(QString::number(f.offset)));
        m_fieldTable->setItem(i, 2, new QTableWidgetItem(QString::number(f.size)));
        m_fieldTable->setItem(i, 3, new QTableWidgetItem(f.dataType));
        m_fieldTable->setItem(i, 4, new QTableWidgetItem(f.value.toString()));
    }
}
