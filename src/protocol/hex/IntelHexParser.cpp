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

// parseWithStats/ParserStatsTracker实现见 IntelHexParserStats.cpp

} // namespace IntelHex
