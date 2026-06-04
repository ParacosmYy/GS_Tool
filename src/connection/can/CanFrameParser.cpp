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
#include "protocol/can/DbcParser.h"

#include <QFile>
#include <QIODevice>

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

/** @brief 将CanFrame结构编码为SLCAN/LAWICEL格式字节流 @param frame 要编码的CAN帧 @return 编码后的字节流 */
QByteArray CanFrameParser::buildFrame(const CanFrame& frame)
{
    QByteArray result;

    if (frame.fd) {
        /* CAN-FD帧使用d/D前缀 */
        if (frame.rtr) {
            /* CAN-FD不常用RTR，但仍然支持 */
            if (frame.extended) {
                result.append('R');
                result.append(QStringLiteral("%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper().toUtf8());
                result.append(QString::number(frame.dlc).toUtf8());
            } else {
                result.append('r');
                result.append(QStringLiteral("%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper().toUtf8());
                result.append(QString::number(frame.dlc).toUtf8());
            }
        } else if (frame.extended) {
            /* 扩展CAN-FD数据帧: D + 8位hexID + 1位DLC + hex数据 */
            result.append('D');
            result.append(QStringLiteral("%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper().toUtf8());
            result.append(QString::number(frame.dlc).toUtf8());
            for (int i = 0; i < frame.data.size(); ++i) {
                result.append(QStringLiteral("%1").arg(
                    static_cast<unsigned char>(frame.data[i]), 2, 16, QLatin1Char('0')).toUpper().toUtf8());
            }
        } else {
            /* 标准CAN-FD数据帧: d + 3位hexID + 1位DLC + hex数据 */
            result.append('d');
            result.append(QStringLiteral("%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper().toUtf8());
            result.append(QString::number(frame.dlc).toUtf8());
            for (int i = 0; i < frame.data.size(); ++i) {
                result.append(QStringLiteral("%1").arg(
                    static_cast<unsigned char>(frame.data[i]), 2, 16, QLatin1Char('0')).toUpper().toUtf8());
            }
        }
    } else if (frame.rtr) {
        if (frame.extended) {
            /* 扩展远程帧: R + 8位hexID + 1位DLC */
            result.append('R');
            result.append(QStringLiteral("%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper().toUtf8());
            result.append(QString::number(frame.dlc).toUtf8());
        } else {
            /* 标准远程帧: r + 3位hexID + 1位DLC */
            result.append('r');
            result.append(QStringLiteral("%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper().toUtf8());
            result.append(QString::number(frame.dlc).toUtf8());
        }
    } else if (frame.extended) {
        /* 扩展数据帧: T + 8位hexID + 1位DLC + hex数据 */
        result.append('T');
        result.append(QStringLiteral("%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper().toUtf8());
        result.append(QString::number(frame.dlc).toUtf8());
        for (int i = 0; i < frame.data.size(); ++i) {
            result.append(QStringLiteral("%1").arg(
                static_cast<unsigned char>(frame.data[i]), 2, 16, QLatin1Char('0')).toUpper().toUtf8());
        }
    } else {
        /* 标准数据帧: t + 3位hexID + 1位DLC + hex数据 */
        result.append('t');
        result.append(QStringLiteral("%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper().toUtf8());
        result.append(QString::number(frame.dlc).toUtf8());
        for (int i = 0; i < frame.data.size(); ++i) {
            result.append(QStringLiteral("%1").arg(
                static_cast<unsigned char>(frame.data[i]), 2, 16, QLatin1Char('0')).toUpper().toUtf8());
        }
    }
    return result;
}

/** @brief 加载DBC数据库文件，使用DbcParser建立信号映射 @param filePath DBC文件路径 @return 加载成功返回true */
bool CanFrameParser::loadDbcFile(const QString& filePath)
{
    m_dbcFilePath = filePath;

    if (!m_dbcParser) {
        m_dbcParser = new DbcParser(this);
    }

    bool ok = m_dbcParser->loadFromFile(filePath);
    return ok;
}

/** @brief 根据已加载的DBC信息解码CAN帧中的信号值 @param frame 要解码的CAN帧 @return 信号名到物理值的映射表 */
QMap<QString, double> CanFrameParser::decodeSignals(const CanFrame& frame) const
{
    QMap<QString, double> result;

    if (!m_dbcParser) { return result; }

    /* 使用DbcParser进行完整的信号解码(factor/offset/位域提取) */
    result = m_dbcParser->decodeFrame(frame.id, frame.data);

    /* 即使无DBC匹配也添加基础信息 */
    if (result.isEmpty()) {
        /* 将帧数据转为64位值(小端序)用于调试 */
        quint64 raw = 0;
        for (int i = 0; i < qMin(frame.data.size(), 8); ++i) {
            raw |= (static_cast<quint64>(static_cast<quint8>(frame.data[i])) << (i * 8));
        }
        result["_id"] = static_cast<double>(frame.id);
        result["_raw"] = static_cast<double>(raw);
        result["_dlc"] = static_cast<double>(frame.dlc);
    }

    return result;
}

/** @brief 获取DBC消息名 @param frameId 帧ID @return 消息名，未匹配返回空 */
QString CanFrameParser::messageName(quint32 frameId) const
{
    if (!m_dbcParser) { return QString(); }
    DbcMessage msg = m_dbcParser->messageById(frameId);
    return msg.name;
}

/** @brief 将CanFrame转换为可读的字符串描述 @param frame 要格式化的CAN帧 @return 格式化后的字符串 */
QString CanFrameParser::frameToString(const CanFrame& frame)
{
    QString typeStr;
    if (frame.fd)         typeStr += tr("FD ");
    if (frame.extended)   typeStr += tr("EXT ");
    if (frame.rtr)        typeStr += tr("RTR ");
    if (frame.error)      typeStr += tr("ERR ");
    if (typeStr.isEmpty()) typeStr = tr("STD");

    QString idStr = frame.extended
        ? QStringLiteral("%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper()
        : QStringLiteral("%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper();

    return tr("[%1] ID:0x%2 DLC:%3 %4")
        .arg(typeStr, idStr)
        .arg(frame.dlc)
        .arg(frame.data.toHex(' ').toUpper());
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

/** @brief 重置所有解析器统计计数器 */
void CanFrameParser::resetParserStatistics()
{
    m_totalFramesParsed = 0;
    m_totalParseErrors = 0;
    m_totalBytesProcessed = 0;
    m_totalStandardFrames = 0;
    m_totalExtendedFrames = 0;
    m_totalFdFrames = 0;
    m_totalRtrFrames = 0;
}
