#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include "terminal/TerminalTypes.h"

// 数据导出器 - 将终端数据导出为不同格式文件
// 支持 TXT/CSV/BIN 三种格式，支持按时间范围过滤
class DataExporter : public QObject {
    Q_OBJECT

public:
    enum Format {
        Txt,    // 纯文本 - [时间戳] [方向] HEX | ASCII
        Csv,    // CSV - 带表头，可被Excel/pandas读取
        Bin     // 二进制 - 仅原始字节
    };

    explicit DataExporter(QObject* parent = nullptr);

    bool exportToFile(const QString& filePath, Format format,
                      const QVector<TerminalLine>& lines,
                      const QDateTime& from = QDateTime(),
                      const QDateTime& to = QDateTime());

private:
    bool exportTxt(const QString& path, const QVector<TerminalLine>& lines);
    bool exportCsv(const QString& path, const QVector<TerminalLine>& lines);
    bool exportBin(const QString& path, const QVector<TerminalLine>& lines);

    QVector<TerminalLine> filterByTime(const QVector<TerminalLine>& lines,
                                        const QDateTime& from,
                                        const QDateTime& to) const;

    // 将字节数组转换为可打印 ASCII 字符串
    // 不可打印字符替换为 '.'
    static QString toAsciiString(const QByteArray& data);
};

#endif // DATA_EXPORTER_H
