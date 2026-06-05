/**
 * @file ProtocolDecoder.cpp
 * @brief 协议解码引擎实现 -- UART/SPI/I2C 协议解码算法
 *
 * 解码算法概述:
 *
 * UART解码:
 *   1. 检测起始位(高→低跳变)
 *   2. 在每个位周期的中点采样数据位
 *   3. LSB优先组装字节
 *   4. 校验停止位(应为高电平)
 *   5. 可选奇偶校验
 *
 * SPI解码:
 *   1. CS低电平有效期间为活跃传输
 *   2. 根据CPOL/CPHA确定采样边沿
 *   3. 每个时钟周期采样一位MOSI/MISO
 *   4. CS上升沿标记帧结束
 *
 * I2C解码:
 *   1. 检测START条件(SCL高时SDA下降沿)
 *   2. 检测STOP条件(SCL高时SDA上升沿)
 *   3. SCL上升沿采样SDA数据位
 *   4. 每9位: 8位数据 + 1位ACK/NACK
 *   5. 第一字节为地址(7位地址 + R/W位)
 */

#include "protocol/logic/ProtocolDecoder.h"

/** @brief 构造协议解码引擎 @param parent 父QObject指针 */
ProtocolDecoder::ProtocolDecoder(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置目标协议类型 */
void ProtocolDecoder::setProtocol(ProtocolType type)
{
    if (m_protocol != type) {
        m_protocol = type;
        ++m_totalProtocolsSwitched;
        reset();
    }
}

/** @brief 获取当前协议类型 */
ProtocolType ProtocolDecoder::protocol() const
{
    return m_protocol;
}

/** @brief 通过通用Map配置协议参数 */
void ProtocolDecoder::configureProtocol(const QVariantMap& config)
{
    if (config.contains(QStringLiteral("baudRate"))) {
        m_uartBaudRate = config.value(
            QStringLiteral("baudRate")).toInt();
    }
    if (config.contains(QStringLiteral("dataBits"))) {
        m_uartDataBits = config.value(
            QStringLiteral("dataBits")).toInt();
    }
    if (config.contains(QStringLiteral("stopBits"))) {
        m_uartStopBits = config.value(
            QStringLiteral("stopBits")).toInt();
    }
    if (config.contains(QStringLiteral("parity"))) {
        m_uartParity = config.value(
            QStringLiteral("parity")).toString();
    }
    if (config.contains(QStringLiteral("clockChannel"))) {
        m_spiClockChannel = config.value(
            QStringLiteral("clockChannel")).toInt();
    }
    if (config.contains(QStringLiteral("mosiChannel"))) {
        m_spiMosiChannel = config.value(
            QStringLiteral("mosiChannel")).toInt();
    }
    if (config.contains(QStringLiteral("misoChannel"))) {
        m_spiMisoChannel = config.value(
            QStringLiteral("misoChannel")).toInt();
    }
    if (config.contains(QStringLiteral("csChannel"))) {
        m_spiCsChannel = config.value(
            QStringLiteral("csChannel")).toInt();
    }
    if (config.contains(QStringLiteral("cpol"))) {
        m_spiCpol = config.value(
            QStringLiteral("cpol")).toBool();
    }
    if (config.contains(QStringLiteral("cpha"))) {
        m_spiCpha = config.value(
            QStringLiteral("cpha")).toBool();
    }
    if (config.contains(QStringLiteral("sdaChannel"))) {
        m_i2cSdaChannel = config.value(
            QStringLiteral("sdaChannel")).toInt();
    }
    if (config.contains(QStringLiteral("sclChannel"))) {
        m_i2cSclChannel = config.value(
            QStringLiteral("sclChannel")).toInt();
    }
}

/** @brief 解码一批采样数据 */
QVector<ProtocolFrame> ProtocolDecoder::decode(
    const QVector<LogicSample>& samples)
{
    if (samples.isEmpty()) {
        return {};
    }

    switch (m_protocol) {
    case ProtocolType::UART:     return decodeUart(samples);
    case ProtocolType::SPI:      return decodeSpi(samples);
    case ProtocolType::I2C:      return decodeI2c(samples);
    case ProtocolType::CAN:      /* TODO: CAN解码 */ break;
    case ProtocolType::OneWire:  /* TODO: OneWire解码 */ break;
    case ProtocolType::Custom:   break;
    }
    return {};
}

/** @brief 重置解码器内部状态 */
void ProtocolDecoder::reset()
{
    /* 无持久状态需要重置 — 每次decode()从零开始 */
}

/** @brief 配置UART解码参数 */
void ProtocolDecoder::setUartConfig(int baudRate, int dataBits,
                                     int stopBits,
                                     const QString& parity)
{
    m_uartBaudRate = qMax(300, baudRate);
    m_uartDataBits = qBound(5, dataBits, 9);
    m_uartStopBits = qBound(1, stopBits, 2);
    m_uartParity   = parity;
}

/** @brief 配置SPI解码参数 */
void ProtocolDecoder::setSpiConfig(int clockChannel, int mosiChannel,
                                    int misoChannel, int csChannel,
                                    bool cpol, bool cpha)
{
    m_spiClockChannel = clockChannel;
    m_spiMosiChannel  = mosiChannel;
    m_spiMisoChannel  = misoChannel;
    m_spiCsChannel    = csChannel;
    m_spiCpol         = cpol;
    m_spiCpha         = cpha;
}

/** @brief 配置I2C解码参数 */
void ProtocolDecoder::setI2cConfig(int sdaChannel, int sclChannel)
{
    m_i2cSdaChannel = sdaChannel;
    m_i2cSclChannel = sclChannel;
}

/**
 * @brief UART协议解码
 * @param samples 采样数据
 * @return 解码帧列表
 *
 * 算法: 检测起始位(高→低跳变)，在每位中点采样数据，
 * LSB优先组装字节，验证停止位。
 */
QVector<ProtocolFrame> ProtocolDecoder::decodeUart(
    const QVector<LogicSample>& samples)
{
    QVector<ProtocolFrame> frames;
    const QVector<bool> signal = extractChannel(samples, m_uartChannel);
    if (signal.isEmpty()) {
        return frames;
    }

    const quint64 bitNs =
        static_cast<quint64>(1e9 / m_uartBaudRate);
    const quint64 halfBitNs = bitNs / 2;

    int idx = 0;
    const int total = signal.size();

    while (idx < total - 1) {
        /* 检测起始位: 高→低跳变 */
        if (signal[idx] || !signal[idx + 1]) {
            ++idx;
            continue;
        }
        const quint64 startTs = samples[idx + 1].timestamp;

        /* 跳到第一位数据的中点(起始位半+半位=1.5位周期) */
        quint64 cursor = samples[idx + 1].timestamp + bitNs + halfBitNs;
        quint8 byte = 0;
        bool framingError = false;

        /* 采样数据位(LSB first) */
        for (int bit = 0; bit < m_uartDataBits; ++bit) {
            const int sIdx = findSampleAtTime(samples, cursor);
            if (sIdx < 0 || sIdx >= total) {
                framingError = true;
                break;
            }
            if (signal[sIdx]) {
                byte |= (static_cast<quint8>(1) << bit);
            }
            cursor += bitNs;
        }

        if (framingError) {
            ++m_totalErrors;
            emit decodeError(tr("UART framing error"));
            idx += 2;
            continue;
        }

        /* 跳过校验位(如有) */
        if (m_uartParity != QStringLiteral("None")) {
            cursor += bitNs;
        }

        /* 验证停止位(应为高电平) */
        const int stopIdx = findSampleAtTime(samples, cursor);
        if (stopIdx >= 0 && stopIdx < total && !signal[stopIdx]) {
            ++m_totalErrors;
            emit decodeError(tr("UART stop bit error"));
        }

        cursor += static_cast<quint64>(m_uartStopBits) * bitNs;

        /* 组装帧 */
        ProtocolFrame frame;
        frame.startTime   = startTs;
        frame.endTime     = cursor;
        frame.type        = QStringLiteral("UART Data");
        frame.data        = QByteArray(1, static_cast<char>(byte));
        frame.decodedText = QString::asprintf("0x%02X '%c'",
                                               byte,
                                               (byte >= 0x20 && byte < 0x7F)
                                                   ? static_cast<char>(byte)
                                                   : '.');

        ++m_totalFramesDecoded;
        ++m_totalBytesDecoded;
        frames.append(frame);
        emit frameDecoded(frame);

        /* 推进到停止位之后 */
        idx = findSampleAtTime(samples, cursor);
        if (idx < 0) {
            break;
        }
    }
    return frames;
}

/**
 * @brief SPI协议解码
 * @param samples 采样数据
 * @return 解码帧列表
 *
 * 算法: 检测CS有效区间，在每个时钟采样边沿读取MOSI/MISO数据。
 */
QVector<ProtocolFrame> ProtocolDecoder::decodeSpi(
    const QVector<LogicSample>& samples)
{
    QVector<ProtocolFrame> frames;
    const QVector<bool> cs    = extractChannel(samples, m_spiCsChannel);
    const QVector<bool> clk   = extractChannel(samples, m_spiClockChannel);
    const QVector<bool> mosi  = extractChannel(samples, m_spiMosiChannel);
    if (cs.isEmpty() || clk.isEmpty()) {
        return frames;
    }

    /* 确定采样边沿: CPOL/CPHA组合 */
    const bool sampleOnRising =
        (m_spiCpol == m_spiCpha);  /* Mode0/Mode2 */

    int frameStart = -1;
    quint8 mosiByte = 0;
    int bitCount = 0;

    for (int i = 1; i < cs.size(); ++i) {
        /* CS下降沿: 帧开始 */
        if (cs[i - 1] && !cs[i]) {
            frameStart = i;
            mosiByte   = 0;
            bitCount   = 0;
            continue;
        }

        /* CS上升沿: 帧结束 */
        if (!cs[i - 1] && cs[i] && frameStart >= 0) {
            if (bitCount > 0) {
                /* 未完成字节也输出 */
                ProtocolFrame frame;
                frame.startTime = samples[frameStart].timestamp;
                frame.endTime   = samples[i].timestamp;
                frame.type      = QStringLiteral("SPI Data");
                frame.data = QByteArray(1, static_cast<char>(mosiByte));
                frame.decodedText = QString::asprintf("MOSI: 0x%02X",
                                                       mosiByte);
                ++m_totalFramesDecoded;
                ++m_totalBytesDecoded;
                frames.append(frame);
                emit frameDecoded(frame);
            }
            frameStart = -1;
            continue;
        }

        /* CS有效期间: 检测时钟边沿 */
        if (frameStart >= 0 && !cs[i]) {
            const bool edgeIsRising = !clk[i - 1] && clk[i];
            const bool edgeIsFalling = clk[i - 1] && !clk[i];

            if ((sampleOnRising && edgeIsRising) ||
                (!sampleOnRising && edgeIsFalling)) {
                mosiByte = (mosiByte << 1) | (mosi[i] ? 1 : 0);
                ++bitCount;

                if (bitCount == 8) {
                    ProtocolFrame frame;
                    frame.startTime = samples[frameStart].timestamp;
                    frame.endTime   = samples[i].timestamp;
                    frame.type      = QStringLiteral("SPI Data");
                    frame.data = QByteArray(
                        1, static_cast<char>(mosiByte));
                    frame.decodedText = QString::asprintf(
                        "MOSI: 0x%02X", mosiByte);
                    ++m_totalFramesDecoded;
                    ++m_totalBytesDecoded;
                    frames.append(frame);
                    emit frameDecoded(frame);
                    mosiByte = 0;
                    bitCount = 0;
                }
            }
        }
    }
    return frames;
}

/**
 * @brief I2C协议解码
 * @param samples 采样数据
 * @return 解码帧列表
 *
 * 算法: 检测START/STOP条件，在SCL上升沿采样SDA，
 * 每9位为一个传输单元(8数据+1 ACK/NACK)。
 */
QVector<ProtocolFrame> ProtocolDecoder::decodeI2c(
    const QVector<LogicSample>& samples)
{
    QVector<ProtocolFrame> frames;
    const QVector<bool> sda = extractChannel(samples, m_i2cSdaChannel);
    const QVector<bool> scl = extractChannel(samples, m_i2cSclChannel);
    if (sda.isEmpty() || scl.isEmpty()) {
        return frames;
    }

    int frameStart = -1;
    quint8 currentByte = 0;
    int bitCount = 0;
    bool isFirstByte = true;
    QByteArray frameData;

    for (int i = 1; i < scl.size(); ++i) {
        /* START条件: SCL高时SDA下降沿 */
        if (scl[i] && scl[i - 1] && !sda[i] && sda[i - 1]) {
            frameStart  = i;
            currentByte = 0;
            bitCount    = 0;
            isFirstByte = true;
            frameData.clear();
            continue;
        }

        /* STOP条件: SCL高时SDA上升沿 */
        if (scl[i] && scl[i - 1] && sda[i] && !sda[i - 1]) {
            if (frameStart >= 0 && !frameData.isEmpty()) {
                ProtocolFrame frame;
                frame.startTime = samples[frameStart].timestamp;
                frame.endTime   = samples[i].timestamp;
                frame.type      = QStringLiteral("I2C Data");
                frame.data      = frameData;
                /* 第一字节解析地址 */
                if (frameData.size() >= 1) {
                    const quint8 addrByte =
                        static_cast<quint8>(frameData[0]);
                    const quint8 addr = (addrByte >> 1) & 0x7F;
                    const bool read = (addrByte & 0x01) != 0;
                    frame.decodedText = QString::asprintf(
                        "Addr:0x%02X %s, %d bytes",
                        addr,
                        read ? "R" : "W",
                        frameData.size() - 1);
                }
                ++m_totalFramesDecoded;
                m_totalBytesDecoded +=
                    static_cast<quint64>(frameData.size());
                frames.append(frame);
                emit frameDecoded(frame);
            }
            frameStart = -1;
            continue;
        }

        /* SCL上升沿: 采样数据位 */
        if (frameStart >= 0 && !scl[i - 1] && scl[i]) {
            if (bitCount < 8) {
                currentByte = (currentByte << 1) | (sda[i] ? 1 : 0);
                ++bitCount;
            } else {
                /* 第9位: ACK/NACK */
                const bool ack = !sda[i];  /* SDA低=ACK */
                if (!ack && isFirstByte) {
                    ++m_totalErrors;
                    emit decodeError(
                        tr("I2C NACK on address"));
                }
                frameData.append(static_cast<char>(currentByte));
                currentByte  = 0;
                bitCount     = 0;
                isFirstByte  = false;
            }
        }
    }
    return frames;
}

/**
 * @brief 从采样序列中提取指定通道的电平序列
 * @param samples 采样数据
 * @param channel 通道索引
 * @return 电平布尔序列(true=High)
 */
QVector<bool> ProtocolDecoder::extractChannel(
    const QVector<LogicSample>& samples, int channel) const
{
    QVector<bool> result;
    result.reserve(samples.size());
    const quint8 mask = static_cast<quint8>(1) << channel;

    for (const LogicSample& s : samples) {
        result.append((s.channelMask & mask) != 0);
    }
    return result;
}

/**
 * @brief 二分查找时间戳最接近targetNs的样本索引
 * @param samples 采样数据(按时间戳递增排列)
 * @param targetNs 目标时间戳(ns)
 * @return 最接近的样本索引，数组为空返回-1
 */
int ProtocolDecoder::findSampleAtTime(
    const QVector<LogicSample>& samples, quint64 targetNs)
{
    if (samples.isEmpty()) {
        return -1;
    }

    int lo = 0;
    int hi = samples.size() - 1;

    while (lo < hi) {
        const int mid = lo + (hi - lo) / 2;
        if (samples[mid].timestamp < targetNs) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    /* 选择距离更近的邻居 */
    if (lo > 0 &&
        (targetNs - samples[lo - 1].timestamp) <
        (samples[lo].timestamp - targetNs)) {
        return lo - 1;
    }
    return lo;
}
