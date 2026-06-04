/**
 * @file IntelHexParser.h
 * @brief Intel HEX文件解析器
 *
 * 解析.iHex/.hex文件为二进制数据，为OTA升级和固件分析提供基础能力。
 * 支持所有Intel HEX记录类型(00-05)，完整校验和验证，HEX->BIN转换输出。
 * 协作: OtaManager调用parse()获取固件二进制，OtaWidget显示解析统计。
 */
#ifndef INTELHEXPARSER_H
#define INTELHEXPARSER_H

#include <QByteArray>
#include <QString>
#include <QVector>

/// Intel HEX文件解析器命名空间
namespace IntelHex {

// ---- 记录类型常量 ----

/** @brief Intel HEX记录类型枚举 */
enum RecordType {
    DataRecord           = 0x00, ///< 数据记录: 包含目标地址和数据
    EndOfFile            = 0x01, ///< 文件结束: 标记HEX文件结束
    ExtendedSegmentAddr  = 0x02, ///< 扩展段地址: 设置20位段基地址高16位
    StartSegmentAddr     = 0x03, ///< 起始段地址: 设置80x86 CS:IP执行入口
    ExtendedLinearAddr   = 0x04, ///< 扩展线性地址: 设置32位线性基地址高16位
    StartLinearAddr      = 0x05  ///< 起始线性地址: 设置32位EIP执行入口
};

// ---- 数据结构 ----

/** @brief 单条HEX记录 */
struct Record {
    quint8 byteCount = 0;   ///< 数据字节数(LL字段)
    quint16 address = 0;    ///< 16位地址(AAAA字段)
    quint8 type = 0;        ///< 记录类型(TT字段)
    QByteArray data;        ///< 数据(DD字段)
    quint8 checksum = 0;    ///< 校验和(CC字段)
};

/** @brief HEX文件解析统计信息 */
struct ParseStats {
    int totalLines = 0;         ///< 总解析行数(不含空行和注释)
    int dataRecordCount = 0;    ///< 数据记录数(类型00)
    qint64 totalDataBytes = 0;  ///< 总数据字节数
    quint32 startAddress = 0;   ///< 二进制起始地址(最低数据地址)
    quint32 endAddress = 0;     ///< 二进制结束地址(最高数据地址+1)
    quint32 executionAddress = 0; ///< 执行入口地址(类型03或05)
    bool hasExecutionAddress = false; ///< 是否包含执行入口地址
    int checksumErrors = 0;     ///< 校验和错误数
};

// ---- 核心解析函数 ----

/** @brief 解析Intel HEX文件为二进制数据，合并后的完整二进制数据(0xFF填充空白区域) */
bool parse(const QString& filePath, QByteArray& outBinary, quint32& startAddress);

/** @brief 解析为记录列表(保留每条记录的地址信息) */
bool parseRecords(const QString& filePath, QVector<Record>& outRecords);

/** @brief 从HEX行解析单条记录，格式 ":LLAAAATT[DD...]CC"，含校验和验证 */
bool parseLine(const QString& line, Record& outRecord);

/** @brief 验证记录校验和，规则: 所有字节(LL+AAAA+TT+DD+CC)之和 mod 256 == 0 */
bool verifyChecksum(const Record& record);

/** @brief 将记录列表合并为连续二进制数据，0xFF填充空白区域 */
bool mergeRecords(const QVector<Record>& records, QByteArray& outBinary, quint32& startAddress);

// ---- 统计信息 ----

/** @brief 解析HEX文件并获取统计信息 */
bool parseWithStats(const QString& filePath, QByteArray& outBinary,
                    quint32& startAddress, ParseStats& stats);

/** @brief 计算记录列表的统计信息 */
void computeStats(const QVector<Record>& records, ParseStats& stats);

// ---- 累积统计类 ----

/** @brief Intel HEX解析器累积统计追踪器，跨多次parse调用的累积统计 */
class ParserStatsTracker {
public:
    quint64 totalRecordsParsed() const;  ///< 累计解析的HEX记录总数
    quint64 totalBytesParsed() const;    ///< 累计解析的数据字节总数
    quint64 errorCount() const;          ///< 累计解析错误次数
    void resetParserStatistics();        ///< 重置所有累积统计计数器
    void accumulate(const ParseStats& stats); ///< 从一次解析结果中累加统计

private:
    quint64 m_totalRecordsParsed = 0; ///< 累计解析记录总数
    quint64 m_totalBytesParsed = 0;   ///< 累计解析数据字节总数
    quint64 m_errorCount = 0;         ///< 累计解析错误次数
};

} // namespace IntelHex

#endif // INTELHEXPARSER_H
