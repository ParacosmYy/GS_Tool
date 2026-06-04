/**
 * @file CanFrameParserBuild.cpp
 * @brief CAN帧构建、格式化和统计接口实现
 *
 * 从 CanFrameParser.cpp 拆分而来，包含帧编码为SLCAN格式、
 * DBC信号解码、帧格式化和统计计数器接口。
 */

#include "connection/can/CanFrameParser.h"
#include "protocol/can/DbcParser.h"

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
    m_totalCrcErrors = 0;
}
