/**
 * @file DataExporter.h
 * @brief 数据导出器 - 将终端数据导出为 Plain/HexDump/CSV/Timestamped/Bin/Json 六种格式
 *
 * 支持: 六种导出格式、全量导出(批量)/流式导出(分批)/EDL范围导出三种模式
 * CSV的BOM头(Excel兼容)和可配置列分隔符、完整会话统计
 */
#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <functional>
#include "terminal/types/TerminalTypes.h"

/// @brief 数据导出器(策略模式) — 协作: TerminalModel(数据源) / RecordingController(调用方)
class DataExporter : public QObject {
    Q_OBJECT

public:
    enum Format { Plain, HexDump, Csv, Timestamped, Bin, Json };
    using LineProvider = std::function<QVector<TerminalLine>(int offset, int count)>;

    explicit DataExporter(QObject* parent = nullptr);

    bool exportToFile(const QString& filePath, Format format, const QVector<TerminalLine>& lines,
                      const QDateTime& from = QDateTime(), const QDateTime& to = QDateTime());
    bool exportStreamed(const QString& filePath, Format format,
                        LineProvider lineProvider, int totalLines, int batchSize = 1000);
    bool exportRange(const QString& edlPath, Format format, const QString& outPath,
                     qint64 fromMs = -1, qint64 toMs = -1);
    int lastExportRangeCount() const;

    // ---- CSV配置 ----
    void setCsvDelimiter(QChar delim);
    QChar csvDelimiter() const;
    void setCsvBomEnabled(bool enable);
    bool isCsvBomEnabled() const;

    // ---- 会话统计 ----
    quint64 totalExports() const;
    quint64 totalBytesExported() const;
    quint64 totalRowsExported() const;
    quint64 totalErrors() const;
    quint64 totalCsvExports() const;
    quint64 totalHexDumpExports() const;
    quint64 totalJsonExports() const;
    quint64 totalBinExports() const;
    quint64 totalPlainExports() const;
    quint64 totalTimestampedExports() const;
    qint64 totalExportDurationMs() const;
    qint64 lastExportDurationMs() const;
    quint64 lastExportRowCount() const;
    quint64 lastExportByteCount() const;
    quint64 totalFilteredRows() const;
    quint64 totalEmptySkips() const;
    void resetStats();

signals:
    void exportError(const QString& filePath, const QString& errorString);
    void exportCompleted(const QString& filePath, Format format,
                         quint64 rowCount, quint64 byteCount, qint64 durationMs);

private:
    bool exportPlain(const QString& path, const QVector<TerminalLine>& lines);
    bool exportHexDump(const QString& path, const QVector<TerminalLine>& lines);
    bool exportCsv(const QString& path, const QVector<TerminalLine>& lines);
    bool exportTimestamped(const QString& path, const QVector<TerminalLine>& lines);
    bool exportBin(const QString& path, const QVector<TerminalLine>& lines);
    bool exportJson(const QString& path, const QVector<TerminalLine>& lines);
    bool exportStreamedPlain(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedHexDump(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedCsv(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedTimestamped(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedBin(const QString& path, LineProvider p, int total, int batch);
    bool exportStreamedJson(const QString& path, LineProvider p, int total, int batch);
    bool openTextFile(QFile& file, QTextStream& out, const QString& path);
    bool flushAndCheck(QFile& file, QTextStream& out, const QString& path);
    QVector<TerminalLine> filterByTime(const QVector<TerminalLine>& lines,
                                        const QDateTime& from, const QDateTime& to) const;
    QVector<TerminalLine> readEdlRange(const QString& edlPath, qint64 fromMs, qint64 toMs);
    bool writeCsvBom(QFile& file, const QString& path);
    QString csvHeader() const;
    static QString toAsciiString(const QByteArray& data);
    static QString escapeCsvField(const QString& field);
    static QByteArray concatData(const QVector<TerminalLine>& lines);
    static QString formatHexDumpLine(const QByteArray& data, quint64 addr);

    static constexpr const char* kEdlMagic = "EDL";
    static constexpr quint8 kEdlVersion = 1;
    static constexpr int kEdlHeaderSize = 8;
    static constexpr quint32 kEdlMaxRecordSize = 1024*1024;

    int m_lastExportRangeCount = 0;
    quint64 m_totalExports = 0, m_totalBytesExported = 0, m_totalRowsExported = 0, m_totalErrors = 0;
    quint64 m_totalCsvExports = 0, m_totalHexDumpExports = 0, m_totalJsonExports = 0;
    quint64 m_totalBinExports = 0, m_totalPlainExports = 0, m_totalTimestampedExports = 0;
    qint64 m_totalExportDurationMs = 0, m_lastExportDurationMs = 0;
    quint64 m_lastExportRowCount = 0, m_lastExportByteCount = 0;
    quint64 m_totalFilteredRows = 0, m_totalEmptySkips = 0;
    QElapsedTimer m_exportTimer;
    QChar m_csvDelimiter = QLatin1Char(',');
    bool m_csvBomEnabled = true;
};

#endif // DATA_EXPORTER_H
