/** @file DataExporter.h @brief 数据导出器 - 将终端数据导出为 Plain/HexDump/CSV/Timestamped/Bin/Json 六种格式 */
#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <functional>
#include "terminal/types/TerminalTypes.h"

/** @brief 数据导出器(策略模式) — 协作: TerminalModel(数据源) / RecordingController(调用方) */
class DataExporter : public QObject {
    Q_OBJECT

public:
    /** @brief 导出格式 */
    enum Format { Plain, HexDump, Csv, Timestamped, Bin, Json };

    /** @brief 行数据回调: 从offset返回count条记录 */
    using LineProvider = std::function<QVector<TerminalLine>(int offset, int count)>;

    explicit DataExporter(QObject* parent = nullptr);

    /** @brief 全量导出(支持时间范围过滤) @param from/to 时间过滤(无效值=不限) */
    bool exportToFile(const QString& filePath, Format format,
                      const QVector<TerminalLine>& lines,
                      const QDateTime& from = QDateTime(), const QDateTime& to = QDateTime());
    /** @brief 流式导出(分批拉取，不支持时间过滤) */
    bool exportStreamed(const QString& filePath, Format format,
                        LineProvider lineProvider, int totalLines, int batchSize = 1000);
    /** @brief EDL范围导出(从录制文件提取指定时间范围) @param fromMs/toMs 毫秒偏移(-1=不限) */
    bool exportRange(const QString& edlPath, Format format, const QString& outPath,
                     qint64 fromMs = -1, qint64 toMs = -1);
    /** @brief 上次exportRange导出的记录数量 */
    int lastExportRangeCount() const;

    // ---- 统计 ----
    quint64 totalExports() const;        ///< 累计导出次数
    quint64 totalBytesExported() const;   ///< 累计导出字节
    quint64 totalRowsExported() const;    ///< 累计导出行数
    quint64 totalErrors() const;          ///< 累计失败次数
    quint64 totalCsvExports() const;      ///< CSV格式次数
    quint64 totalHexDumpExports() const;  ///< HexDump格式次数
    quint64 totalJsonExports() const;     ///< JSON格式次数
    quint64 totalBinExports() const;      ///< 二进制格式次数
    void resetStats();                    ///< 重置所有统计

signals:
    void exportError(const QString& filePath, const QString& errorString); ///< 导出失败信号

private:
    // 全量导出(按格式分发)
    bool exportPlain(const QString& path, const QVector<TerminalLine>& lines);
    bool exportHexDump(const QString& path, const QVector<TerminalLine>& lines);
    bool exportCsv(const QString& path, const QVector<TerminalLine>& lines);
    bool exportTimestamped(const QString& path, const QVector<TerminalLine>& lines);
    bool exportBin(const QString& path, const QVector<TerminalLine>& lines);
    bool exportJson(const QString& path, const QVector<TerminalLine>& lines);

    // 流式导出(按格式分发)
    bool exportStreamedPlain(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedHexDump(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedCsv(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedTimestamped(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedBin(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedJson(const QString& path, LineProvider p, int total, int batch);

    // 辅助
    bool openTextFile(QFile& file, QTextStream& out, const QString& path);
    bool flushAndCheck(QFile& file, QTextStream& out, const QString& path);
    QVector<TerminalLine> filterByTime(const QVector<TerminalLine>& lines,
                                        const QDateTime& from, const QDateTime& to) const;
    QVector<TerminalLine> readEdlRange(const QString& edlPath, qint64 fromMs, qint64 toMs);

    static QString toAsciiString(const QByteArray& data);     ///< 不可打印→'.'
    static QString escapeCsvField(const QString& field);      ///< CSV转义
    static QByteArray concatData(const QVector<TerminalLine>& lines); ///< 拼接所有行数据
    static QString formatHexDumpLine(const QByteArray& data, quint64 addr); ///< HexDump格式化

    // EDL格式常量
    static constexpr const char* kEdlMagic = "EDL";
    static constexpr quint8 kEdlVersion = 1;
    static constexpr int kEdlHeaderSize = 8;
    static constexpr quint32 kEdlMaxRecordSize = 1024*1024;

    // 成员
    int m_lastExportRangeCount = 0;
    quint64 m_totalExports = 0;
    quint64 m_totalBytesExported = 0;
    quint64 m_totalRowsExported = 0;
    quint64 m_totalErrors = 0;
    quint64 m_totalCsvExports = 0;
    quint64 m_totalHexDumpExports = 0;
    quint64 m_totalJsonExports = 0;
    quint64 m_totalBinExports = 0;
};

#endif // DATA_EXPORTER_H
