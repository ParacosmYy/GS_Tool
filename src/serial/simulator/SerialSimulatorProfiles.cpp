/**
 * @file SerialSimulatorProfiles.cpp
 * @brief 串口设备模拟器 — 响应生成/错误注入/设备配置
 *
 * 从 SerialSimulator.cpp 拆分而来，包含:
 *   - generateResponse(): 占位符替换生成响应
 *   - injectErrors(): 位翻转/丢弃/重复/帧错误注入
 *   - setupGpsProfile()/setupModbusSlaveProfile()/setupSensorArrayProfile()
 */

#include "serial/simulator/SerialSimulator.h"

#include <QRandomGenerator>

// ═══════════════════════════════════════════════════════════
// 内部: 响应生成
// ═══════════════════════════════════════════════════════════

/**
 * @brief 生成响应内容 -- 处理占位符替换
 *
 * {random}: 8位随机hex; {counter}: 递增计数; {hex}: 4字节随机hex; {time}: 毫秒时间戳
 */
QByteArray SerialSimulator::generateResponse(const QByteArray& templ) const
{
    QByteArray result = templ;

    /* {random}: 8位随机hex字符串 */
    while (result.contains("{random}")) {
        QByteArray hex;
        for (int i = 0; i < 8; ++i) {
            hex += QByteArray::number(QRandomGenerator::global()->bounded(16), 16).toUpper();
        }
        result.replace("{random}", hex);
    }

    /* {counter}: 递增计数器 */
    if (result.contains("{counter}")) {
        result.replace("{counter}", QByteArray::number(static_cast<quint64>(m_counter)));
        m_counter++;
    }

    /* {hex}: 4个随机hex字节 */
    while (result.contains("{hex}")) {
        QByteArray hexBytes;
        for (int i = 0; i < 4; ++i) {
            const quint8 byte = static_cast<quint8>(QRandomGenerator::global()->bounded(256));
            hexBytes += QByteArray::number(byte, 16).rightJustified(2, '0').toUpper();
        }
        result.replace("{hex}", hexBytes);
    }

    /* {time}: 当前毫秒时间戳 */
    if (result.contains("{time}")) {
        result.replace("{time}", QByteArray::number(m_timer.elapsed()));
    }

    return result;
}

// ═══════════════════════════════════════════════════════════
// 内部: 错误注入
// ═══════════════════════════════════════════════════════════

void SerialSimulator::injectErrors(QByteArray& data)
{
    if (data.isEmpty()) return;

    /* 1. 位错误注入 */
    if (m_bitErrorRate > 0.0) {
        for (int i = 0; i < data.size(); ++i) {
            for (int bit = 0; bit < 8; ++bit) {
                if (QRandomGenerator::global()->generateDouble() < m_bitErrorRate) {
                    data[i] ^= static_cast<char>(1 << bit);
                    m_stats.totalErrorsInjected++;
                    emit errorInjected(QStringLiteral("bit_error"));
                }
            }
        }
    }

    /* 2. 字节丢弃 */
    if (m_dropRate > 0.0) {
        QByteArray kept;
        kept.reserve(data.size());
        for (int i = 0; i < data.size(); ++i) {
            if (QRandomGenerator::global()->generateDouble() < m_dropRate) {
                m_stats.totalBytesDropped++;
                m_stats.totalErrorsInjected++;
                emit errorInjected(QStringLiteral("dropped_byte"));
            } else {
                kept.append(data[i]);
            }
        }
        data = kept;
    }

    /* 3. 字节重复 */
    if (m_dropRate > 0.0 && !data.isEmpty()) {
        if (QRandomGenerator::global()->generateDouble() < m_dropRate) {
            const int srcIdx = QRandomGenerator::global()->bounded(data.size());
            const int dstIdx = QRandomGenerator::global()->bounded(data.size() + 1);
            data.insert(dstIdx, data.at(srcIdx));
            m_stats.totalErrorsInjected++;
            emit errorInjected(QStringLiteral("duplicated_byte"));
        }
    }

    /* 4. 帧错误 */
    if (m_dropRate > 0.0 && QRandomGenerator::global()->generateDouble() < m_dropRate * 0.5) {
        const char badByte = static_cast<char>(QRandomGenerator::global()->bounded(256));
        data.append(badByte);
        m_stats.totalErrorsInjected++;
        emit errorInjected(QStringLiteral("framing_error"));
    }
}

// ═══════════════════════════════════════════════════════════
// 内部: 预定义设备配置
// ═══════════════════════════════════════════════════════════

/** @brief GPS预设 -- NMEA-0183: $GPGGA/$GPRMC/$GPGSV */
void SerialSimulator::setupGpsProfile()
{
    setResponseDelay(10, 30);
    addRule("GPS", "$GPGGA,123519,{counter},4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n");
    addRule("RMC", "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n");
    addRule("GSV", "$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75\r\n");
    addRule("*",   "$GPGGA,{time},,N,,E,0,00,99.0,,M,,M,,*66\r\n");
}

/** @brief Modbus从站预设 -- 功能码03/06/10 */
void SerialSimulator::setupModbusSlaveProfile()
{
    setResponseDelay(5, 15);
    addRule("\x01\x03", QByteArray("\x01\x03\x08\x00\x64\x00\xC8\x01\x2C\x01\x90\x5A\x3D", 12));
    addRule("\x01\x06", QByteArray("\x01\x06\x00\x01\x00\x03\xD9\xCB", 8));
    addRule("\x01\x10", QByteArray("\x01\x10\x00\x01\x00\x02\x61\x78", 8));
    addRule("*", QByteArray("\x01\x86\x01\x83\xA0", 5));
}

/** @brief 传感器阵列预设 -- 5通道 */
void SerialSimulator::setupSensorArrayProfile()
{
    setResponseDelay(8, 25);
    addRule("READ", "T:{random}C H:{hex}% P:1013hPa L:{counter}lx V:3.3V\r\n");
    addRule("TEMP", "TEMP:{random}\r\n");
    addRule("HUM",  "HUM:{hex}\r\n");
    addRule("ALL",  "CH0:{counter},CH1:{random},CH2:{hex},CH3:{time},CH4:{counter}\r\n");
    addRule("*",    "SENSOR:{time},DATA:{random}\r\n");
}
