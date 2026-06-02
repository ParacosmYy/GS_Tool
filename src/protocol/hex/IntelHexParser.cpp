/**
 * @file IntelHexParser.cpp
 * @brief Intel HEX文件解析器实现
 *
 * 完整支持所有记录类型(00-05)，包含校验和验证、
 * HEX转BIN输出和解析统计信息计算。
 */

#include "protocol/hex/IntelHexParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace IntelHex {

// ---- 内部辅助函数 ----

/**
 * @brief 根据记录类型更新基地址
 *
 * 处理 ExtendedLinearAddr(04) 和 ExtendedSegmentAddr(02) 两种基地址记录，
 * 提取记录数据中的地址高16位并返回更新后的基地址。
 * 其他记录类型不修改基地址，原样返回。
 *
 * @param rec HEX记录
 * @param currentBase 当前基地址
 * @return 更新后的基地址
 */
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

bool parseLine(const QString& line, Record& outRecord)
{
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty() || !trimmed.startsWith(':')) {
        return false;
    }

    // 去掉起始符 ':'
    QString hexData = trimmed.mid(1);
    if (hexData.length() < 10) {
        return false; // 最少: LL(2) + AAAA(4) + TT(2) + CC(2) = 10字符
    }

    bool ok;

    // 字节计数(LL)
    outRecord.byteCount = hexData.mid(0, 2).toUInt(&ok, 16);
    if (!ok) return false;

    // 地址(AAAA)
    outRecord.address = hexData.mid(2, 4).toUInt(&ok, 16);
    if (!ok) return false;

    // 记录类型(TT)
    outRecord.type = hexData.mid(6, 2).toUInt(&ok, 16);
    if (!ok) return false;

    // 验证记录类型范围(00-05)
    if (outRecord.type > 0x05) {
        return false;
    }

    // 数据字段长度检查
    int dataChars = outRecord.byteCount * 2;
    if (hexData.length() < 8 + dataChars + 2) {
        return false;
    }

    // 解析数据字节
    outRecord.data.clear();
    outRecord.data.reserve(outRecord.byteCount);
    for (int i = 0; i < outRecord.byteCount; ++i) {
        unsigned char byte = static_cast<unsigned char>(
            hexData.mid(8 + i * 2, 2).toUInt(&ok, 16));
        if (!ok) return false;
        outRecord.data.append(byte);
    }

    // 校验和(CC)
    outRecord.checksum = hexData.mid(8 + dataChars, 2).toUInt(&ok, 16);
    if (!ok) return false;

    // 验证校验和
    return verifyChecksum(outRecord);
}

bool verifyChecksum(const Record& record)
{
    quint8 sum = record.byteCount;
    sum += (record.address >> 8) & 0xFF;
    sum += record.address & 0xFF;
    sum += record.type;
    for (int i = 0; i < record.data.size(); ++i) {
        sum += static_cast<quint8>(record.data[i]);
    }
    sum += record.checksum;
    // 校验和计算: 所有字节(含checksum)之和 mod 256 应等于 0
    return (sum == 0);
}

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

bool parse(const QString& filePath, QByteArray& outBinary,
           quint32& startAddress)
{
    QVector<Record> records;
    if (!parseRecords(filePath, records)) {
        return false;
    }
    return mergeRecords(records, outBinary, startAddress);
}

// ---- 统计信息 ----

void computeStats(const QVector<Record>& records, ParseStats& stats)
{
    stats = ParseStats();
    quint32 baseAddr = 0;
    quint32 minAddr = 0xFFFFFFFF;
    quint32 maxAddr = 0;

    for (const auto& rec : records) {
        stats.totalLines++;

        if (rec.type == DataRecord) {
            stats.dataRecordCount++;
            stats.totalDataBytes += rec.byteCount;

            quint32 absAddr = baseAddr + rec.address;
            if (absAddr < minAddr) minAddr = absAddr;
            quint32 endAddr = absAddr + rec.byteCount;
            if (endAddr > maxAddr) maxAddr = endAddr;
        } else if (rec.type == ExtendedLinearAddr) {
            if (rec.data.size() >= 2) {
                baseAddr = (static_cast<quint8>(rec.data[0]) << 24) |
                           (static_cast<quint8>(rec.data[1]) << 16);
            }
        } else if (rec.type == ExtendedSegmentAddr) {
            if (rec.data.size() >= 2) {
                baseAddr = (static_cast<quint8>(rec.data[0]) << 12) |
                           (static_cast<quint8>(rec.data[1]) << 4);
            }
        } else if (rec.type == StartSegmentAddr) {
            // 类型03: 80x86 CS:IP (4字节: CS高+CS低+IP高+IP低)
            if (rec.data.size() >= 4) {
                quint32 cs = (static_cast<quint8>(rec.data[0]) << 8) |
                             static_cast<quint8>(rec.data[1]);
                quint32 ip = (static_cast<quint8>(rec.data[2]) << 8) |
                             static_cast<quint8>(rec.data[3]);
                stats.executionAddress = (cs << 4) + ip;
                stats.hasExecutionAddress = true;
            }
        } else if (rec.type == StartLinearAddr) {
            // 类型05: 32位EIP (4字节: 高→低)
            if (rec.data.size() >= 4) {
                stats.executionAddress =
                    (static_cast<quint8>(rec.data[0]) << 24) |
                    (static_cast<quint8>(rec.data[1]) << 16) |
                    (static_cast<quint8>(rec.data[2]) << 8) |
                    static_cast<quint8>(rec.data[3]);
                stats.hasExecutionAddress = true;
            }
        }
    }

    if (stats.dataRecordCount > 0) {
        stats.startAddress = minAddr;
        stats.endAddress = maxAddr;
    }
}

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

} // namespace IntelHex
