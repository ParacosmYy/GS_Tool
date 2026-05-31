#ifndef INTELHEXPARSER_H
#define INTELHEXPARSER_H

#include <QByteArray>
#include <QString>
#include <QVector>

// Intel HEX文件解析器 - 解析.iHex/.hex文件为二进制数据
// 为OTA升级和固件分析提供基础能力
namespace IntelHex {

// 单条HEX记录
struct Record {
    quint8 byteCount = 0;   // 数据字节数
    quint16 address = 0;    // 16位地址
    quint8 type = 0;        // 记录类型
    QByteArray data;        // 数据
    quint8 checksum = 0;    // 校验和
};

// 记录类型常量
enum RecordType {
    DataRecord           = 0x00, // 数据记录
    EndOfFile            = 0x01, // 文件结束
    ExtendedSegmentAddr  = 0x02, // 扩展段地址
    StartSegmentAddr     = 0x03, // 起始段地址
    ExtendedLinearAddr   = 0x04, // 扩展线性地址
    StartLinearAddr      = 0x05  // 起始线性地址
};

// 解析Intel HEX文件为二进制数据
// outBinary: 合并后的完整二进制数据
// startAddress: 起始地址
// 返回: 成功/失败
bool parse(const QString& filePath, QByteArray& outBinary, quint32& startAddress);

// 解析为记录列表（保留每条记录的地址信息）
bool parseRecords(const QString& filePath, QVector<Record>& outRecords);

// 从HEX行解析单条记录
// line: 格式 ":LLAAAATT[DD...]CC"
bool parseLine(const QString& line, Record& outRecord);

// 验证记录校验和
bool verifyChecksum(const Record& record);

// 将记录列表合并为连续二进制数据
bool mergeRecords(const QVector<Record>& records, QByteArray& outBinary, quint32& startAddress);

} // namespace IntelHex

#endif // INTELHEXPARSER_H
