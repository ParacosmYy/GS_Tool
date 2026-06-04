/**
 * @file BleConnection.cpp
 * @brief BLE连接实现
 *
 * 模拟BLE连接行为：状态机(Disconnected→Connecting→Connected)，
 * write()回环数据到dataReceived信号，discoverServices()返回预设列表。
 */

#include "connection/ble/BleConnection.h"

static constexpr int CONNECT_DELAY_MS = 1500; ///< 模拟连接建立延迟

/// 模拟GATT服务UUID
static const QStringList MOCK_SERVICES = {
    "00001800-0000-1000-8000-00805f9b34fb",  // Generic Access
    "00001801-0000-1000-8000-00805f9b34fb",  // Generic Attribute
    "0000180a-0000-1000-8000-00805f9b34fb",  // Device Information
    "0000ffe0-0000-1000-8000-00805f9b34fb",  // Custom Service
    "6e400001-b5a3-f393-e0a9-e50e24dcca9e"   // Nordic UART Service
};

/** @brief 构造BLE连接对象，初始化连接定时器和模拟服务列表 @param parent 父对象 */
BleConnection::BleConnection(QObject* parent)
    : IConnection(parent)
    , m_connectTimer(new QTimer(this))
{
    m_connectTimer->setSingleShot(true);
    connect(m_connectTimer, &QTimer::timeout,
            this, &BleConnection::onConnectTimeout);
    initMockServices();
}

/** @brief 析构BLE连接对象，关闭连接释放资源 */
BleConnection::~BleConnection()
{
    close();
}

/** @brief 获取连接类型 @return 固定返回ConnectionType::Ble */
ConnectionType BleConnection::type() const
{
    return ConnectionType::Ble;
}

/** @brief 获取连接显示名称 @return 已连接时返回设备地址，否则返回"未连接" */
QString BleConnection::name() const
{
    return m_deviceAddress.isEmpty() ? tr("未连接") : m_deviceAddress;
}

/** @brief 获取当前BLE连接状态 @return 当前连接状态枚举值 */
ConnectionState BleConnection::state() const
{
    return m_state;
}

/** @brief 打开BLE连接，启动模拟连接延迟定时器 @return true=成功发起连接，false=地址未设置或已连接 */
bool BleConnection::open()
{
    if (m_deviceAddress.isEmpty()) {
        emit errorOccurred(tr("BLE设备地址未设置"));
        ++m_errorCount;
        return false;
    }
    if (m_state == ConnectionState::Connected ||
        m_state == ConnectionState::Connecting) {
        return true;
    }

    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);
    ++m_totalConnections;
    m_connectTimer->start(CONNECT_DELAY_MS);
    return true;
}

/** @brief 关闭BLE连接，停止连接定时器并重置状态为Disconnected */
void BleConnection::close()
{
    m_connectTimer->stop();
    if (m_state != ConnectionState::Disconnected) {
        ++m_totalDisconnections;
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
    }
}

/** @brief 向BLE设备写入数据，模拟回环将写入数据作为接收数据回传 @param data 待写入的数据 @return 实际写入字节数，未连接返回-1 */
qint64 BleConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) {
        emit errorOccurred(tr("BLE未连接，无法写入数据"));
        ++m_errorCount;
        return -1;
    }
    if (data.isEmpty()) {
        return 0;
    }

    const qint64 written = data.size();
    m_bytesWritten += written;
    emit bytesWritten(written);

    /// 更新统计: GATT写入操作
    ++m_totalWrites;
    ++m_totalCharacteristicWrites;  ///< 累计特征值写入次数
    m_totalBytesWritten += static_cast<quint64>(written);

    // 模拟BLE回环: 将写入数据作为接收数据回传(模拟通知)
    QTimer::singleShot(50, this, [this, data]() {
        /// 更新统计: 回环读取+通知
        ++m_totalReads;
        ++m_totalCharacteristicReads;  ///< 累计特征值读取次数
        ++m_totalNotifications;        ///< 累计BLE通知接收次数
        m_totalBytesRead += static_cast<quint64>(data.size());
        emit dataReceived(data);
    });

    return written;
}

/** @brief 配置BLE连接参数 @param params 参数映射，支持address(设备地址)和deviceName(设备名称) */
void BleConnection::configure(const QVariantMap& params)
{
    if (params.contains("address")) {
        m_deviceAddress = params.value("address").toString();
    }
    if (params.contains("deviceName")) {
        m_deviceName = params.value("deviceName").toString();
    }
}

/** @brief 连接到指定地址的BLE设备 @param address BLE设备MAC地址 */
void BleConnection::connectToDevice(const QString& address)
{
    m_deviceAddress = address;
    open();
}

/** @brief 断开BLE设备连接，等同于close() */
void BleConnection::disconnectDevice()
{
    close();
}

/** @brief 发现BLE设备的GATT服务列表 @return 当前预设的服务UUID列表，未连接时返回空列表 */
QStringList BleConnection::discoverServices()
{
    if (m_state != ConnectionState::Connected) {
        return {};
    }

    // 统计: 累计服务发现次数
    m_totalServicesDiscovered += static_cast<quint64>(m_services.size());

    // 模拟服务发现延迟后通知
    QTimer::singleShot(300, this, [this]() {
        emit servicesDiscovered(m_services);
    });

    return m_services;
}

/** @brief 获取BLE设备名称 @return 设备名称字符串 */
QString BleConnection::deviceName() const
{
    return m_deviceName;
}

/** @brief 连接定时器超时回调，将状态从Connecting切换为Connected */
void BleConnection::onConnectTimeout()
{
    if (m_state == ConnectionState::Connecting) {
        m_state = ConnectionState::Connected;
        emit stateChanged(m_state);
    }
}

/** @brief 初始化模拟GATT服务UUID列表 */
void BleConnection::initMockServices()
{
    m_services = MOCK_SERVICES;
}

/** @brief 重置所有统计计数器 */
void BleConnection::resetStats()
{
    m_totalScans = 0;
    m_totalConnections = 0;
    m_totalDisconnections = 0;
    m_totalServicesDiscovered = 0;
    m_totalCharacteristicsRead = 0;
    m_totalWrites = 0;
    m_totalReads = 0;
    m_totalCharacteristicWrites = 0;
    m_totalCharacteristicReads = 0;
    m_totalNotifications = 0;
    m_totalBytesWritten = 0;
    m_totalBytesRead = 0;
    m_errorCount = 0;
}
