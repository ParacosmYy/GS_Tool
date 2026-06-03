/**
 * @file CanFrameParser.cpp
 * @brief CAN帧解析器实现 — LAWICEL/SLCAN协议格式
 */

#include "connection/can/CanFrameParser.h"

#include <QFile>
#include <QIODevice>

/** @brief 构造CAN帧解析器 @param parent 父QObject指针 */
CanFrameParser::CanFrameParser(QObject* parent)
    : QObject(parent)
{
}

/** @brief 解析原始SLCAN/LAWICEL格式数据为CanFrame结构 @param rawData 原始帧数据(以t/T/r/R开头) @return 解析后的CanFrame，解析失败返回空帧 */
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
        const int dataLen = frame.dlc;
        for (int i = 0; i < dataLen && (5 + i * 2) < rawData.size(); ++i) {
            frame.data.append(static_cast<char>(parseHexByte(d + 4 + i * 2)));
        }
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
        const int dataLen = frame.dlc;
        for (int i = 0; i < dataLen && (10 + i * 2) < rawData.size(); ++i) {
            frame.data.append(static_cast<char>(parseHexByte(d + 9 + i * 2)));
        }
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

/** @brief 加载DBC数据库文件，解析报文定义 @param filePath DBC文件路径 @return 加载成功返回true */
bool CanFrameParser::loadDbcFile(const QString& filePath)
{
    m_dbcFilePath = filePath;

    /* 使用DbcParser解析DBC文件 */
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    /* 按行解析BO_报文定义，提取报文ID和报文名 */
    const QStringList lines = content.split('\n');
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("BO_ ")) {
            /* BO_<ID> <MessageName>: ... */
            QStringList parts = trimmed.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                bool ok = false;
                quint32 msgId = parts[1].toUInt(&ok);
                if (ok) {
                    /* 移除冒号后缀 */
                    QString msgName = parts[2];
                    msgName.remove(':');
                    m_messageNames[msgId] = msgName;
                }
            }
        }
    }

    m_dbcLoaded = !m_messageNames.isEmpty();
    return m_dbcLoaded;
}

/** @brief 根据已加载的DBC信息解码CAN帧中的信号值 @param frame 要解码的CAN帧 @return 信号名到值的映射表 */
QMap<QString, double> CanFrameParser::decodeSignals(const CanFrame& frame) const
{
    QMap<QString, double> result;

    if (!m_dbcLoaded) { return result; }

    /* 查找报文名 */
    QString msgName = m_messageNames.value(frame.id);
    if (msgName.isEmpty()) { return result; }

    /* 将帧数据转为64位值（小端序）用于位域提取 */
    quint64 raw = 0;
    for (int i = 0; i < qMin(frame.data.size(), 8); ++i) {
        raw |= (static_cast<quint64>(static_cast<quint8>(frame.data[i])) << (i * 8));
    }

    /* 存储原始值 — 实际信号解码需要DbcParser的完整信号定义
     * 此处提供基础解码: 按字节分割为channel_0..N */
    for (int i = 0; i < qMin(frame.data.size(), 8); ++i) {
        result[QString("byte_%1").arg(i)] =
            static_cast<double>(static_cast<quint8>(frame.data[i]));
    }

    /* 添加帧ID和原始值 */
    result["_id"] = static_cast<double>(frame.id);
    result["_raw"] = static_cast<double>(raw);

    return result;
}

/** @brief 将CanFrame转换为可读的字符串描述 @param frame 要格式化的CAN帧 @return 格式化后的字符串 */
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

/** @brief 解析两个十六进制字符为一个字节 @param hex 指向两个十六进制字符的指针 @return 解析后的字节值 */
quint8 CanFrameParser::parseHexByte(const char* hex)
{
    auto hexVal = [](char c) -> quint8 {
        if (c >= '0' && c <= '9') return static_cast<quint8>(c - '0');
        if (c >= 'A' && c <= 'F') return static_cast<quint8>(c - 'A' + 10);
        if (c >= 'a' && c <= 'f') return static_cast<quint8>(c - 'a' + 10);
        return 0;
    };
    /* 防御空指针 — 调用方已通过rawData.size()确保不会越界
     * 但仍需保护hex[1]的访问安全性 */
    if (!hex) return 0;
    return static_cast<quint8>((hexVal(hex[0]) << 4) | hexVal(hex[1]));
}

/** @brief 重置所有解析器统计计数器 */
void CanFrameParser::resetParserStatistics()
{
    m_totalFramesParsed = 0;
    m_totalParseErrors = 0;
    m_totalBytesProcessed = 0;
}
