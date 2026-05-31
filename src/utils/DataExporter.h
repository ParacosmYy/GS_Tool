#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <QObject>
#include <QVector>
#include <QDateTime>

struct TerminalLine;

// 数据导出器 - 将终端数据导出为不同格式文件
//
// 设计说明:
//   负责将 TerminalModel 中的 TerminalLine 数据导出为外部文件。
//   支持三种格式:
//     - TXT: 纯文本格式，方便阅读和搜索
//     - CSV: 逗号分隔格式，方便导入 Excel 等工具分析
//     - BIN: 原始二进制数据，方便后续程序处理
//
//   支持按时间范围过滤导出数据，只导出指定时间段内的记录。
//   当 from/to 参数为无效 QDateTime 时，表示不做时间过滤，导出全部数据。
//
// 使用示例:
//   DataExporter exporter;
//   exporter.exportToFile("log.txt", DataExporter::Txt, lines);
//   exporter.exportToFile("log.csv", DataExporter::Csv, lines,
//                         startTime, endTime);
class DataExporter : public QObject {
    Q_OBJECT

public:
    // 导出格式枚举
    enum Format {
        Txt,    // 纯文本格式 - 每行一条记录，包含时间戳、方向、数据
        Csv,    // CSV格式 - 带表头，可被 Excel/Python pandas 读取
        Bin     // 二进制格式 - 仅导出原始数据字节，不含时间戳和方向
    };

    explicit DataExporter(QObject* parent = nullptr);

    // 导出数据到文件
    //
    // 参数:
    //   filePath - 目标文件路径（完整路径含文件名）
    //   format   - 导出格式（Txt/Csv/Bin）
    //   lines    - 要导出的终端数据行列表
    //   from     - 起始时间过滤（无效时间表示不限制起始）
    //   to       - 结束时间过滤（无效时间表示不限制结束）
    //
    // 返回:
    //   true  - 导出成功
    //   false - 导出失败（文件无法打开、无数据等）
    bool exportToFile(const QString& filePath, Format format,
                      const QVector<TerminalLine>& lines,
                      const QDateTime& from = QDateTime(),
                      const QDateTime& to = QDateTime());

private:
    // 导出为纯文本格式
    // 格式: [2024-01-15 14:30:25.123] [RX] 48 65 6C 6C 6F
    bool exportTxt(const QString& path, const QVector<TerminalLine>& lines);

    // 导出为CSV格式
    // 包含表头: timestamp,direction,data_hex,data_ascii
    bool exportCsv(const QString& path, const QVector<TerminalLine>& lines);

    // 导出为二进制格式
    // 仅写入原始字节数据，丢弃时间戳和方向信息
    bool exportBin(const QString& path, const QVector<TerminalLine>& lines);

    // 根据时间范围过滤数据行
    // from/to 为无效时间时跳过对应的时间边界检查
    QVector<TerminalLine> filterByTime(const QVector<TerminalLine>& lines,
                                        const QDateTime& from,
                                        const QDateTime& to) const;

    // 将字节数组转换为十六进制字符串（空格分隔）
    // 例如: {0x48, 0x65, 0x6C} -> "48 65 6C"
    static QString toHexString(const QByteArray& data);

    // 将字节数组转换为可打印 ASCII 字符串
    // 不可打印字符替换为 '.' (0x2E)
    static QString toAsciiString(const QByteArray& data);
};

#endif // DATA_EXPORTER_H
