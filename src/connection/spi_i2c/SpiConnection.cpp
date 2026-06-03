/**
 * @file SpiConnection.cpp
 * @brief SPI总线连接实现 - 通过串口桥接协议
 */

#include "connection/spi_i2c/SpiConnection.h"

/** @brief 构造SPI连接对象 @param parent 父QObject指针 */
SpiConnection::SpiConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构SPI连接，关闭并释放资源 */
SpiConnection::~SpiConnection()
{
    close();
}

/** @brief 获取连接类型 @return ConnectionType::Spi */
ConnectionType SpiConnection::type() const
{
    return ConnectionType::Spi;
}

/** @brief 获取连接显示名称 @return 已连接时返回"SPI:适配器"格式，否则返回"未连接" */
QString SpiConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return tr("SPI:%1").arg(m_adapterDevice);
    }
    return tr("SPI (未连接)");
}

/** @brief 获取当前连接状态 @return 当前连接状态枚举值 */
ConnectionState SpiConnection::state() const
{
    return m_state;
}

/** @brief 设置底层串口传输通道，断开旧通道并连接dataReceived信号 @param serial 串口IConnection实例 */
void SpiConnection::setTransport(IConnection* serial)
{
    if (m_serial) {
        disconnect(m_serial, nullptr, this, nullptr);
    }
    m_serial = serial;
    if (m_serial) {
        connect(m_serial, &IConnection::dataReceived,
                this, &SpiConnection::onTransportData);
    }
}

/** @brief 打开SPI连接，通过串口桥接器发送模式/时钟配置 @return true=成功，false=通道未设置或串口打开失败 */
bool SpiConnection::open()
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

    /// 发送SPI配置命令
    QByteArray configPayload;
    configPayload.append(static_cast<char>(m_mode));
    /// 时钟频率4字节小端
    configPayload.append(static_cast<char>(m_clockSpeed & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 8) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 16) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 24) & 0xFF));

    sendCommand(CMD_SPI_CONFIG, configPayload);
    updateState(ConnectionState::Connected);
    return true;
}

/** @brief 关闭SPI连接，释放片选引脚并重置状态 */
void SpiConnection::close()
{
    if (m_serial && m_state == ConnectionState::Connected) {
        /// 释放片选
        setChipSelect(m_csPin, false);
    }
    updateState(ConnectionState::Disconnected);
}

/** @brief SPI半双工写操作，自动控制片选 @param data 待发送数据 @return 发送字节数，未连接返回-1 */
qint64 SpiConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) {
        ++m_errorCount;
        return -1;
    }

    setChipSelect(m_csPin, true);
    qint64 written = sendCommand(CMD_SPI_WRITE, data);
    setChipSelect(m_csPin, false);

    if (written > 0) {
        ++m_totalTransactions;
        m_totalBytesSent += static_cast<quint64>(written);
    } else {
        ++m_errorCount;
    }
    return written;
}

/** @brief 配置SPI参数(mode/clockSpeed/csPin/adapter) @param params 参数映射 */
void SpiConnection::configure(const QVariantMap& params)
{
    if (params.contains("mode")) {
        m_mode = params["mode"].toInt();
    }
    if (params.contains("clockSpeed")) {
        m_clockSpeed = params["clockSpeed"].toInt();
    }
    if (params.contains("csPin")) {
        m_csPin = params["csPin"].toInt();
    }
    if (params.contains("adapter")) {
        m_adapterDevice = params["adapter"].toString();
    }
}

/** @brief 设置SPI模式，限制范围0~3 @param mode SPI模式(0-3) */
void SpiConnection::setSpiMode(int mode)
{
    m_mode = qBound(0, mode, 3);
}

/** @brief 设置时钟频率 @param speedHz 时钟频率(Hz) */
void SpiConnection::setClockSpeed(int speedHz)
{
    m_clockSpeed = speedHz;
}

/** @brief SPI全双工传输，同时发送和接收数据 @param txData 发送数据 @return 接收到的MISO数据 */
QByteArray SpiConnection::transfer(const QByteArray& txData)
{
    if (m_state != ConnectionState::Connected || !m_serial) {
        ++m_errorCount;
        return QByteArray();
    }

    m_responseBuffer.clear();

    /// 构建transfer帧并写入
    QByteArray frame = buildTransferFrame(txData);
    m_serial->write(frame);

    /// 同步等待响应(简化实现，实际应异步)
    QByteArray rxData;
    if (!m_responseBuffer.isEmpty()) {
        rxData = m_responseBuffer;
    }

    /// 更新统计: 全双工同时计发送和接收
    ++m_totalTransactions;
    m_totalBytesSent += static_cast<quint64>(txData.size());
    m_totalBytesReceived += static_cast<quint64>(rxData.size());

    emit dataReceived(rxData);
    return rxData;
}

/** @brief 控制片选引脚电平 @param csPin 片选引脚编号 @param active true=拉低(选中)，false=拉高(释放) */
void SpiConnection::setChipSelect(int csPin, bool active)
{
    if (!m_serial || m_state != ConnectionState::Connected) return;

    QByteArray payload;
    payload.append(static_cast<char>(csPin));
    payload.append(static_cast<char>(active ? 1 : 0));
    sendCommand(CMD_SPI_CS, payload);
}

/** @brief 底层串口数据到达回调，追加到响应缓冲区 @param data 从串口适配器收到的响应数据 */
void SpiConnection::onTransportData(const QByteArray& data)
{
    m_responseBuffer.append(data);
}

/** @brief 更新连接状态，状态变化时发射stateChanged信号 @param newState 新连接状态 */
void SpiConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/** @brief 发送协议命令帧[CMD][LEN(2字节小端)][PAYLOAD] @param cmd 命令字节 @param payload 负载数据 @return 发送字节数 */
qint64 SpiConnection::sendCommand(quint8 cmd, const QByteArray& payload)
{
    if (!m_serial) return -1;

    QByteArray frame;
    frame.append(static_cast<char>(cmd));
    /// 长度(2字节小端)
    quint16 len = static_cast<quint16>(payload.size());
    frame.append(static_cast<char>(len & 0xFF));
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(payload);

    return m_serial->write(frame);
}

/** @brief 组装SPI全双工传输命令帧[CMD][LEN][txData] @param txData 发送数据 @return 完整协议帧 */
QByteArray SpiConnection::buildTransferFrame(const QByteArray& txData)
{
    QByteArray frame;
    frame.append(static_cast<char>(CMD_SPI_TRANSFER));
    quint16 len = static_cast<quint16>(txData.size());
    frame.append(static_cast<char>(len & 0xFF));
    frame.append(static_cast<char>((len >> 8) & 0xFF));
    frame.append(txData);
    return frame;
}

/** @brief 重置所有SPI统计计数器(传输次数/字节数/错误计数) */
void SpiConnection::resetStats()
{
    m_totalTransactions = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}
