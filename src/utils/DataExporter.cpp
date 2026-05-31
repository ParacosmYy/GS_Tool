#include "utils/DataExporter.h"
#include "terminal/TerminalModel.h"
#include "core/Constants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

// 构造函数
DataExporter::DataExporter(QObject* parent)
    : QObject(parent)
{
}

// 导出数据到文件 - 统一入口
//
// 工作流程:
//   1. 检查数据是否为空
//   2. 按时间范围过滤（如果指定了有效的时间范围）
//   3. 根据格式调用对应的导出方法
//
// 参数说明见头文件
bool DataExporter::exportToFile(const QString& filePath, Format format,
                                 const QVector<TerminalLine>& lines,
                                 const QDateTime& from, const QDateTime& to)
{
    // 空数据检查 - 没有数据直接返回失败
    if (lines.isEmpty()) {
        return false;
    }

    // 文件路径检查
    if (filePath.isEmpty()) {
        return false;
    }

    // 按时间范围过滤数据
    QVector<TerminalLine> filtered = filterByTime(lines, from, to);

    // 过滤后可能没有数据（全部被排除）
    if (filtered.isEmpty()) {
        return false;
    }

    // 根据格式分派到具体的导出方法
    switch (format) {
    case Txt:
        return exportTxt(filePath, filtered);
    case Csv:
        return exportCsv(filePath, filtered);
    case Bin:
        return exportBin(filePath, filtered);
    }

    return false;
}

// ---- 私有方法实现 ----

// 按时间范围过滤数据行
//
// 过滤逻辑:
//   - from 有效: 只保留 timestamp >= from 的行
//   - to   有效: 只保留 timestamp <= to   的行
//   - 两者都无效: 返回原始数据（不做过滤）
//
// 注意: 使用浅拷贝（QByteArray 隐式共享），不会产生大量内存开销
QVector<TerminalLine> DataExporter::filterByTime(
    const QVector<TerminalLine>& lines,
    const QDateTime& from,
    const QDateTime& to) const
{
    // 没有时间过滤条件，直接返回原始数据
    bool hasFrom = from.isValid();
    bool hasTo = to.isValid();

    if (!hasFrom && !hasTo) {
        return lines;
    }

    QVector<TerminalLine> result;
    result.reserve(lines.size());   // 预分配空间，避免频繁扩容

    for (const TerminalLine& line : lines) {
        // 检查起始时间: timestamp 必须不早于 from
        if (hasFrom && line.timestamp < from) {
            continue;
        }
        // 检查结束时间: timestamp 必须不晚于 to
        if (hasTo && line.timestamp > to) {
            continue;
        }
        result.append(line);
    }

    return result;
}

// 导出为纯文本格式
//
// 每行格式:
//   [2024-01-15 14:30:25.123] [RX] 48 65 6C 6C 6F | Hello
//
// 包含:
//   - 时间戳（精确到毫秒）
//   - 数据方向（RX=接收 / TX=发送）
//   - 十六进制数据
//   - ASCII 可读文本（竖线分隔）
bool DataExporter::exportTxt(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    for (const TerminalLine& line : lines) {
        // 格式化时间戳: yyyy-MM-dd HH:mm:ss.zzz
        QString timeStr = line.timestamp.toString(
            QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));

        // 数据方向标识
        QString dirStr = (line.direction == DataDirection::Rx)
                             ? QStringLiteral("RX")
                             : QStringLiteral("TX");

        // 十六进制和 ASCII 表示
        QString hex = toHexString(line.data);
        QString ascii = toAsciiString(line.data);

        // 组装一行: [时间戳] [方向] HEX数据 | ASCII文本
        out << QStringLiteral("[%1] [%2] %3 | %4\n")
                   .arg(timeStr, dirStr, hex, ascii);
    }

    file.close();
    return true;
}

// 导出为 CSV 格式
//
// CSV 结构:
//   第一行是表头: timestamp,direction,data_hex,data_ascii
//   后续每行一条数据记录
//
// 字段说明:
//   timestamp    - ISO 格式时间戳（含毫秒）
//   direction    - RX 或 TX
//   data_hex     - 十六进制数据（空格分隔）
//   data_ascii   - ASCII 可读文本（不可打印字符替换为 .）
bool DataExporter::exportCsv(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // 写入 CSV 表头
    out << QStringLiteral("timestamp,direction,data_hex,data_ascii\n");

    for (const TerminalLine& line : lines) {
        // ISO 格式时间戳，保留毫秒精度
        QString timeStr = line.timestamp.toString(
            QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));

        // 数据方向
        QString dirStr = (line.direction == DataDirection::Rx)
                             ? QStringLiteral("RX")
                             : QStringLiteral("TX");

        // 十六进制和 ASCII
        QString hex = toHexString(line.data);
        QString ascii = toAsciiString(line.data);

        // 写入一行 CSV 数据
        // ASCII 字段用双引号包裹，防止内部逗号/引号破坏 CSV 结构
        out << timeStr << QLatin1Char(',')
            << dirStr << QLatin1Char(',')
            << hex << QLatin1Char(',')
            << QLatin1Char('"') << ascii << QLatin1Char('"')
            << QLatin1Char('\n');
    }

    file.close();
    return true;
}

// 导出为二进制格式
//
// 仅写入原始字节数据，丢弃所有元信息（时间戳、方向）。
// 适用于:
//   - 将通信数据还原为原始二进制文件
//   - 后续用 hex 编辑器或程序解析
//   - 保存固件/镜像等纯二进制数据
bool DataExporter::exportBin(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    // 逐行写入原始字节数据
    for (const TerminalLine& line : lines) {
        file.write(line.data);
    }

    file.close();
    return true;
}

// 将字节数组转换为十六进制字符串
//
// 示例: {0x48, 0x65, 0x6C, 0x6C, 0x6F} -> "48 65 6C 6C 6F"
//
// 每个字节转为两位十六进制，字节之间用空格分隔。
// 大写十六进制字母（A-F），与嵌入式开发习惯一致。
QString DataExporter::toHexString(const QByteArray& data)
{
    if (data.isEmpty()) {
        return QString();
    }

    // 预分配: 每个字节占 3 个字符（2位hex + 1个空格），最后一字节不需要空格
    QString result;
    result.reserve(data.size() * 3 - 1);

    const char* ptr = data.constData();
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) {
            result += QLatin1Char(' ');
        }
        // toHex() 返回 "0x" 前缀格式，这里用 sprintf 风格确保两位大写
        result += QString::asprintf("%02X",
                                    static_cast<unsigned char>(ptr[i]));
    }

    return result;
}

// 将字节数组转换为可打印 ASCII 字符串
//
// 规则:
//   - 可打印 ASCII 字符（0x20 ~ 0x7E）原样保留
//   - 其他所有字节（控制字符、高位字节）替换为 '.'
//
// 示例: {0x48, 0x65, 0x00, 0x6C, 0x6C} -> "He..ll"
QString DataExporter::toAsciiString(const QByteArray& data)
{
    if (data.isEmpty()) {
        return QString();
    }

    QString result;
    result.reserve(data.size());

    const char* ptr = data.constData();
    for (int i = 0; i < data.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(ptr[i]);
        // 可打印 ASCII 范围: 空格(0x20) 到 波浪号(0x7E)
        if (ch >= 0x20 && ch <= 0x7E) {
            result += QLatin1Char(ch);
        } else {
            result += QLatin1Char('.');
        }
    }

    return result;
}
