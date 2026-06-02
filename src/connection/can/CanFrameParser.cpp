/**
 * @file CanFrameParser.cpp
 * @brief CAN帧解析器实现 — LAWICEL/SLCAN协议格式
 */

#include "connection/can/CanFrameParser.h"

CanFrameParser::CanFrameParser(QObject* parent)
    : QObject(parent)
{
}

CanFrame CanFrameParser::parseFrame(const QByteArray& rawData)
{
    CanFrame frame;
    if (rawData.size() < 2) return frame;

    const char type = rawData.at(0);
    const char* d = rawData.constData() + 1;

    switch (type) {
    case 't': {
        /* 标准帧: tiiildd... → t + 3位hexID + 1位DLC + N*2位hex数据 */
        if (rawData.size() < 5) return frame;
        bool ok = false;
        frame.id = QByteArray(d, 3).toUInt(&ok, 16);
        if (!ok) return frame;
        frame.dlc = static_cast<quint8>(QByteArray(d + 3, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) return frame;
        frame.extended = false;
        const int dataLen = frame.dlc;
        for (int i = 0; i < dataLen && (5 + i * 2) < rawData.size(); ++i) {
            frame.data.append(static_cast<char>(parseHexByte(d + 4 + i * 2)));
        }
        break;
    }
    case 'T': {
        /* 扩展帧: TIIIIIIIIldd... → T + 8位hexID + 1位DLC + N*2位hex数据 */
        if (rawData.size() < 10) return frame;
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        if (!ok) return frame;
        frame.dlc = static_cast<quint8>(QByteArray(d + 8, 1).toUInt(&ok, 16));
        if (!ok || frame.dlc > 8) return frame;
        frame.extended = true;
        const int dataLen = frame.dlc;
        for (int i = 0; i < dataLen && (10 + i * 2) < rawData.size(); ++i) {
            frame.data.append(static_cast<char>(parseHexByte(d + 9 + i * 2)));
        }
        break;
    }
    case 'r': {
        /* 标准远程帧: riii → r + 3位hexID + 1位DLC */
        if (rawData.size() < 5) return frame;
        bool ok = false;
        frame.id = QByteArray(d, 3).toUInt(&ok, 16);
        frame.dlc = static_cast<quint8>(QByteArray(d + 3, 1).toUInt(&ok, 16));
        frame.extended = false;
        frame.rtr = true;
        break;
    }
    case 'R': {
        /* 扩展远程帧: RIIIIIIIIl → R + 8位hexID + 1位DLC */
        if (rawData.size() < 10) return frame;
        bool ok = false;
        frame.id = QByteArray(d, 8).toUInt(&ok, 16);
        frame.dlc = static_cast<quint8>(QByteArray(d + 8, 1).toUInt(&ok, 16));
        frame.extended = true;
        frame.rtr = true;
        break;
    }
    default:
        break;  /* 非帧类型命令，返回空帧 */
    }
    return frame;
}

QByteArray CanFrameParser::buildFrame(const CanFrame& frame)
{
    QByteArray result;

    if (frame.rtr) {
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

bool CanFrameParser::loadDbcFile(const QString& filePath)
{
    m_dbcFilePath = filePath;
    /* TODO: 解析DBC文件内容，建立信号映射表
     * DBC格式包含: BO_(报文), SG_(信号), VAL_(值表)等条目
     * 需要按BO_分割报文，按SG_提取信号名、起始位、长度、因子、偏移
     */
    return false;
}

QString CanFrameParser::frameToString(const CanFrame& frame)
{
    QString typeStr;
    if (frame.fd)         typeStr += QStringLiteral("FD ");
    if (frame.extended)   typeStr += QStringLiteral("EXT ");
    if (frame.rtr)        typeStr += QStringLiteral("RTR ");
    if (typeStr.isEmpty()) typeStr = QStringLiteral("STD");

    QString idStr = frame.extended
        ? QStringLiteral("%1").arg(frame.id, 8, 16, QLatin1Char('0')).toUpper()
        : QStringLiteral("%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper();

    return QStringLiteral("[%1] ID:0x%2 DLC:%3 %4")
        .arg(typeStr, idStr)
        .arg(frame.dlc)
        .arg(frame.data.toHex(' ').toUpper());
}

quint8 CanFrameParser::parseHexByte(const char* hex)
{
    auto hexVal = [](char c) -> quint8 {
        if (c >= '0' && c <= '9') return static_cast<quint8>(c - '0');
        if (c >= 'A' && c <= 'F') return static_cast<quint8>(c - 'A' + 10);
        if (c >= 'a' && c <= 'f') return static_cast<quint8>(c - 'a' + 10);
        return 0;
    };
    return static_cast<quint8>((hexVal(hex[0]) << 4) | hexVal(hex[1]));
}
