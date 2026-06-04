/**
 * @file CanFrameParser.cpp
 * @brief CAN帧解析器实现 — LAWICEL/SLCAN协议格式，支持CAN-FD和DBC信号解码
 *
 * 支持的LAWICEL帧类型:
 * - t: 标准数据帧(11位ID, 最多8字节)
 * - T: 扩展数据帧(29位ID, 最多8字节)
 * - r: 标准远程帧(11位ID)
 * - R: 扩展远程帧(29位ID)
 * - d: 标准CAN-FD帧(11位ID, 最多64字节)
 * - D: 扩展CAN-FD帧(29位ID, 最多64字节)
 */

#include "connection/can/CanFrameParser.h"

/** @brief 构造CAN帧解析器 @param parent 父QObject指针 */
CanFrameParser::CanFrameParser(QObject* parent)
    : QObject(parent)
{
}

/** @brief 解析原始SLCAN/LAWICEL格式数据为CanFrame结构 @param rawData 原始帧数据 @return 解析后的CanFrame，解析失败返回空帧 */
CanFrame CanFrameParser::parseFrame(const QByteArray& rawData)
{
    CanFrame frame;
    m_totalBytesProcessed += static_cast<quint64>(rawData.size());

    if (rawData.size() < 2) {
        ++m_totalParseErrors;
        return frame;
    }

    const char type = rawData.at(0);
    const char* d = rawData.constData() + 1;

    switch (type) {
    case 't': {
        /* 标准帧: tiiildd... → t + 3位hexID + 1位DLC + N*2位hex数据 */
        if (rawData.size() < 5) { ++m_totalParseErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 3).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(QByteArray(d + 3, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) { ++m_totalParseErrors; return frame; }
        if (rawData.size() < 4 + frame.dlc * 2) { ++m_totalParseErrors; return frame; }
        frame.extended = false;
        frame.data = parseHexData(d + 4, frame.dlc);
        ++m_totalStandardFrames;
        break;
    }
    case 'T': {
        /* 扩展帧: TIIIIIIIIldd... → T + 8位hexID + 1位DLC + N*2位hex数据 */
        if (rawData.size() < 10) { ++m_totalParseErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(QByteArray(d + 8, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) { ++m_totalParseErrors; return frame; }
        if (rawData.size() < 9 + frame.dlc * 2) { ++m_totalParseErrors; return frame; }
        frame.extended = true;
        frame.data = parseHexData(d + 9, frame.dlc);
        ++m_totalExtendedFrames;
        break;
    }
    case 'r': {
        /* 标准远程帧: riii → r + 3位hexID + 1位DLC */
        if (rawData.size() < 5) { ++m_totalParseErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 3).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(QByteArray(d + 3, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) { ++m_totalParseErrors; return frame; }
        frame.extended = false;
        frame.rtr = true;
        ++m_totalRtrFrames;
        ++m_totalStandardFrames;
        break;
    }
    case 'R': {
        /* 扩展远程帧: RIIIIIIIIl → R + 8位hexID + 1位DLC */
        if (rawData.size() < 10) { ++m_totalParseErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(QByteArray(d + 8, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) { ++m_totalParseErrors; return frame; }
        frame.extended = true;
        frame.rtr = true;
        ++m_totalRtrFrames;
        ++m_totalExtendedFrames;
        break;
    }
    case 'd': {
        /* 标准CAN-FD帧: diiildd... → d + 3位hexID + 1位DLC + N*2位hex数据
         * DLC使用CAN-FD非线性映射，数据最多64字节 */
        if (rawData.size() < 5) { ++m_totalParseErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 3).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        const int rawDlc = QByteArray(d + 3, 1).toUInt(&ok, 16);
        if (!ok || rawDlc > 15) { ++m_totalParseErrors; return frame; }
        const int byteCount = CanFrame::dlcToBytes(rawDlc);
        if (rawData.size() < 4 + byteCount * 2) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(rawDlc);
        frame.extended = false;
        frame.fd = true;
        frame.data = parseHexData(d + 4, byteCount);
        ++m_totalFdFrames;
        ++m_totalStandardFrames;
        break;
    }
    case 'D': {
        /* 扩展CAN-FD帧: DIIIIIIIIlldd... → D + 8位hexID + 1位DLC + N*2位hex数据 */
        if (rawData.size() < 10) { ++m_totalParseErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        const int rawDlc = QByteArray(d + 8, 1).toUInt(&ok, 16);
        if (!ok || rawDlc > 15) { ++m_totalParseErrors; return frame; }
        const int byteCount = CanFrame::dlcToBytes(rawDlc);
        if (rawData.size() < 9 + byteCount * 2) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(rawDlc);
        frame.extended = true;
        frame.fd = true;
        frame.data = parseHexData(d + 9, byteCount);
        ++m_totalFdFrames;
        ++m_totalExtendedFrames;
        break;
    }
    default:
        ++m_totalParseErrors;
        break;  /* 非帧类型命令，返回空帧 */
    }
    ++m_totalFramesParsed;
    return frame;
}

// 帧构建(buildFrame)/DBC解码(decodeSignals/loadDbcFile)/格式化(frameToString)/统计(resetParserStatistics)
// 见 CanFrameParserBuild.cpp

/** @brief 解析数据区中所有十六进制字节 @param d 数据区指针 @param byteCount 要解析的字节数 @return 解析后的QByteArray */
QByteArray CanFrameParser::parseHexData(const char* d, int byteCount)
{
    QByteArray data;
    data.reserve(byteCount);
    for (int i = 0; i < byteCount; ++i) {
        data.append(static_cast<char>(parseHexByte(d + i * 2)));
    }
    return data;
}

/** @brief 解析两个十六进制字符为一个字节 @param hex 指向两个十六进制字符的指针 @return 解析后的字节值 */
quint8 CanFrameParser::parseHexByte(const char* hex)
{
    auto hexVal = [](char c) -> quint8 {
        if (c >= '0' && c <= '9') return static_cast<quint8>(c - '0');
        if (c >= 'A' && c <= 'F') return static_cast<quint8>(c - 'A' + 10);
        if (c >= 'a' && c <= 'f') return static_cast<quint8>(c - 'a' + 10);
        return 0;
    };
    if (!hex) return 0;
    return static_cast<quint8>((hexVal(hex[0]) << 4) | hexVal(hex[1]));
}
