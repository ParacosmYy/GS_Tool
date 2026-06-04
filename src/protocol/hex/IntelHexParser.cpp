/**
 * @file IntelHexParser.cpp
 * @brief Intel HEX文件解析器实现 — 文件I/O与二进制合并
 *
 * 本文件包含文件级解析、记录合并为二进制数据、统计解析和累积统计追踪。
 * 单条记录的解析/校验和验证/统计计算拆分至 IntelHexParserRecord.cpp。
 *
 * 完整支持所有记录类型(00-05)，包含校验和验证、
 * HEX转BIN输出和解析统计信息计算。
 */

#include "protocol/hex/IntelHexParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace IntelHex {

// ---- 文件I/O与记录合并 ----
// 注: updateBaseAddress 为内部 static 函数，定义于 IntelHexParserRecord.cpp 的 compile unit。
//     parseLine / verifyChecksum / computeStats 同样在 IntelHexParserRecord.cpp 中实现。

/** @brief 从HEX文件解析所有记录到向量 @param filePath HEX文件路径 @param outRecords 输出的记录向量 @return 解析成功且非空返回true，文件无法打开或无有效记录返回false */
bool parseRecords(const QString& filePath, QVector<Record>& outRecords)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open HEX file:" << filePath;
        return false;
    }

    outRecords.clear();
    QTextStream stream(&file);

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        Record record;
        if (!parseLine(line, record)) {
            qWarning() << "Invalid HEX record:" << line;
            continue;
        }
        outRecords.append(record);

        if (record.type == EndOfFile) {
            break;
        }
    }

    file.close();
    return !outRecords.isEmpty();
}

// IntelHexParserRecord.cpp 中的 mergeRecords 需要基地址更新，此处提供链接可见性
// updateBaseAddress 在 IntelHexParserRecord.cpp 中是 static 的，因此此处重新定义
// 以供 mergeRecords 使用。

/** @brief 根据记录类型更新基地址(ExtendedLinearAddr/ExtendedSegmentAddr) @param rec HEX记录 @param currentBase 当前基地址 @return 更新后的基地址 */
static quint32 updateBaseAddress(const Record& rec, quint32 currentBase)
{
    switch (rec.type) {
    case ExtendedLinearAddr:
        // 类型04: 设置32位地址高16位
        if (rec.data.size() >= 2) {
            return (static_cast<quint8>(rec.data[0]) << 24) |
                   (static_cast<quint8>(rec.data[1]) << 16);
        }
        break;
    case ExtendedSegmentAddr:
        // 类型02: 设置20位段地址(左移4位)
        if (rec.data.size() >= 2) {
            return (static_cast<quint8>(rec.data[0]) << 12) |
                   (static_cast<quint8>(rec.data[1]) << 4);
        }
        break;
    default:
        break;
    }
    return currentBase;
}

/** @brief 将HEX记录合并为连续二进制数据块 @param records 已解析的HEX记录向量 @param outBinary 输出的二进制数据(0xFF填充) @param startAddress 输出的起始地址 @return 合并成功返回true，无数据记录或数据过大返回false */
bool mergeRecords(const QVector<Record>& records, QByteArray& outBinary,
                  quint32& startAddress)
{
    if (records.isEmpty()) return false;

    // 第一遍: 确定地址范围（扫描所有记录，计算最小/最大绝对地址）
    quint32 baseAddr = 0;
    quint32 minAddr = 0xFFFFFFFF;
    quint32 maxAddr = 0;
    bool hasData = false;

    for (const auto& rec : records) {
        if (rec.type == DataRecord) {
            quint32 absAddr = baseAddr + rec.address;
            if (absAddr < minAddr) minAddr = absAddr;
            quint32 endAddr = absAddr + rec.byteCount;
            if (endAddr > maxAddr) maxAddr = endAddr;
            hasData = true;
        }
        baseAddr = updateBaseAddress(rec, baseAddr);
    }

    if (!hasData) return false;

    // 地址范围校验: 最大16MB（防止异常HEX文件导致内存爆炸）
    startAddress = minAddr;
    const quint32 totalSize = maxAddr - minAddr;
    if (totalSize == 0 || totalSize > 16 * 1024 * 1024) {
        qWarning() << "HEX data too large or empty:" << totalSize;
        return false;
    }

    // 初始化输出缓冲区(填充0xFF，与Flash默认值一致)
    outBinary.fill(static_cast<char>(0xFF), static_cast<int>(totalSize));

    // 第二遍: 填充数据到输出缓冲区
    baseAddr = 0;
    for (const auto& rec : records) {
        if (rec.type == DataRecord) {
            quint32 absAddr = baseAddr + rec.address;
            quint32 offset = absAddr - minAddr;
            if (offset + rec.byteCount <= totalSize) {
                memcpy(outBinary.data() + offset,
                       rec.data.constData(), rec.byteCount);
            }
        }
        baseAddr = updateBaseAddress(rec, baseAddr);
    }

    return true;
}

/** @brief 一步完成HEX文件解析与二进制合并 @param filePath HEX文件路径 @param outBinary 输出的二进制数据 @param startAddress 输出的起始地址 @return 解析并合并成功返回true，失败返回false */
bool parse(const QString& filePath, QByteArray& outBinary,
           quint32& startAddress)
{
    QVector<Record> records;
    if (!parseRecords(filePath, records)) {
        return false;
    }
    return mergeRecords(records, outBinary, startAddress);
}

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
