/**
 * @file DataExporter.cpp
 * @brief 数据导出器实现 - Plain/HexDump/CSV/Timestamped/Bin/Json 六种格式
 *
 * HexDump 格式将所有行数据拼接后按经典16字节/行输出。
 * 时间范围过滤仅在 exportToFile 模式下支持。
 * 所有写入操作均检查 QFile 错误状态，失败时发射 exportError 信号。
 *
 * CSV增强:
 * - 可配置列分隔符(setCsvDelimiter)，默认逗号
 * - 可配置BOM头(setCsvBomEnabled)，默认启用，确保Excel中文兼容
 *
 * 统计增强:
 * - 每次导出记录耗时(totalExportDurationMs/lastExportDurationMs)
 * - 每次导出记录行数和字节数(lastExportRowCount/lastExportByteCount)
 * - 成功后发射exportCompleted信号携带完整统计摘要
 *
 * 流式导出方法见 DataExporterStreamed.cpp
 * EDL范围导出方法见 DataExporterEdl.cpp
 */

#include "utils/export/DataExporter.h"
#include "utils/crypto/HexConverter.h"
#include "shared/AppConstants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/** @brief 构造数据导出器 @param parent 父对象 */
DataExporter::DataExporter(QObject* parent) : QObject(parent) {}

// ---- CSV配置 ----

/** @brief 设置CSV列分隔符 @param delim 分隔符字符 */
void DataExporter::setCsvDelimiter(QChar delim) {
    m_csvDelimiter = delim;
}

/** @brief 获取当前CSV列分隔符 @return 分隔符字符 */
QChar DataExporter::csvDelimiter() const {
    return m_csvDelimiter;
}

/** @brief 设置CSV是否写入UTF-8 BOM头 @param enable true=写入BOM */
void DataExporter::setCsvBomEnabled(bool enable) {
    m_csvBomEnabled = enable;
}

/** @brief 获取CSV是否写入BOM头 @return true=启用BOM */
bool DataExporter::isCsvBomEnabled() const {
    return m_csvBomEnabled;
}

// ---- 公共入口 ----

/** @brief 导出数据到文件(批量模式)，记录耗时和统计 @param filePath 目标文件路径 @param format 导出格式 @param lines 终端行数据 @param from 起始时间过滤 @param to 结束时间过滤 @return 是否成功 */
bool DataExporter::exportToFile(const QString& filePath, Format format,
                                 const QVector<TerminalLine>& lines,
                                 const QDateTime& from, const QDateTime& to)
{
    if (lines.isEmpty() || filePath.isEmpty()) return false;

    ++m_totalExports;
    QVector<TerminalLine> filtered = filterByTime(lines, from, to);
    if (filtered.isEmpty()) return false;

    // 开始计时
    m_exportTimer.start();

    bool ok = false;
    switch (format) {
    case Plain:       ok = exportPlain(filePath, filtered); break;
    case HexDump:     ok = exportHexDump(filePath, filtered); ++m_totalHexDumpExports; break;
    case Csv:         ok = exportCsv(filePath, filtered); ++m_totalCsvExports; break;
    case Timestamped: ok = exportTimestamped(filePath, filtered); break;
    case Bin:         ok = exportBin(filePath, filtered); ++m_totalBinExports; break;
    case Json:        ok = exportJson(filePath, filtered); ++m_totalJsonExports; break;
    default:
        ++m_totalErrors;
        emit exportError(filePath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }

    // 计算耗时
    qint64 durationMs = m_exportTimer.elapsed();
    m_lastExportDurationMs = durationMs;
    m_totalExportDurationMs += durationMs;

    if (ok) {
        quint64 byteCount = 0;
        for (const auto& line : filtered) {
            byteCount += static_cast<quint64>(line.data.size());
        }
        m_totalBytesExported += byteCount;
        m_totalRowsExported += static_cast<quint64>(filtered.size());
        m_lastExportRowCount = static_cast<quint64>(filtered.size());
        m_lastExportByteCount = byteCount;
        emit exportCompleted(filePath, format,
                             m_lastExportRowCount, byteCount, durationMs);
    } else {
        ++m_totalErrors;
        m_lastExportRowCount = 0;
        m_lastExportByteCount = 0;
    }
    return ok;
}

// ---- 辅助方法 ----

/** @brief 打开文本文件用于写入并设置UTF8编码，失败时发射exportError @param file 文件对象 @param out 文本流 @param path 文件路径 @return 是否成功 */
bool DataExporter::openTextFile(QFile& file, QTextStream& out, const QString& path)
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    out.setDevice(&file);
    out.setEncoding(QStringConverter::Utf8);
    return true;
}

/** @brief 刷新流并检查写入错误 @param file 文件对象 @param out 文本流 @param path 文件路径(用于错误消息) @return 是否成功 */
bool DataExporter::flushAndCheck(QFile& file, QTextStream& out, const QString& path)
{
    out.flush();
    if (file.error() != QFile::NoError) {
        emit exportError(path, tr("写入文件失败: %1").arg(file.errorString()));
        file.close();
        return false;
    }
    file.close();
    return true;
}

/** @brief 按时间范围过滤终端行(支持单端/双端/无过滤) @param lines 原始行列表 @param from 起始时间(无效=无下界) @param to 结束时间(无效=无上界) @return 过滤后的行列表 */
QVector<TerminalLine> DataExporter::filterByTime(
    const QVector<TerminalLine>& lines, const QDateTime& from, const QDateTime& to) const
{
    bool hasFrom = from.isValid();
    bool hasTo = to.isValid();
    if (!hasFrom && !hasTo) return lines;

    QVector<TerminalLine> result;
    result.reserve(lines.size());
    for (const TerminalLine& line : lines) {
        if (hasFrom && line.timestamp < from) continue;
        if (hasTo && line.timestamp > to) continue;
        result.append(line);
    }
    return result;
}

/** @brief 写入CSV BOM头(如果启用) @param file 已打开的文件对象 @param path 文件路径(用于错误报告) @return true=BOM写入成功或不需要BOM */
bool DataExporter::writeCsvBom(QFile& file, const QString& path)
{
    if (!m_csvBomEnabled) return true;
    if (file.write("\xEF\xBB\xBF") != 3) {
        emit exportError(path, tr("写入BOM失败"));
        return false;
    }
    return true;
}

/** @brief 生成CSV表头行(使用当前配置的分隔符) @return 表头字符串(不含尾随换行) */
QString DataExporter::csvHeader() const
{
    return QString("timestamp%1direction%2data_hex%3data_ascii")
        .arg(m_csvDelimiter, m_csvDelimiter, m_csvDelimiter);
}

// 全量导出格式方法见 DataExporterFormats.cpp

// ---- 静态辅助方法 ----

/** @brief 将字节数据转换为可打印ASCII字符串(不可打印字符替换为'.') @param data 原始字节 @return ASCII字符串 */
QString DataExporter::toAsciiString(const QByteArray& data)
{
    if (data.isEmpty()) return QString();
    QString result;
    result.reserve(data.size());
    const char* ptr = data.constData();
    for (int i = 0; i < data.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(ptr[i]);
        result += (ch >= 0x20 && ch <= 0x7E) ? QLatin1Char(ch) : QLatin1Char('.');
    }
    return result;
}

/** @brief 转义CSV字段中的特殊字符(逗号/自定义分隔符、引号、换行) @param field 原始字段 @return 转义后的字段 */
QString DataExporter::escapeCsvField(const QString& field)
{
    // RFC 4180: 字段含分隔符、双引号或换行时，用双引号包裹，内部双引号翻倍
    // 注意: 始终检查逗号(RFC标准)和当前配置的分隔符
    if (!field.contains(QLatin1Char(',')) &&
        !field.contains(QLatin1Char('"')) &&
        !field.contains(QLatin1Char('\n')) &&
        !field.contains(QLatin1Char('\r')) &&
        !field.contains(QLatin1Char('\t'))) {
        return field;
    }
    QString escaped = field;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

/** @brief 将多行数据拼接为单个QByteArray(预分配总大小避免反复重分配) @param lines 终端行列表 @return 拼接后的原始字节 */
QByteArray DataExporter::concatData(const QVector<TerminalLine>& lines)
{
    qsizetype totalSize = 0;
    for (const TerminalLine& line : lines) totalSize += line.data.size();

    QByteArray result;
    result.reserve(totalSize);
    for (const TerminalLine& line : lines) result.append(line.data);
    return result;
}

/** @brief 格式化单行HEX转储(地址+HEX+ASCII) @param data 原始字节 @param address 起始地址 @return 格式化的HEX转储行 */
QString DataExporter::formatHexDumpLine(const QByteArray& data, quint64 address)
{
    const int bytesPerLine = 16;
    QString addrStr = QString("%1").arg(address, 8, 16, QChar('0')).toUpper();

    QString hexPart;
    hexPart.reserve(bytesPerLine * 3 + 2);
    for (int i = 0; i < bytesPerLine; ++i) {
        if (i > 0) hexPart += ' ';
        if (i == 8) hexPart += ' ';
        if (i < data.size()) {
            hexPart += QString("%1").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
        } else {
            hexPart += "  ";
        }
    }
    return QString("%1 | %2 | %3").arg(addrStr, hexPart, toAsciiString(data));
}

// 会话统计查询方法见 DataExporterStats.cpp

// exportPlain/exportHexDump/exportCsv/exportTimestamped/exportBin/exportJson
// 见 DataExporterFormats.cpp
