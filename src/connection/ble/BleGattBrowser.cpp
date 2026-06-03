/**
 * @file BleGattBrowser.cpp
 * @brief GATT服务浏览器实现
 *
 * 完整UI: 顶部连接状态+刷新，左侧GATT树(服务→特征→描述符)，
 * 右侧hex dump显示，底部读写操作栏(读取/写入/通知订阅/格式选择)。
 */

#include "connection/ble/BleGattBrowser.h"
#include "connection/ble/BleConnection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QTreeWidgetItem>
#include <QRandomGenerator>

/// 模拟服务名称映射(短UUID -> 可翻译名称)
static const QMap<QString, const char*> SVC_KEYS = {
    {"00001800", QT_TRANSLATE_NOOP("BleGattBrowser", "通用访问")},
    {"00001801", QT_TRANSLATE_NOOP("BleGattBrowser", "通用属性")},
    {"0000180a", QT_TRANSLATE_NOOP("BleGattBrowser", "设备信息")},
    {"0000ffe0", QT_TRANSLATE_NOOP("BleGattBrowser", "自定义服务")},
    {"6e400001", QT_TRANSLATE_NOOP("BleGattBrowser", "Nordic UART")}
};

/// 特征属性表: {显示名, 属性标志}
static const struct { const char* name; BleGattBrowser::CharProperties prop; }
PROP_TABLE[] = {
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取"),          BleGattBrowser::PropRead},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取|写入"),     BleGattBrowser::PropRead | BleGattBrowser::PropWrite},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取|通知"),     BleGattBrowser::PropRead | BleGattBrowser::PropNotify},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "写入|通知"),     BleGattBrowser::PropWrite | BleGattBrowser::PropNotify},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取|写入|通知"),BleGattBrowser::PropRead | BleGattBrowser::PropWrite | BleGattBrowser::PropNotify},
};

/** @brief 构造GATT浏览器UI @param parent 父控件 */
BleGattBrowser::BleGattBrowser(QWidget* parent)
    : QWidget(parent)
    , m_serviceTree(new QTreeWidget(this))
    , m_valueDisplay(new QTextEdit(this))
    , m_selectedLabel(new QLabel(tr("未选择特征"), this))
    , m_writeInput(new QLineEdit(this))
    , m_readBtn(new QPushButton(tr("读取"), this))
    , m_writeBtn(new QPushButton(tr("写入"), this))
    , m_refreshBtn(new QPushButton(tr("刷新服务"), this))
    , m_notifyCheck(new QCheckBox(tr("订阅通知"), this))
    , m_writeFormatCombo(new QComboBox(this))
    , m_connectionStatus(new QLabel(tr("未连接"), this))
    , m_serviceCountLabel(new QLabel(tr("服务: 0"), this))
    , m_notificationTimer(new QTimer(this))
{
    setObjectName("BleGattBrowser");
    m_serviceTree->setObjectName("gattServiceTree");
    m_valueDisplay->setObjectName("gattValueDisplay");
    m_selectedLabel->setObjectName("gattSelectedLabel");
    m_writeInput->setObjectName("gattWriteInput");
    m_readBtn->setObjectName("btnGattRead");
    m_writeBtn->setObjectName("btnGattWrite");
    m_refreshBtn->setObjectName("btnGattRefresh");
    m_notifyCheck->setObjectName("chkGattNotify");
    m_writeFormatCombo->setObjectName("cmbWriteFormat");
    m_connectionStatus->setObjectName("lblConnectionStatus");
    m_serviceCountLabel->setObjectName("lblServiceCount");

    /* 树控件: 三列展示 */
    m_serviceTree->setHeaderLabels({tr("名称"), tr("UUID"), tr("属性")});
    m_serviceTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_serviceTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_serviceTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_serviceTree->setAlternatingRowColors(true);

    /* 值显示区: 等宽字体 */
    m_valueDisplay->setReadOnly(true);
    m_valueDisplay->setPlaceholderText(tr("选择特征后点击读取查看值"));
    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_valueDisplay->setFont(monoFont);

    m_writeInput->setPlaceholderText(tr("输入写入值(十六进制，如 0A1B2C)"));
    m_writeFormatCombo->addItems({tr("HEX"), tr("ASCII"), tr("UTF-8")});
    m_readBtn->setEnabled(false);
    m_writeBtn->setEnabled(false);
    m_notifyCheck->setEnabled(false);
    m_notificationTimer->setInterval(2000);

    /* 顶部状态栏 */
    auto* topBar = new QHBoxLayout();
    m_connectionStatus->setStyleSheet("font-weight: bold;");
    topBar->addWidget(m_connectionStatus);
    topBar->addStretch();
    topBar->addWidget(m_serviceCountLabel);
    topBar->addWidget(m_refreshBtn);

    /* 底部操作栏 */
    auto* opLayout = new QHBoxLayout();
    opLayout->addWidget(m_selectedLabel, 1);
    opLayout->addWidget(m_notifyCheck);
    opLayout->addWidget(m_readBtn);
    opLayout->addWidget(m_writeFormatCombo);
    opLayout->addWidget(m_writeInput, 1);
    opLayout->addWidget(m_writeBtn);

    /* 右侧面板 */
    auto* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_valueDisplay, 1);
    rightLayout->addLayout(opLayout);

    /* 主布局 */
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topBar);
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName("gattSplitter");
    splitter->addWidget(m_serviceTree);
    auto* rightWidget = new QWidget(this);
    rightWidget->setLayout(rightLayout);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    mainLayout->addWidget(splitter, 1);

    /* 信号连接 */
    connect(m_serviceTree, &QTreeWidget::currentItemChanged, this, &BleGattBrowser::onTreeItemChanged);
    connect(m_readBtn, &QPushButton::clicked, this, &BleGattBrowser::onReadClicked);
    connect(m_writeBtn, &QPushButton::clicked, this, &BleGattBrowser::onWriteClicked);
    connect(m_notifyCheck, &QCheckBox::toggled, this, &BleGattBrowser::onNotifyToggled);
    connect(m_refreshBtn, &QPushButton::clicked, this, &BleGattBrowser::onRefreshClicked);
    connect(m_writeFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BleGattBrowser::onWriteFormatChanged);
    connect(m_notificationTimer, &QTimer::timeout, this, &BleGattBrowser::onSimulateNotification);
}

/** @brief 设置BLE连接实例 @param connection BleConnection对象 */
void BleGattBrowser::setConnection(BleConnection* connection)
{
    if (m_connection) disconnect(m_connection, nullptr, this, nullptr);
    m_connection = connection;
    if (!m_connection) { updateConnectionStatus(); return; }

    connect(m_connection, &BleConnection::stateChanged, this, &BleGattBrowser::onConnectionStateChanged);
    connect(m_connection, &BleConnection::servicesDiscovered, this, &BleGattBrowser::onServicesDiscovered);
    connect(m_connection, &BleConnection::characteristicRead, this,
        [this](const QString& uuid, const QByteArray& value) {
            m_totalBytesRead += static_cast<quint64>(value.size());
            emit characteristicReadComplete(uuid, value);
        });

    if (m_connection->state() == ConnectionState::Connected) {
        const QStringList svcs = m_connection->discoverServices();
        populateTree(svcs);
    }
    updateConnectionStatus();
}

/** @brief 树选中项变更，更新UUID/属性/按钮状态 */
void BleGattBrowser::onTreeItemChanged()
{
    QTreeWidgetItem* item = m_serviceTree->currentItem();
    if (!item) {
        m_selectedUuid.clear();
        m_selectedProps = CharProperties();
        m_selectedLabel->setText(tr("未选择特征"));
        updateButtonStates();
        return;
    }
    m_selectedUuid = item->data(1, Qt::DisplayRole).toString();
    m_selectedProps = getItemProperties(item);
    m_selectedLabel->setText(tr("已选择: %1 [%2]")
        .arg(item->data(0, Qt::DisplayRole).toString(),
             item->data(2, Qt::DisplayRole).toString()));
    updateButtonStates();
}

/** @brief 读取按钮: 模拟读取选中特征的值 */
void BleGattBrowser::onReadClicked()
{
    if (m_selectedUuid.isEmpty()) { m_valueDisplay->append(tr("[错误] 请先选择一个特征")); return; }
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        m_valueDisplay->append(tr("[错误] BLE未连接")); return;
    }

    const int dataSize = 4 + QRandomGenerator::global()->bounded(13);
    QByteArray mockData(dataSize, 0);
    for (int i = 0; i < mockData.size(); ++i)
        mockData[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));

    m_valueDisplay->setText(formatHexDump(mockData));
    m_valueDisplay->append(tr("\n[读取完成] %1 (%2 bytes)").arg(m_selectedUuid, QString::number(mockData.size())));
    ++m_totalCharacteristicReads;
    m_totalBytesRead += static_cast<quint64>(mockData.size());
    emit characteristicReadComplete(m_selectedUuid, mockData);
}

/** @brief 写入按钮: 将输入值写入选中特征 */
void BleGattBrowser::onWriteClicked()
{
    const QString inputStr = m_writeInput->text().trimmed();
    if (inputStr.isEmpty()) { m_valueDisplay->append(tr("[错误] 请输入写入值")); return; }
    if (m_selectedUuid.isEmpty()) { m_valueDisplay->append(tr("[错误] 请先选择一个特征")); return; }

    QByteArray writeData;
    switch (m_writeFormatCombo->currentIndex()) {
    case 0: writeData = parseHexInput(inputStr);
        if (writeData.isEmpty()) { m_valueDisplay->append(tr("[错误] 无效的十六进制输入")); return; }
        break;
    case 1: writeData = inputStr.toLatin1(); break;
    default: writeData = inputStr.toUtf8(); break;
    }

    if (!m_connection) return;
    const qint64 written = m_connection->write(writeData);
    m_valueDisplay->append(tr("[写入] %1 → %2 (%3/%4 bytes)")
        .arg(QString::fromUtf8(writeData.toHex(' ')).toUpper(),
             m_selectedUuid, QString::number(written), QString::number(writeData.size())));
    ++m_totalCharacteristicWrites;
    m_totalBytesWritten += static_cast<quint64>(qMax<qint64>(0, written));
    emit characteristicWritten(m_selectedUuid, written);
}

/** @brief 通知订阅切换 @param checked 是否勾选 */
void BleGattBrowser::onNotifyToggled(bool checked)
{
    if (m_selectedUuid.isEmpty()) return;
    if (checked) {
        if (!m_subscribedUuids.contains(m_selectedUuid)) m_subscribedUuids.append(m_selectedUuid);
        m_valueDisplay->append(tr("[通知] 已订阅 %1").arg(m_selectedUuid));
        if (!m_notificationTimer->isActive()) m_notificationTimer->start();
    } else {
        m_subscribedUuids.removeAll(m_selectedUuid);
        m_valueDisplay->append(tr("[通知] 已取消订阅 %1").arg(m_selectedUuid));
        if (m_subscribedUuids.isEmpty()) m_notificationTimer->stop();
    }
}

void BleGattBrowser::onServicesDiscovered(const QStringList& services) { populateTree(services); }

/** @brief 刷新按钮: 重新发现GATT服务 */
void BleGattBrowser::onRefreshClicked()
{
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        m_valueDisplay->append(tr("[错误] BLE未连接，无法刷新服务")); return;
    }
    m_valueDisplay->append(tr("[刷新] 正在重新发现GATT服务..."));
    m_discoveryTimer.start();
    const QStringList svcs = m_connection->discoverServices();
    if (!svcs.isEmpty()) populateTree(svcs);
}

/** @brief 连接状态变化: 更新UI、自动发现或停止通知 */
void BleGattBrowser::onConnectionStateChanged()
{
    updateConnectionStatus();
    if (m_connection && m_connection->state() == ConnectionState::Connected) {
        m_discoveryTimer.start();
        m_connection->discoverServices();
    } else {
        m_notificationTimer->stop();
        m_subscribedUuids.clear();
        m_notifyCheck->setChecked(false);
    }
    emit connectionStatusChanged(m_connection && m_connection->state() == ConnectionState::Connected);
}

/** @brief 模拟通知数据到达 */
void BleGattBrowser::onSimulateNotification()
{
    if (m_subscribedUuids.isEmpty()) { m_notificationTimer->stop(); return; }
    const int idx = QRandomGenerator::global()->bounded(m_subscribedUuids.size());
    const QString uuid = m_subscribedUuids.at(idx);
    const int len = 2 + QRandomGenerator::global()->bounded(7);
    QByteArray data(len, 0);
    for (int i = 0; i < len; ++i) data[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    ++m_totalNotificationsReceived;
    m_totalBytesRead += static_cast<quint64>(len);
    m_valueDisplay->append(tr("[通知] %1: %2 (%3 bytes)")
        .arg(uuid, QString::fromUtf8(data.toHex(' ')).toUpper(), QString::number(len)));
    emit notificationReceived(uuid, data);
}

/** @brief 写入格式切换 @param index 格式索引 */
void BleGattBrowser::onWriteFormatChanged(int index)
{
    static const char* hints[] = {
        QT_TRANSLATE_NOOP("BleGattBrowser", "输入写入值(十六进制，如 0A1B2C)"),
        QT_TRANSLATE_NOOP("BleGattBrowser", "输入写入值(ASCII文本)"),
        QT_TRANSLATE_NOOP("BleGattBrowser", "输入写入值(UTF-8文本)")
    };
    m_writeInput->setPlaceholderText(tr(hints[qBound(0, index, 2)]));
}

/** @brief 填充GATT服务树(三级: Service→Characteristic→Descriptor) */
void BleGattBrowser::populateTree(const QStringList& services)
{
    m_serviceTree->clear();
    if (m_discoveryTimer.isValid()) {
        m_lastDiscoveryDurationMs = m_discoveryTimer.elapsed();
        m_discoveryTimer.invalidate();
    }
    ++m_totalServiceDiscoveries;

    for (const QString& svcUuid : services) {
        const QString shortUuid = svcUuid.left(8).toLower();
        auto* svcItem = new QTreeWidgetItem(m_serviceTree);
        svcItem->setText(0, tr(SVC_KEYS.value(shortUuid, QT_TRANSLATE_NOOP("BleGattBrowser", "未知服务"))));
        svcItem->setText(1, svcUuid);
        svcItem->setText(2, tr("主要服务"));
        svcItem->setExpanded(true);
        svcItem->setData(0, Qt::UserRole, "service");

        const int charCount = (shortUuid == "00001800") ? 3 : (shortUuid == "6e400001") ? 2 : 2;
        for (int i = 0; i < charCount; ++i) {
            auto* charItem = new QTreeWidgetItem(svcItem);
            const int propIdx = (shortUuid == "6e400001") ? (i == 0 ? 3 : 4) : i % 5;
            charItem->setText(0, tr("特征 %1").arg(i + 1));
            charItem->setText(1, svcUuid.left(8) + QString("-%1").arg(i, 4, 16, QChar('0')));
            charItem->setText(2, tr(PROP_TABLE[propIdx].name));
            charItem->setData(0, Qt::UserRole, "characteristic");
            charItem->setData(3, Qt::UserRole, static_cast<uint32_t>(PROP_TABLE[propIdx].prop));
            addDescriptors(charItem, i);
        }
    }
    m_serviceCountLabel->setText(tr("服务: %1").arg(services.size()));
}

/** @brief 添加描述符子节点 @param charItem 特征节点 @param charIndex 特征索引 */
void BleGattBrowser::addDescriptors(QTreeWidgetItem* charItem, int charIndex)
{
    auto* cccd = new QTreeWidgetItem(charItem);
    cccd->setText(0, tr("CCCD(通知/指示开关)"));
    cccd->setText(1, "00002902-0000-1000-8000-00805f9b34fb");
    cccd->setText(2, tr("读取|写入"));
    cccd->setData(0, Qt::UserRole, "descriptor");

    if (charIndex == 0) {
        auto* udesc = new QTreeWidgetItem(charItem);
        udesc->setText(0, tr("特征用户描述"));
        udesc->setText(1, "00002901-0000-1000-8000-00805f9b34fb");
        udesc->setText(2, tr("读取"));
        udesc->setData(0, Qt::UserRole, "descriptor");
    }
}

/** @brief 十六进制转储格式化(偏移+HEX+ASCII) */
QString BleGattBrowser::formatHexDump(const QByteArray& data) const
{
    QString r;
    for (int i = 0; i < data.size(); i += 16) {
        r += QString("%1  ").arg(i, 4, 16, QChar('0')).toUpper();
        for (int j = 0; j < 16; ++j) {
            r += (i + j < data.size())
                ? QString("%1 ").arg(static_cast<uint8_t>(data.at(i+j)), 2, 16, QChar('0')).toUpper()
                : "   ";
            if (j == 7) r += " ";
        }
        r += " |";
        for (int j = 0; j < 16 && i+j < data.size(); ++j) {
            const char c = data.at(i+j);
            r += (c >= 0x20 && c <= 0x7E) ? c : '.';
        }
        r += "|\n";
    }
    return r;
}

/** @brief 解析十六进制输入 */
QByteArray BleGattBrowser::parseHexInput(const QString& hexStr) const
{
    QString s = hexStr;
    s.remove(' ').remove("0x", Qt::CaseInsensitive);
    return QByteArray::fromHex(s.toUtf8());
}

BleGattBrowser::CharProperties BleGattBrowser::getItemProperties(QTreeWidgetItem* item) const
{
    if (!item) return CharProperties();
    return static_cast<CharProperties>(item->data(3, Qt::UserRole).toUInt());
}

/** @brief 根据选中特征属性更新按钮启用状态 */
void BleGattBrowser::updateButtonStates()
{
    const bool conn = m_connection && m_connection->state() == ConnectionState::Connected;
    const bool sel = !m_selectedUuid.isEmpty();
    m_readBtn->setEnabled(conn && sel && (m_selectedProps & PropRead));
    m_writeBtn->setEnabled(conn && sel && (m_selectedProps & (PropWrite | PropWriteNoRsp)));
    m_notifyCheck->setEnabled(conn && sel && (m_selectedProps & (PropNotify | PropIndicate)));
    if (m_subscribedUuids.contains(m_selectedUuid)) {
        m_notifyCheck->blockSignals(true);
        m_notifyCheck->setChecked(true);
        m_notifyCheck->blockSignals(false);
    }
}

/** @brief 更新连接状态标签 */
void BleGattBrowser::updateConnectionStatus()
{
    if (!m_connection) { m_connectionStatus->setText(tr("未连接")); return; }
    switch (m_connection->state()) {
    case ConnectionState::Connected:
        m_connectionStatus->setText(tr("已连接: %1")
            .arg(m_connection->deviceName().isEmpty() ? m_connection->name() : m_connection->deviceName()));
        break;
    case ConnectionState::Connecting:
        m_connectionStatus->setText(tr("连接中...")); break;
    default:
        m_connectionStatus->setText(tr("未连接")); break;
    }
}

// ---- 统计信息 ----
quint64 BleGattBrowser::totalServiceDiscoveries() const { return m_totalServiceDiscoveries; }
quint64 BleGattBrowser::totalCharacteristicReads() const { return m_totalCharacteristicReads; }
quint64 BleGattBrowser::totalCharacteristicWrites() const { return m_totalCharacteristicWrites; }
quint64 BleGattBrowser::totalNotificationsReceived() const { return m_totalNotificationsReceived; }
quint64 BleGattBrowser::totalDescriptorOps() const { return m_totalDescriptorOps; }
quint64 BleGattBrowser::totalBytesRead() const { return m_totalBytesRead; }
quint64 BleGattBrowser::totalBytesWritten() const { return m_totalBytesWritten; }
qint64 BleGattBrowser::lastDiscoveryDurationMs() const { return m_lastDiscoveryDurationMs; }

void BleGattBrowser::resetStatistics()
{
    m_totalServiceDiscoveries = m_totalCharacteristicReads = m_totalCharacteristicWrites = 0;
    m_totalNotificationsReceived = m_totalDescriptorOps = 0;
    m_totalBytesRead = m_totalBytesWritten = 0;
    m_lastDiscoveryDurationMs = 0;
}
