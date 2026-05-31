#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <functional>
#include "terminal/TerminalTypes.h"

// 数据导出器 - 将终端数据导出为不同格式文件
// 支持 TXT/CSV/BIN 三种格式，支持按时间范围过滤
//
// 两种导出模式:
// 1. exportToFile: 全量导出，调用者传入完整的 QVector<TerminalLine>，适合数据量小的场景
// 2. exportStreamed: 批量流式导出，通过 lineProvider 分批拉取数据，避免对全部数据做深拷贝
class DataExporter : public QObject {
    Q_OBJECT

public:
    enum Format {
        Txt,    // 纯文本 - [时间戳] [方向] HEX | ASCII
        Csv,    // CSV - 带表头，可被Excel/pandas读取
        Bin     // 二进制 - 仅原始字节
    };

    // 行数据提供回调: 返回从 offset 开始的 count 条记录
    // 调用者负责线程安全和数据生命周期
    using LineProvider = std::function<QVector<TerminalLine>(int offset, int count)>;

    explicit DataExporter(QObject* parent = nullptr);

    // 全量导出: 调用者传入完整的行数据，内部可选按时间范围过滤
    // 适合数据量小（< 5000 行）或已有全量数据的场景
    bool exportToFile(const QString& filePath, Format format,
                      const QVector<TerminalLine>& lines,
                      const QDateTime& from = QDateTime(),
                      const QDateTime& to = QDateTime());

    // 批量流式导出: 通过 lineProvider 分批拉取数据，避免一次性深拷贝全部行
    // totalLines: 数据总行数，用于预计算和进度报告
    // batchSize: 每批拉取的行数，默认 1000
    // 注意: 不支持时间范围过滤，因为无法在不加载全部数据的情况下高效过滤
    //       如需时间过滤，请使用 exportToFile 并传入完整数据
    bool exportStreamed(const QString& filePath, Format format,
                        LineProvider lineProvider,
                        int totalLines, int batchSize = 1000);

private:
    bool exportTxt(const QString& path, const QVector<TerminalLine>& lines);
    bool exportCsv(const QString& path, const QVector<TerminalLine>& lines);
    bool exportBin(const QString& path, const QVector<TerminalLine>& lines);

    // 流式写入: 打开文件后逐批拉取并写入，避免持有全部数据
    bool exportStreamedTxt(const QString& path, LineProvider provider,
                           int totalLines, int batchSize);
    bool exportStreamedCsv(const QString& path, LineProvider provider,
                           int totalLines, int batchSize);
    bool exportStreamedBin(const QString& path, LineProvider provider,
                           int totalLines, int batchSize);

    QVector<TerminalLine> filterByTime(const QVector<TerminalLine>& lines,
                                        const QDateTime& from,
                                        const QDateTime& to) const;

    // 将字节数组转换为可打印 ASCII 字符串
    // 不可打印字符替换为 '.'
    static QString toAsciiString(const QByteArray& data);
};

#endif // DATA_EXPORTER_H
