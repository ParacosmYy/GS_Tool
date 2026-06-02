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

BleConnection::BleConnection(QObject* parent)
    : IConnection(parent)
    , m_connectTimer(new QTimer(this))
{
    m_connectTimer->setSingleShot(true);
    connect(m_connectTimer, &QTimer::timeout,
            this, &BleConnection::onConnectTimeout);
    initMockServices();
}

BleConnection::~BleConnection()
{
    close();
}

ConnectionType BleConnection::type() const
{
    // TODO: ConnectionType枚举添加Ble后改为return ConnectionType::Ble
    return ConnectionType::Ble;
}

QString BleConnection::name() const
{
    return m_deviceAddress.isEmpty() ? tr("未连接") : m_deviceAddress;
}

ConnectionState BleConnection::state() const
{
    return m_state;
}

bool BleConnection::open()
{
    if (m_deviceAddress.isEmpty()) {
        emit errorOccurred(tr("BLE设备地址未设置"));
        return false;
    }
    if (m_state == ConnectionState::Connected ||
        m_state == ConnectionState::Connecting) {
        return true;
    }

    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);
    m_connectTimer->start(CONNECT_DELAY_MS);
    return true;
}

void BleConnection::close()
{
    m_connectTimer->stop();
    if (m_state != ConnectionState::Disconnected) {
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
    }
}

qint64 BleConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) {
        emit errorOccurred(tr("BLE未连接，无法写入数据"));
        return -1;
    }
    if (data.isEmpty()) {
        return 0;
    }

    const qint64 written = data.size();
    m_bytesWritten += written;
    emit bytesWritten(written);

    // 模拟BLE回环: 将写入数据作为接收数据回传
    QTimer::singleShot(50, this, [this, data]() {
        emit dataReceived(data);
    });

    return written;
}

void BleConnection::configure(const QVariantMap& params)
{
    if (params.contains("address")) {
        m_deviceAddress = params.value("address").toString();
    }
    if (params.contains("deviceName")) {
        m_deviceName = params.value("deviceName").toString();
    }
}

void BleConnection::connectToDevice(const QString& address)
{
    m_deviceAddress = address;
    open();
}

void BleConnection::disconnectDevice()
{
    close();
}

QStringList BleConnection::discoverServices()
{
    if (m_state != ConnectionState::Connected) {
        return {};
    }

    // 模拟服务发现延迟后通知
    QTimer::singleShot(300, this, [this]() {
        emit servicesDiscovered(m_services);
    });

    return m_services;
}

QString BleConnection::deviceName() const
{
    return m_deviceName;
}

void BleConnection::onConnectTimeout()
{
    if (m_state == ConnectionState::Connecting) {
        m_state = ConnectionState::Connected;
        emit stateChanged(m_state);
    }
}

void BleConnection::initMockServices()
{
    m_services = MOCK_SERVICES;
}
