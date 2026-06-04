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
        ++m_totalTransferErrors;
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
        ++m_totalTransferErrors;
    }
    return written;
}

// 配置/帧组装见 SpiConnectionConfig.cpp

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

// transferByMode/resetStats见 SpiConnectionStats.cpp
