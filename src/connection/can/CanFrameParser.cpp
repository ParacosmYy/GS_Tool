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
 * - e: CAN错误帧(错误标志位+错误类型解码)
 *
 * CAN错误帧使用 'e' 前缀(LAWICEL扩展)，格式:
 *   eIIIIIIII DD...
 *   8位hex ID + 空格 + 错误数据字节(包含错误标志位)
 *
 * 错误分类基于CAN 2.0B标准中的错误帧定义:
 *   - 位错误(Bit Error): TX和RX电平不匹配
 *   - 填充错误(Stuff Error): 6个连续相同电平位
 *   - CRC错误(CRC Error): CRC校验失败
 *   - 格式错误(Form Error): 固定格式位域非法
 *   - 应答错误(ACK Error): ACK槽无应答
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
        if (rawData.size() < 5) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 3).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(QByteArray(d + 3, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) { ++m_totalParseErrors; return frame; }
        if (rawData.size() < 4 + frame.dlc * 2) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
        frame.extended = false;
        frame.data = parseHexData(d + 4, frame.dlc);
        ++m_totalStandardFrames;
        break;
    }
    case 'T': {
        /* 扩展帧: TIIIIIIIIldd... → T + 8位hexID + 1位DLC + N*2位hex数据 */
        if (rawData.size() < 10) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        frame.dlc = static_cast<quint8>(QByteArray(d + 8, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) { ++m_totalParseErrors; return frame; }
        if (rawData.size() < 9 + frame.dlc * 2) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
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
        if (rawData.size() < 5) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 3).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        const int rawDlc = QByteArray(d + 3, 1).toUInt(&ok, 16);
        if (!ok || rawDlc > 15) { ++m_totalParseErrors; return frame; }
        const int byteCount = CanFrame::dlcToBytes(rawDlc);
        if (rawData.size() < 4 + byteCount * 2) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
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
        if (rawData.size() < 10) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        const int rawDlc = QByteArray(d + 8, 1).toUInt(&ok, 16);
        if (!ok || rawDlc > 15) { ++m_totalParseErrors; return frame; }
        const int byteCount = CanFrame::dlcToBytes(rawDlc);
        if (rawData.size() < 9 + byteCount * 2) { ++m_totalParseErrors; ++m_totalCrcErrors; return frame; }
        frame.dlc = static_cast<quint8>(rawDlc);
        frame.extended = true;
        frame.fd = true;
        frame.data = parseHexData(d + 9, byteCount);
        ++m_totalFdFrames;
        ++m_totalExtendedFrames;
        break;
    }
    case 'e': {
        /* CAN错误帧: eIIIIIIII 或 eIIIIIIII DD...
         * 8位hex ID(包含错误标志位) + 可选的错误数据字节
         * ID字段的位定义(基于CAN错误帧格式):
         *   bit[2:0] = 错误类型(0=Bit, 1=Stuff, 2=CRC, 3=Form, 4=ACK)
         *   bit[7:3] = 错误位置/附加信息
         */
        if (rawData.size() < 9) { ++m_totalParseErrors; return frame; }
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        if (!ok) { ++m_totalParseErrors; return frame; }
        frame.error = true;
        /* 解析可选的错误数据字节 */
        if (rawData.size() > 9) {
            int dataChars = rawData.size() - 9;
            if (dataChars % 2 == 0) {
                frame.data = parseHexData(d + 8, dataChars / 2);
            }
        }
        /* 分类错误类型并发射信号 */
        parseErrorFrame(frame);
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

/**
 * @brief 解析CAN错误帧，分类错误类型并更新计数器
 *
 * 错误类型判定优先级:
 * 1. 检查帧数据字节中的错误标志位(CAN控制器硬件上报)
 * 2. 根据ID低3位作为错误类型索引(软件兼容层)
 * 3. 无法判定时标记为ErrorNone(通用错误帧)
 *
 * @param frame 接收到的错误帧(已标记error=true，将被填充errorType)
 */
void CanFrameParser::parseErrorFrame(CanFrame& frame)
{
    ++m_totalErrorFrames;

    /* 从帧数据中的错误标志字节判定错误类型
     * 典型CAN控制器错误帧数据格式:
     *   byte[0] bit0=BitError, bit1=StuffError, bit2=CRCError, bit3=FormError, bit4=ACKError
     *   多位同时置位时按优先级取最高位 */
    if (!frame.data.isEmpty()) {
        quint8 errFlags = static_cast<quint8>(frame.data[0]);
        if (errFlags & 0x01) {
            frame.errorType = CanFrame::BitError;
            ++m_totalBitErrors;
        } else if (errFlags & 0x02) {
            frame.errorType = CanFrame::StuffError;
            ++m_totalStuffErrors;
        } else if (errFlags & 0x04) {
            frame.errorType = CanFrame::CrcError;
            ++m_totalCrcFrameErrors;
        } else if (errFlags & 0x08) {
            frame.errorType = CanFrame::FormError;
            ++m_totalFormErrors;
        } else if (errFlags & 0x10) {
            frame.errorType = CanFrame::AckError;
            ++m_totalAckErrors;
        }
    } else {
        /* 无数据字节时从ID低3位解码错误类型(兼容模式) */
        switch (frame.id & 0x07) {
        case 0x00: frame.errorType = CanFrame::BitError;  ++m_totalBitErrors; break;
        case 0x01: frame.errorType = CanFrame::StuffError; ++m_totalStuffErrors; break;
        case 0x02: frame.errorType = CanFrame::CrcError;  ++m_totalCrcFrameErrors; break;
        case 0x03: frame.errorType = CanFrame::FormError; ++m_totalFormErrors; break;
        case 0x04: frame.errorType = CanFrame::AckError;  ++m_totalAckErrors; break;
        default:   frame.errorType = CanFrame::ErrorNone; break;
        }
    }

    /* 发射错误帧检测信号 */
    emit errorFrameDetected(frame, frame.errorType);
}

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
