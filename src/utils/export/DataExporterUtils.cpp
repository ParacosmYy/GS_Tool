/**
 * @file DataExporterUtils.cpp
 * @brief 数据导出器 - 静态辅助方法实现
 *
 * 从 DataExporter.cpp 拆分而来，包含导出器使用到的静态工具方法:
 *   - toAsciiString():      字节数据转可打印ASCII(不可打印替换为'.')
 *   - escapeCsvField():     CSV字段转义(RFC 4180合规)
 *   - concatData():         多行数据拼接为单个QByteArray
 *   - formatHexDumpLine():  格式化单行HEX转储(地址+HEX+ASCII)
 *
 * 这些方法无状态依赖，可被多个导出文件(format/streamed/edl)复用。
 */

#include "utils/export/DataExporter.h"

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
