/**
 * @file I2cConnection.cpp
 * @brief I2C总线连接实现 - 核心连接生命周期管理
 *
 * 包含I2C连接的打开/关闭/配置/状态管理以及基本读写操作。
 * 总线扫描和突发读写方法见 I2cConnectionScan.cpp。
 *
 * @see I2cConnectionScan.cpp — 总线扫描/突发读写及统计管理
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

/** @brief 从指定设备的寄存器读取数据(I2C读时序) @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param length 读取长度 @return 读取到的数据 */
QByteArray I2cConnection::readRegister(int deviceAddr, int regAddr, int length)
{
    QByteArray data;
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        emit registerRead(regAddr, data);
        return data;
    }

    m_responseBuffer.clear();

    /// 构建并发送I2C读命令帧
    QByteArray frame = buildReadFrame(deviceAddr, regAddr, length);
    m_serial->write(frame);

    /// 解析响应数据
    QByteArray payload = parseResponsePayload();
    if (!payload.isEmpty()) {
        data = payload;
    }

    /// 更新统计: 读操作
    ++m_totalTransactions;
    m_totalBytesSent += static_cast<quint64>(frame.size());
    m_totalBytesReceived += static_cast<quint64>(data.size());
    m_totalBytesRead += static_cast<quint64>(data.size());

    emit registerRead(regAddr, data);
    return data;
}

/** @brief 向指定设备的寄存器写入数据(I2C写时序) @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param data 待写入数据 @return true=写入成功 */
bool I2cConnection::writeRegister(int deviceAddr, int regAddr, const QByteArray& data)
{
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        return false;
    }

    QByteArray frame = buildWriteFrame(deviceAddr, regAddr, data);
    qint64 written = m_serial->write(frame);

    if (written > 0) {
        ++m_totalTransactions;
        m_totalBytesSent += static_cast<quint64>(written);
        m_totalBytesWritten += static_cast<quint64>(data.size());
    } else {
        ++m_errorCount;
        ++m_totalBusErrors;
    }
    return written > 0;
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

/** @brief 发送协议命令帧[CMD][LEN(2字节小端)][PAYLOAD] @param cmd 命令字节 @param payload 负载数据 @return 发送字节数 */
qint64 I2cConnection::sendCommand(quint8 cmd, const QByteArray& payload)
{
    if (!m_serial) return -1;

    QByteArray frame;
    frame.append(static_cast<char>(cmd));
    quint16 len = static_cast<quint16>(payload.size());
    frame.append(static_cast<char>(len & 0xFF));
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(payload);

    return m_serial->write(frame);
}

/** @brief 构建I2C读命令帧[CMD][LEN][deviceAddr+regAddr+length] @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param length 读取长度 @return 完整协议帧 */
QByteArray I2cConnection::buildReadFrame(int deviceAddr, int regAddr, int length)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(regAddr));
    payload.append(static_cast<char>(length));
    return QByteArray().append(static_cast<char>(CMD_I2C_READ))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 构建I2C写命令帧[CMD][LEN][deviceAddr+regAddr+data] @param deviceAddr 设备7位地址 @param regAddr 寄存器地址 @param data 写入数据 @return 完整协议帧 */
QByteArray I2cConnection::buildWriteFrame(int deviceAddr, int regAddr, const QByteArray& data)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(regAddr));
    payload.append(data);
    return QByteArray().append(static_cast<char>(CMD_I2C_WRITE))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 构建I2C突发读命令帧[CMD][LEN][deviceAddr+startReg+count(2字节小端)] @param deviceAddr 设备7位地址 @param startReg 起始寄存器地址 @param count 读取字节数 @return 完整协议帧 */
QByteArray I2cConnection::buildBurstReadFrame(int deviceAddr, int startReg, int count)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(startReg));
    /// 突发长度用2字节小端表示(支持大于255字节)
    payload.append(static_cast<char>(count & 0xFF));
    payload.append(static_cast<char>((count >> 8) & 0xFF));
    return QByteArray().append(static_cast<char>(CMD_I2C_BURST_RD))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 构建I2C突发写命令帧[CMD][LEN][deviceAddr+startReg+data] @param deviceAddr 设备7位地址 @param startReg 起始寄存器地址 @param data 待写入的连续数据 @return 完整协议帧 */
QByteArray I2cConnection::buildBurstWriteFrame(int deviceAddr, int startReg, const QByteArray& data)
{
    QByteArray payload;
    payload.append(static_cast<char>(deviceAddr & 0x7F));
    payload.append(static_cast<char>(startReg));
    payload.append(data);
    return QByteArray().append(static_cast<char>(CMD_I2C_BURST_WR))
                       .append(static_cast<char>(payload.size() & 0xFF))
                       .append(static_cast<char>((payload.size() >> 8) & 0xFF))
                       .append(payload);
}

/** @brief 从响应缓冲区解析协议帧的负载数据(跳过CMD+LEN=3字节帧头) @return 负载数据，无效帧返回空 */
QByteArray I2cConnection::parseResponsePayload()
{
    if (m_responseBuffer.size() < 3) {
        return QByteArray();
    }
    quint16 len = static_cast<quint8>(m_responseBuffer[1]) |
                  (static_cast<quint8>(m_responseBuffer[2]) << 8);
    int dataStart = 3;
    int avail = qMin(static_cast<int>(len), m_responseBuffer.size() - dataStart);
    if (avail <= 0) {
        return QByteArray();
    }
    return m_responseBuffer.mid(dataStart, avail);
}

/** @brief 重置所有I2C统计计数器(传输次数/字节数/错误/NACK/总线错误/设备发现) */
void I2cConnection::resetStats()
{
    m_totalTransactions = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_totalBytesWritten = 0;
    m_totalBytesRead = 0;
    m_errorCount = 0;
    m_nackCount = 0;
    m_totalNacks = 0;
    m_totalBusErrors = 0;
    m_devicesFound = 0;
    m_lastScanResults.clear();
}
