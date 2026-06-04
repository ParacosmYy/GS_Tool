/**
 * @file SpiConnectionConfig.cpp
 * @brief SPI连接 - 参数配置与协议帧组装实现
 *
 * 从 SpiConnection.cpp 拆分而来，包含SPI模式/时钟/位序/字长/CS配置
 * 和协议命令帧/传输帧/配置帧的组装方法。
 */

#include "connection/spi_i2c/SpiConnection.h"

/** @brief 配置SPI参数(mode/clockSpeed/csPin/adapter/bitOrder/wordSize/csPolarity) @param params 参数映射 */
void SpiConnection::configure(const QVariantMap& params)
{
    if (params.contains("mode")) {
        int newMode = params["mode"].toInt();
        if (m_mode != newMode) {
            ++m_totalModeChanges;  // 累计SPI模式变更次数
        }
        m_mode = newMode;
    }
    if (params.contains("clockSpeed")) {
        int newSpeed = params["clockSpeed"].toInt();
        if (m_clockSpeed != newSpeed) {
            ++m_totalFrequencyChanges;  // 累计时钟频率变更次数
        }
        m_clockSpeed = newSpeed;
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
    if (m_mode != qBound(0, mode, 3)) {
        ++m_totalModeChanges;  // 累计SPI模式变更次数
    }
    m_mode = qBound(0, mode, 3);
}

/** @brief 设置时钟频率 @param speedHz 时钟频率(Hz) */
void SpiConnection::setClockSpeed(int speedHz)
{
    if (m_clockSpeed != speedHz) {
        ++m_totalFrequencyChanges;  // 累计时钟频率变更次数
    }
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

    ++m_totalCsToggles;
    QByteArray payload;
    payload.append(static_cast<char>(csPin));
    payload.append(static_cast<char>(active ? 1 : 0));
    sendCommand(CMD_SPI_CS, payload);
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
