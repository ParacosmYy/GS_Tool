/**
 * @file IntelHexParserRecord.cpp
 * @brief Intel HEX单条记录解析、校验和验证与统计计算
 *
 * 从 IntelHexParser.cpp 拆分而来，包含记录级别的解析与验证函数:
 *   - 单行HEX文本解析 (parseLine)
 *   - 校验和验证 (verifyChecksum)
 *   - 基地址更新 (updateBaseAddress)
 *   - 统计信息计算 (computeStats)
 *
 * 文件级I/O和二进制合并保留在 IntelHexParser.cpp 中。
 */

#include "protocol/hex/IntelHexParser.h"
#include <QDebug>

namespace IntelHex {

// ---- 记录级辅助函数 ----

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

/** @brief 解析单行HEX文本为Record结构体 @param line HEX文本行(以':'开头) @param outRecord 输出的解析结果 @return 解析成功返回true，格式错误或校验和失败返回false */
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

/** @brief 验证HEX记录的校验和是否正确 @param record 待验证的HEX记录 @return 校验和正确返回true，错误返回false */
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

// ---- 统计信息 ----

/** @brief 从已解析记录中计算统计信息(记录数/数据量/地址范围/执行入口) @param records 已解析的HEX记录向量 @param stats 输出的统计结果 */
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

} // namespace IntelHex
