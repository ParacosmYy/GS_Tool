/**
 * @file BleGattBrowser.cpp
 * @brief GATT服务浏览器实现
 *
 * 完整UI: 左侧GATT树、右侧hex dump显示、底部读写操作栏。
 * 模拟模式下生成预设服务和特征。
 */

#include "connection/ble/BleGattBrowser.h"
#include "connection/ble/BleConnection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QTreeWidgetItem>

/// 模拟服务名称映射
static const QMap<QString, QString> SVC_NAMES = {
    {"00001800", "Generic Access"},
    {"00001801", "Generic Attribute"},
    {"0000180a", "Device Information"},
    {"0000ffe0", "Custom Service"},
    {"6e400001", "Nordic UART"}
};

BleGattBrowser::BleGattBrowser(QWidget* parent)
    : QWidget(parent)
    , m_serviceTree(new QTreeWidget(this))
    , m_valueDisplay(new QTextEdit(this))
    , m_selectedLabel(new QLabel(tr("未选择特征"), this))
    , m_writeInput(new QLineEdit(this))
    , m_readBtn(new QPushButton(tr("读取"), this))
    , m_writeBtn(new QPushButton(tr("写入"), this))
{
    setObjectName("BleGattBrowser");

    // 树控件配置
    m_serviceTree->setHeaderLabels({tr("名称"), tr("UUID"), tr("属性")});
    m_serviceTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_serviceTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_serviceTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    // 值显示区配置
    m_valueDisplay->setReadOnly(true);
    m_valueDisplay->setPlaceholderText(tr("选择特征后点击读取查看值"));
    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_valueDisplay->setFont(monoFont);

    // 写入输入框
    m_writeInput->setPlaceholderText(tr("输入写入值(十六进制，如 0A1B2C)"));

    // 控件命名（QSS依赖）
    m_readBtn->setObjectName("btnGattRead");
    m_writeBtn->setObjectName("btnGattWrite");
    m_writeInput->setObjectName("editGattWrite");

    // 操作栏布局
    auto opLayout = new QHBoxLayout();
    opLayout->addWidget(m_selectedLabel, 1);
    opLayout->addWidget(m_readBtn);
    opLayout->addWidget(m_writeInput, 1);
    opLayout->addWidget(m_writeBtn);

    // 右侧面板
    auto rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_valueDisplay, 1);
    rightLayout->addLayout(opLayout);

    // 主布局: 左树右面板
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_serviceTree, 2);
    mainLayout->addLayout(rightLayout, 3);

    // 信号连接
    connect(m_serviceTree, &QTreeWidget::currentItemChanged,
            this, &BleGattBrowser::onTreeItemChanged);
    connect(m_readBtn, &QPushButton::clicked,
            this, &BleGattBrowser::onReadClicked);
    connect(m_writeBtn, &QPushButton::clicked,
            this, &BleGattBrowser::onWriteClicked);
}

void BleGattBrowser::setConnection(BleConnection* connection)
{
    m_connection = connection;
    if (m_connection) {
        connect(m_connection, &BleConnection::servicesDiscovered,
                this, &BleGattBrowser::onServicesDiscovered);

        // 如果已连接，立即刷新服务列表
        if (m_connection->state() == ConnectionState::Connected) {
            const QStringList svcs = m_connection->discoverServices();
            populateTree(svcs);
        }
    }
}

void BleGattBrowser::onTreeItemChanged()
{
    QTreeWidgetItem* item = m_serviceTree->currentItem();
    if (!item) {
        m_selectedUuid.clear();
        m_selectedLabel->setText(tr("未选择特征"));
        return;
    }
    m_selectedUuid = item->data(1, Qt::DisplayRole).toString();
    const QString name = item->data(0, Qt::DisplayRole).toString();
    m_selectedLabel->setText(tr("已选择: %1").arg(name));
}

void BleGattBrowser::onReadClicked()
{
    if (m_selectedUuid.isEmpty()) {
        m_valueDisplay->append(tr("[错误] 请先选择一个特征"));
        return;
    }

    if (!m_connection ||
        m_connection->state() != ConnectionState::Connected) {
        m_valueDisplay->append(tr("[错误] BLE未连接"));
        return;
    }

    // 模拟读取: 生成随机十六进制数据
    QByteArray mockData(8, 0);
    for (int i = 0; i < mockData.size(); ++i) {
        mockData[i] = static_cast<char>(i * 0x11);
    }
    m_valueDisplay->setText(formatHexDump(mockData));
    m_valueDisplay->append(
        tr("\n[读取完成] %1 (%2 bytes)")
            .arg(m_selectedUuid, QString::number(mockData.size())));
}

void BleGattBrowser::onWriteClicked()
{
    const QString hexStr = m_writeInput->text().trimmed();
    if (hexStr.isEmpty()) {
        m_valueDisplay->append(tr("[错误] 请输入写入值"));
        return;
    }
    if (m_selectedUuid.isEmpty()) {
        m_valueDisplay->append(tr("[错误] 请先选择一个特征"));
        return;
    }

    // 解析十六进制输入
    QByteArray writeData = QByteArray::fromHex(hexStr.toUtf8());
    if (writeData.isEmpty()) {
        m_valueDisplay->append(tr("[错误] 无效的十六进制输入"));
        return;
    }

    if (m_connection) {
        const qint64 written = m_connection->write(writeData);
        m_valueDisplay->append(
            tr("[写入] %1 → %2 (%3/%4 bytes)")
                .arg(hexStr, m_selectedUuid,
                     QString::number(written),
                     QString::number(writeData.size())));
    }
}

void BleGattBrowser::onServicesDiscovered(const QStringList& services)
{
    populateTree(services);
}

void BleGattBrowser::populateTree(const QStringList& services)
{
    m_serviceTree->clear();

    for (const QString& svcUuid : services) {
        // 提取短UUID用于名称查找
        const QString shortUuid = svcUuid.left(8).toLower();
        const QString svcName = SVC_NAMES.value(
            shortUuid, tr("未知服务"));

        auto* svcItem = new QTreeWidgetItem(m_serviceTree);
        svcItem->setText(0, svcName);
        svcItem->setText(1, svcUuid);
        svcItem->setText(2, "PRIMARY");
        svcItem->setExpanded(true);

        // 每个服务下添加模拟特征
        const int charCount = (shortUuid == "00001800") ? 3 : 2;
        for (int i = 0; i < charCount; ++i) {
            auto* charItem = new QTreeWidgetItem(svcItem);
            charItem->setText(0, tr("特征 %1").arg(i + 1));
            charItem->setText(1, svcUuid.left(8)
                + QString("-%1").arg(i, 4, 16, QChar('0')));
            const QStringList props = {"READ", "READ|WRITE", "READ|NOTIFY"};
            charItem->setText(2, props.value(i, "READ"));
        }
    }
}

QString BleGattBrowser::formatHexDump(const QByteArray& data) const
{
    QString result;
    for (int i = 0; i < data.size(); i += 16) {
        // 偏移量
        result += QString("%1  ").arg(i, 4, 16, QChar('0')).toUpper();
        // 十六进制
        for (int j = 0; j < 16; ++j) {
            if (i + j < data.size()) {
                result += QString("%1 ").arg(
                    static_cast<uint8_t>(data.at(i + j)), 2, 16, QChar('0'))
                    .toUpper();
            } else {
                result += "   ";
            }
            if (j == 7) result += " ";
        }
        result += " |";
        // ASCII
        for (int j = 0; j < 16 && i + j < data.size(); ++j) {
            const char c = data.at(i + j);
            result += (c >= 0x20 && c <= 0x7E) ? c : '.';
        }
        result += "|\n";
    }
    return result;
}
