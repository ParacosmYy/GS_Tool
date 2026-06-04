/**
 * @file I2cConnection.cpp
 * @brief I2C总线连接实现 - 核心连接生命周期管理
 *
 * 包含I2C连接的构造/析构/打开/关闭/配置/状态管理以及基本写操作。
 * 总线扫描和突发读写方法见 I2cConnectionScan.cpp。
 * 寄存器读写/帧构建/响应解析/统计管理见 I2cConnectionTransfer.cpp。
 *
 * @see I2cConnectionScan.cpp — 总线扫描/突发读写及探测
 * @see I2cConnectionTransfer.cpp — 寄存器读写/帧构建/响应解析/统计管理
 */

#include "connection/spi_i2c/I2cConnection.h"

/** @brief 构造I2C连接对象 @param parent 父QObject指针 */
I2cConnection::I2cConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构I2C连接，关闭并释放资源 */
I2cConnection::~I2cConnection()
{
    close();
}

/** @brief 获取连接类型 @return ConnectionType::I2c */
ConnectionType I2cConnection::type() const
{
    return ConnectionType::I2c;
}

/** @brief 获取连接显示名称 @return 已连接时返回"I2C:适配器@地址"格式，否则返回"未连接" */
QString I2cConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return tr("I2C:%1@0x%2")
            .arg(m_adapterDevice)
            .arg(m_deviceAddress, 2, 16, QChar('0'));
    }
    return tr("I2C (未连接)");
}

/** @brief 获取当前连接状态 @return 当前连接状态枚举值 */
ConnectionState I2cConnection::state() const
{
    return m_state;
}

/** @brief 设置底层串口传输通道，断开旧通道并连接dataReceived信号 @param serial 串口IConnection实例 */
void I2cConnection::setTransport(IConnection* serial)
{
    if (m_serial) {
        disconnect(m_serial, nullptr, this, nullptr);
    }
    m_serial = serial;
    if (m_serial) {
        connect(m_serial, &IConnection::dataReceived,
                this, &I2cConnection::onTransportData);
    }
}

/** @brief 打开I2C连接，通过串口桥接器发送配置命令 @return true=成功，false=通道未设置或串口打开失败 */
bool I2cConnection::open()
{
    ++m_totalOpenAttempts;  // 累计open()调用次数
    if (!m_serial) {
        emit errorOccurred(tr("未设置串口传输通道"));
        ++m_errorCount;
        updateState(ConnectionState::Error);
        return false;
    }

    if (!m_serial->open()) {
        emit errorOccurred(tr("串口打开失败"));
        ++m_errorCount;
        updateState(ConnectionState::Error);
        return false;
    }

    /// 发送I2C配置命令(设备地址+时钟频率)
    QByteArray configPayload;
    configPayload.append(static_cast<char>(m_deviceAddress));
    /// 时钟频率4字节小端
    configPayload.append(static_cast<char>(m_clockSpeed & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 8) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 16) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 24) & 0xFF));

    sendCommand(CMD_I2C_CONFIG, configPayload);
    updateState(ConnectionState::Connected);
    return true;
}

/** @brief 关闭I2C连接，将状态重置为Disconnected */
void I2cConnection::close()
{
    updateState(ConnectionState::Disconnected);
}

/** @brief 发送I2C写操作数据 @param data 待发送数据 @return 发送字节数，未连接返回-1 */
qint64 I2cConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) {
        ++m_errorCount;
        return -1;
    }

    qint64 written = sendCommand(CMD_I2C_WRITE, data);

    if (written > 0) {
        ++m_totalTransactions;
        m_totalBytesSent += static_cast<quint64>(written);
        m_totalBytesWritten += static_cast<quint64>(written);
    } else {
        ++m_errorCount;
        ++m_totalBusErrors;
    }
    return written;
}

/** @brief 配置I2C参数(deviceAddress/adapter/clockSpeed) @param params 参数映射 */
void I2cConnection::configure(const QVariantMap& params)
{
    if (params.contains("deviceAddress")) {
        m_deviceAddress = params["deviceAddress"].toInt();
    }
    if (params.contains("adapter")) {
        m_adapterDevice = params["adapter"].toString();
    }
    if (params.contains("clockSpeed")) {
        m_clockSpeed = params["clockSpeed"].toInt();
    }
}

/** @brief 底层串口数据到达回调，追加到响应缓冲区 @param data 从串口适配器收到的响应数据 */
void I2cConnection::onTransportData(const QByteArray& data)
{
    m_responseBuffer.append(data);
}

/** @brief 更新连接状态，状态变化时发射stateChanged信号 @param newState 新连接状态 */
void I2cConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
