/**
 * @file IntelHexParserStats.cpp
 * @brief Intel HEX解析器 - 统计追踪与带统计解析实现
 *
 * 从 IntelHexParser.cpp 拆分而来，包含ParserStatsTracker的
 * 统计访问/重置/累加方法和parseWithStats带统计解析方法。
 */

#include "protocol/hex/IntelHexParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace IntelHex {

/** @brief 解析HEX文件并同时计算详细统计信息(含校验和错误计数) @param filePath HEX文件路径 @param outBinary 输出的二进制数据 @param startAddress 输出的起始地址 @param stats 输出的解析统计信息 @return 解析成功返回true，失败返回false */
bool parseWithStats(const QString& filePath, QByteArray& outBinary,
                    quint32& startAddress, ParseStats& stats)
{
    QVector<Record> records;

    // 先解析记录，同时统计校验和错误
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open HEX file:" << filePath;
        return false;
    }

    records.clear();
    QTextStream stream(&file);
    stats = ParseStats();

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        Record record;
        if (!parseLine(line, record)) {
            // 解析失败可能是校验和错误或其他格式错误
            // 尝试仅做校验和检查
            QString hexData = line.trimmed().mid(1);
            if (hexData.length() >= 10) {
                // 如果能解析出基本字段但校验和失败，计入错误
                stats.checksumErrors++;
            }
            qWarning() << "Invalid HEX record:" << line;
            continue;
        }
        records.append(record);

        if (record.type == EndOfFile) {
            break;
        }
    }
    file.close();

    if (records.isEmpty()) return false;

    // 计算统计信息
    ParseStats computedStats;
    computeStats(records, computedStats);
    // 保留checksumErrors(已被parseLine过滤的记录)
    computedStats.checksumErrors = stats.checksumErrors;
    stats = computedStats;

    // 合并为二进制
    return mergeRecords(records, outBinary, startAddress);
}

// ── ParserStatsTracker 实现 ──

/** @brief 获取累计解析的HEX记录总数 @return 记录总数 */
quint64 ParserStatsTracker::totalRecordsParsed() const
{
    return m_totalRecordsParsed;
}

/** @brief 获取累计解析的数据字节总数 @return 字节总数 */
quint64 ParserStatsTracker::totalBytesParsed() const
{
    return m_totalBytesParsed;
}

/** @brief 获取累计解析错误次数 @return 错误次数 */
quint64 ParserStatsTracker::errorCount() const
{
    return m_errorCount;
}

/** @brief 重置所有累积统计计数器 */
void ParserStatsTracker::resetParserStatistics()
{
    m_totalRecordsParsed = 0;
    m_totalBytesParsed = 0;
    m_errorCount = 0;
}

/** @brief 从一次解析结果中累加统计 @param stats 单次解析的统计快照 */
void ParserStatsTracker::accumulate(const ParseStats& stats)
{
    m_totalRecordsParsed += static_cast<quint64>(stats.totalLines);
    m_totalBytesParsed += static_cast<quint64>(stats.totalDataBytes);
    m_errorCount += static_cast<quint64>(stats.checksumErrors);
}

} // namespace IntelHex
