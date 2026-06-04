/**
 * @file DataExporterStats.cpp
 * @brief 数据导出器会话统计查询和重置方法实现
 *
 * 从 DataExporter.cpp 拆分而来，包含导出次数/字节数/行数/
 * 各格式计数/耗时等统计getter和resetStats方法。
 */

#include "utils/export/DataExporter.h"

/** @brief 获取累计导出操作总次数 @return 导出次数 */
quint64 DataExporter::totalExports() const { return m_totalExports; }

/** @brief 获取累计导出的字节总数 @return 字节数 */
quint64 DataExporter::totalBytesExported() const { return m_totalBytesExported; }

/** @brief 获取累计导出的数据行总数 @return 行数 */
quint64 DataExporter::totalRowsExported() const { return m_totalRowsExported; }

/** @brief 获取累计导出失败次数 @return 失败次数 */
quint64 DataExporter::totalErrors() const { return m_totalErrors; }

/** @brief 获取累计CSV格式导出次数 @return CSV导出次数 */
quint64 DataExporter::totalCsvExports() const { return m_totalCsvExports; }

/** @brief 获取累计HexDump格式导出次数 @return HexDump导出次数 */
quint64 DataExporter::totalHexDumpExports() const { return m_totalHexDumpExports; }

/** @brief 获取累计JSON格式导出次数 @return JSON导出次数 */
quint64 DataExporter::totalJsonExports() const { return m_totalJsonExports; }

/** @brief 获取累计二进制格式导出次数 @return 二进制导出次数 */
quint64 DataExporter::totalBinExports() const { return m_totalBinExports; }

/** @brief 获取累计Plain格式导出次数 @return Plain导出次数 */
quint64 DataExporter::totalPlainExports() const { return m_totalPlainExports; }

/** @brief 获取累计Timestamped格式导出次数 @return Timestamped导出次数 */
quint64 DataExporter::totalTimestampedExports() const { return m_totalTimestampedExports; }

/** @brief 获取累计导出总耗时(毫秒) @return 总耗时毫秒数 */
qint64 DataExporter::totalExportDurationMs() const { return m_totalExportDurationMs; }

/** @brief 获取最近一次导出操作的耗时(毫秒) @return 最近一次导出耗时 */
qint64 DataExporter::lastExportDurationMs() const { return m_lastExportDurationMs; }

/** @brief 获取最近一次导出的记录数量 @return 最近一次导出的行数 */
quint64 DataExporter::lastExportRowCount() const { return m_lastExportRowCount; }

/** @brief 获取最近一次导出的字节总数 @return 最近一次导出的字节数 */
quint64 DataExporter::lastExportByteCount() const { return m_lastExportByteCount; }

/** @brief 重置所有会话统计计数器(导出次数/字节数/行数/错误数/各格式次数/耗时) */
void DataExporter::resetStats()
{
    m_totalExports = 0;
    m_totalBytesExported = 0;
    m_totalRowsExported = 0;
    m_totalErrors = 0;
    m_totalCsvExports = 0;
    m_totalHexDumpExports = 0;
    m_totalJsonExports = 0;
    m_totalBinExports = 0;
    m_totalPlainExports = 0;
    m_totalTimestampedExports = 0;
    m_totalExportDurationMs = 0;
    m_lastExportDurationMs = 0;
    m_lastExportRowCount = 0;
    m_lastExportByteCount = 0;
}
