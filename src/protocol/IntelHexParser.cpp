#include "IntelHexParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace IntelHex {

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

    // 字节计数
    outRecord.byteCount = hexData.mid(0, 2).toUInt(&ok, 16);
    if (!ok) return false;

    // 地址
    outRecord.address = hexData.mid(2, 4).toUInt(&ok, 16);
    if (!ok) return false;

    // 记录类型
    outRecord.type = hexData.mid(6, 2).toUInt(&ok, 16);
    if (!ok) return false;

    // 数据
    int dataChars = outRecord.byteCount * 2;
    if (hexData.length() < 8 + dataChars + 2) {
        return false;
    }

    outRecord.data.clear();
    outRecord.data.reserve(outRecord.byteCount);
    for (int i = 0; i < outRecord.byteCount; ++i) {
        unsigned char byte = static_cast<unsigned char>(
            hexData.mid(8 + i * 2, 2).toUInt(&ok, 16));
        if (!ok) return false;
        outRecord.data.append(byte);
    }

    // 校验和
    outRecord.checksum = hexData.mid(8 + dataChars, 2).toUInt(&ok, 16);
    if (!ok) return false;

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

bool mergeRecords(const QVector<Record>& records, QByteArray& outBinary, quint32& startAddress)
{
    if (records.isEmpty()) return false;

    // 计算地址范围
    quint32 baseAddr = 0;
    quint32 minAddr = 0xFFFFFFFF;
    quint32 maxAddr = 0;
    bool hasData = false;

    // 第一遍: 确定地址范围
    for (const auto& rec : records) {
        switch (rec.type) {
        case DataRecord: {
            quint32 absAddr = baseAddr + rec.address;
            if (absAddr < minAddr) minAddr = absAddr;
            quint32 endAddr = absAddr + rec.byteCount;
            if (endAddr > maxAddr) maxAddr = endAddr;
            hasData = true;
            break;
        }
        case ExtendedLinearAddr:
            baseAddr = (static_cast<quint8>(rec.data[0]) << 24) |
                       (static_cast<quint8>(rec.data[1]) << 16);
            break;
        case ExtendedSegmentAddr:
            baseAddr = (static_cast<quint8>(rec.data[0]) << 12) |
                       (static_cast<quint8>(rec.data[1]) << 4);
            break;
        default:
            break;
        }
    }

    if (!hasData) return false;

    startAddress = minAddr;
    quint32 totalSize = maxAddr - minAddr;
    if (totalSize == 0 || totalSize > 16 * 1024 * 1024) {
        qWarning() << "HEX data too large or empty:" << totalSize;
        return false;
    }

    // 初始化输出缓冲区（填充0xFF，与Flash默认值一致）
    outBinary.fill(static_cast<char>(0xFF), static_cast<int>(totalSize));

    // 第二遍: 填充数据
    baseAddr = 0;
    for (const auto& rec : records) {
        switch (rec.type) {
        case DataRecord: {
            quint32 absAddr = baseAddr + rec.address;
            quint32 offset = absAddr - minAddr;
            if (offset + rec.byteCount <= totalSize) {
                memcpy(outBinary.data() + offset, rec.data.constData(), rec.byteCount);
            }
            break;
        }
        case ExtendedLinearAddr:
            baseAddr = (static_cast<quint8>(rec.data[0]) << 24) |
                       (static_cast<quint8>(rec.data[1]) << 16);
            break;
        case ExtendedSegmentAddr:
            baseAddr = (static_cast<quint8>(rec.data[0]) << 12) |
                       (static_cast<quint8>(rec.data[1]) << 4);
            break;
        default:
            break;
        }
    }

    return true;
}

bool parse(const QString& filePath, QByteArray& outBinary, quint32& startAddress)
{
    QVector<Record> records;
    if (!parseRecords(filePath, records)) {
        return false;
    }
    return mergeRecords(records, outBinary, startAddress);
}

} // namespace IntelHex
