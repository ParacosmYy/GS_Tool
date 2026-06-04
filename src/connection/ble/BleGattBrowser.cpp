/**
 * @file BleGattBrowser.cpp
 * @brief GATT服务浏览器实现 — 构造、连接管理、读写操作与统计
 *
 * 完整UI: 顶部连接状态+刷新，左侧GATT树(服务→特征→描述符)，
 * 右侧hex dump显示，底部读写操作栏(读取/写入/通知订阅/格式选择)。
 *
 * 拆分文件:
 *   - BleGattBrowserServices.cpp: 服务发现与GATT树操作方法
 *   - BleGattBrowserDisplay.cpp:  显示格式化与UI状态更新方法
 */

#include "connection/ble/BleGattBrowser.h"
#include "connection/ble/BleConnection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QTreeWidgetItem>
#include <QRandomGenerator>

// 服务发现与GATT树操作方法见 BleGattBrowserServices.cpp

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
    rightWidget->setObjectName("gattRightPanel");
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


// 显示格式化与UI状态更新方法见 BleGattBrowserDisplay.cpp

// ---- 统计信息 ----
/** @brief 获取累计服务发现次数 @return 服务发现总数 */
quint64 BleGattBrowser::totalServiceDiscoveries() const { return m_totalServiceDiscoveries; }
/** @brief 获取累计特征值读取次数 @return 特征值读取总数 */
quint64 BleGattBrowser::totalCharacteristicReads() const { return m_totalCharacteristicReads; }
/** @brief 获取累计特征值写入次数 @return 特征值写入总数 */
quint64 BleGattBrowser::totalCharacteristicWrites() const { return m_totalCharacteristicWrites; }
/** @brief 获取累计通知接收次数 @return 通知接收总数 */
quint64 BleGattBrowser::totalNotificationsReceived() const { return m_totalNotificationsReceived; }
/** @brief 获取累计描述符操作次数 @return 描述符操作总数 */
quint64 BleGattBrowser::totalDescriptorOps() const { return m_totalDescriptorOps; }
/** @brief 获取累计读取字节数 @return 读取字节总量 */
quint64 BleGattBrowser::totalBytesRead() const { return m_totalBytesRead; }
/** @brief 获取累计写入字节数 @return 写入字节总量 */
quint64 BleGattBrowser::totalBytesWritten() const { return m_totalBytesWritten; }
/** @brief 获取最后一次服务发现耗时 @return 发现耗时(毫秒) */
qint64 BleGattBrowser::lastDiscoveryDurationMs() const { return m_lastDiscoveryDurationMs; }

/** @brief 重置所有统计计数器 */
void BleGattBrowser::resetStatistics()
{
    m_totalServiceDiscoveries = m_totalCharacteristicReads = m_totalCharacteristicWrites = 0;
    m_totalNotificationsReceived = m_totalDescriptorOps = 0;
    m_totalBytesRead = m_totalBytesWritten = 0;
    m_lastDiscoveryDurationMs = 0;
}

// onRefreshClicked/onConnectionStateChanged/onServicesDiscovered/onTreeItemChanged/populateTree
// 见 BleGattBrowserServices.cpp
// formatHexDump/parseHexInput/getItemProperties/updateButtonStates/updateConnectionStatus
// 见 BleGattBrowserDisplay.cpp
