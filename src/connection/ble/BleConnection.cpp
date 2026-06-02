/**
 * @file BleConnection.cpp
 * @brief BLE连接实现
 */

#include "connection/ble/BleConnection.h"

BleConnection::BleConnection(QObject* parent)
    : IConnection(parent)
{
}

BleConnection::~BleConnection()
{
    close();
}

ConnectionType BleConnection::type() const
{
    // TODO: 在ConnectionType枚举中添加Ble类型后修改
    return ConnectionType::Serial;
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
    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);
    // TODO: 集成 QLowEnergyController 连接逻辑
    return true;
}

void BleConnection::close()
{
    if (m_state != ConnectionState::Disconnected) {
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
    }
}

qint64 BleConnection::write(const QByteArray& data)
{
    Q_UNUSED(data)
    return -1; // TODO: 写入BLE特征值
}

void BleConnection::configure(const QVariantMap& params)
{
    if (params.contains("address")) {
        m_deviceAddress = params.value("address").toString();
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
    // TODO: 通过 QLowEnergyController::discoverServices 实现
    return m_services;
}
