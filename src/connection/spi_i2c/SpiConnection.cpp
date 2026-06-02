/**
 * @file SpiConnection.cpp
 * @brief SPI总线连接实现 - 通过串口桥接协议
 */

#include "connection/spi_i2c/SpiConnection.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SpiConnection::SpiConnection(QObject* parent)
    : IConnection(parent)
{
}

/**
 * @brief 析构函数
 */
SpiConnection::~SpiConnection()
{
    close();
}

/**
 * @brief 获取连接类型
 * @return Serial类型(SPI归入串行总线)
 */
ConnectionType SpiConnection::type() const
{
    return ConnectionType::Spi;
}

/**
 * @brief 获取连接显示名称
 * @return "SPI:适配器" 格式
 */
QString SpiConnection::name() const
{
    if (m_state == ConnectionState::Connected) {
        return QString("SPI:%1").arg(m_adapterDevice);
    }
    return tr("SPI (未连接)");
}

/**
 * @brief 获取当前状态
 */
ConnectionState SpiConnection::state() const
{
    return m_state;
}

/**
 * @brief 设置底层串口传输通道
 * @param serial 串口IConnection实例
 */
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

/**
 * @brief 打开SPI连接 - 通过串口桥接器初始化
 * @return true=成功
 */
bool SpiConnection::open()
{
    if (!m_serial) {
        emit errorOccurred(tr("未设置串口传输通道"));
        updateState(ConnectionState::Error);
        return false;
    }

    if (!m_serial->open()) {
        emit errorOccurred(tr("串口打开失败"));
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

/**
 * @brief 关闭SPI连接
 */
void SpiConnection::close()
{
    if (m_serial && m_state == ConnectionState::Connected) {
        /// 释放片选
        setChipSelect(m_csPin, false);
    }
    updateState(ConnectionState::Disconnected);
}

/**
 * @brief 发送数据(SPI半双工写)
 * @param data 待发送数据
 * @return 发送字节数
 */
qint64 SpiConnection::write(const QByteArray& data)
{
    if (m_state != ConnectionState::Connected) return -1;

    setChipSelect(m_csPin, true);
    qint64 written = sendCommand(CMD_SPI_WRITE, data);
    setChipSelect(m_csPin, false);
    return written;
}

/**
 * @brief 配置SPI参数
 * @param params 参数映射
 */
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

/**
 * @brief 设置SPI模式
 * @param mode SPI模式(0-3)
 */
void SpiConnection::setSpiMode(int mode)
{
    m_mode = qBound(0, mode, 3);
}

/**
 * @brief 设置时钟频率
 * @param speedHz 时钟频率(Hz)
 */
void SpiConnection::setClockSpeed(int speedHz)
{
    m_clockSpeed = speedHz;
}

/**
 * @brief SPI全双工传输
 * @param txData 发送数据
 * @return 接收数据
 */
QByteArray SpiConnection::transfer(const QByteArray& txData)
{
    if (m_state != ConnectionState::Connected || !m_serial) {
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
    emit dataReceived(rxData);
    return rxData;
}

/**
 * @brief 控制片选引脚
 * @param csPin 片选引脚
 * @param active true=选中
 */
void SpiConnection::setChipSelect(int csPin, bool active)
{
    if (!m_serial || m_state != ConnectionState::Connected) return;

    QByteArray payload;
    payload.append(static_cast<char>(csPin));
    payload.append(static_cast<char>(active ? 1 : 0));
    sendCommand(CMD_SPI_CS, payload);
}

/**
 * @brief 底层串口数据到达回调
 * @param data 从串口适配器收到的响应数据
 */
void SpiConnection::onTransportData(const QByteArray& data)
{
    m_responseBuffer.append(data);
}

/**
 * @brief 更新连接状态
 */
void SpiConnection::updateState(ConnectionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

/**
 * @brief 发送协议命令帧
 * @param cmd 命令字节
 * @param payload 负载数据
 * @return 发送字节数
 */
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

/**
 * @brief 组装SPI传输命令帧
 * @param txData 发送数据
 * @return 完整协议帧
 */
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
