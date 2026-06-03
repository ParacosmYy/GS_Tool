/**
 * @file SpiConnection.cpp
 * @brief SPI总线连接实现 - 核心连接生命周期管理
 *
 * 包含SPI连接的打开/关闭/配置/状态管理以及片选控制。
 * 传输相关方法见 SpiConnectionTransfer.cpp。
 *
 * @see SpiConnectionTransfer.cpp — SPI传输方法及统计管理
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

/** @brief 打开SPI连接，通过串口桥接器发送完整配置 @return true=成功，false=通道未设置或串口打开失败 */
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

    /// 发送完整SPI配置命令(含模式/时钟/位序/字长/CS极性)
    QByteArray configFrame = buildConfigFrame();
    sendCommand(CMD_SPI_CONFIG, configFrame);
    updateState(ConnectionState::Connected);
    return true;
}

/** @brief 关闭SPI连接，释放片选引脚并重置状态 */
void SpiConnection::close()
{
    if (m_serial && m_state == ConnectionState::Connected) {
        /// 根据CS极性释放片选
        bool releaseLevel = !m_csActiveLow;
        setChipSelect(m_csPin, releaseLevel);
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

    /// 根据CS极性决定选中电平
    bool selectLevel = m_csActiveLow;
    setChipSelect(m_csPin, selectLevel);
    qint64 written = sendCommand(CMD_SPI_WRITE, data);
    bool releaseLevel = !m_csActiveLow;
    setChipSelect(m_csPin, releaseLevel);

    if (written > 0) {
        ++m_totalTransfers;
        m_totalBytesSent += static_cast<quint64>(written);
        /// 按当前SPI模式累计
        if (m_mode >= 0 && m_mode < 4) {
            ++m_transferByMode[m_mode];
        }
    } else {
        ++m_errorCount;
    }
    return written;
}

/** @brief 配置SPI参数(mode/clockSpeed/csPin/adapter/bitOrder/wordSize/csPolarity) @param params 参数映射 */
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
    if (params.contains("bitOrder")) {
        int order = params["bitOrder"].toInt();
        m_bitOrder = (order == 1) ? SpiBitOrder::LSB : SpiBitOrder::MSB;
    }
    if (params.contains("wordSize")) {
        int ws = params["wordSize"].toInt();
        if (ws == 16) m_wordSize = SpiWordSize::Bit16;
        else if (ws == 32) m_wordSize = SpiWordSize::Bit32;
        else m_wordSize = SpiWordSize::Bit8;
    }
    if (params.contains("csActiveLow")) {
        m_csActiveLow = params["csActiveLow"].toBool();
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

/** @brief 设置位序 @param order MSB或LSB位序 */
void SpiConnection::setBitOrder(SpiBitOrder order)
{
    m_bitOrder = order;
}

/** @brief 设置字长 @param wordSize 8/16/32位字长 */
void SpiConnection::setWordSize(SpiWordSize wordSize)
{
    m_wordSize = wordSize;
}

/** @brief 设置CS极性 @param activeLow true=低电平有效，false=高电平有效 */
void SpiConnection::setCsPolarity(bool activeLow)
{
    m_csActiveLow = activeLow;
}

/** @brief 控制片选引脚电平 @param csPin 片选引脚编号 @param active true=选中，false=释放 */
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

/** @brief 组装完整SPI配置命令帧(模式+时钟+位序+字长+CS极性) @return 配置负载数据 */
QByteArray SpiConnection::buildConfigFrame()
{
    QByteArray configPayload;
    configPayload.append(static_cast<char>(m_mode));
    /// 时钟频率4字节小端
    configPayload.append(static_cast<char>(m_clockSpeed & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 8) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 16) & 0xFF));
    configPayload.append(static_cast<char>((m_clockSpeed >> 24) & 0xFF));
    /// 位序: 0=MSB, 1=LSB
    configPayload.append(static_cast<char>(
        (m_bitOrder == SpiBitOrder::LSB) ? 1 : 0));
    /// 字长: 8/16/32
    configPayload.append(static_cast<char>(
        static_cast<int>(m_wordSize)));
    /// CS极性: 0=高有效, 1=低有效
    configPayload.append(static_cast<char>(m_csActiveLow ? 1 : 0));
    return configPayload;
}

/** @brief 获取指定SPI模式的传输次数 @param mode SPI模式(0-3) @return 该模式累计传输次数 */
quint64 SpiConnection::transferByMode(int mode) const
{
    if (mode < 0 || mode >= 4) return 0;
    return m_transferByMode[mode];
}

/** @brief 重置所有SPI统计计数器(传输次数/字节数/错误计数/模式统计) */
void SpiConnection::resetStats()
{
    m_totalTransfers = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
    for (int i = 0; i < 4; ++i) {
        m_transferByMode[i] = 0;
    }
}
