/**
 * @file DataExporter.h
 * @brief 数据导出器 - 将终端数据导出为 Plain/HexDump/CSV/Timestamped/Bin/Json 六种格式
 *
 * 支持:
 * - 六种导出格式(Plain/HexDump/CSV/Timestamped/Bin/Json)
 * - 全量导出(批量)、流式导出(分批)、EDL范围导出三种模式
 * - CSV的BOM头(Excel兼容)和可配置列分隔符
 * - 完整的会话统计(总次数/字节/行数/错误/各格式/总耗时/单次耗时)
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

    // ---- CSV配置 ----

    /** @brief 设置CSV列分隔符，默认逗号(',')，可设为制表符/分号等 @param delim 分隔符字符 */
    void setCsvDelimiter(QChar delim);

    /** @brief 获取当前CSV列分隔符 @return 分隔符字符 */
    QChar csvDelimiter() const;

    /** @brief 设置CSV是否写入UTF-8 BOM头(默认启用，确保Excel中文兼容) @param enable true=写入BOM */
    void setCsvBomEnabled(bool enable);

    /** @brief 获取CSV是否写入BOM头 @return true=启用BOM */
    bool isCsvBomEnabled() const;

    // ---- 会话统计 ----

    quint64 totalExports() const;        ///< 累计导出次数
    quint64 totalBytesExported() const;   ///< 累计导出字节
    quint64 totalRowsExported() const;    ///< 累计导出行数
    quint64 totalErrors() const;          ///< 累计失败次数
    quint64 totalCsvExports() const;      ///< CSV格式次数
    quint64 totalHexDumpExports() const;  ///< HexDump格式次数
    quint64 totalJsonExports() const;     ///< JSON格式次数
    quint64 totalBinExports() const;      ///< 二进制格式次数

    /** @brief 获取累计导出总耗时(仅统计成功操作) @return 总耗时毫秒数 */
    qint64 totalExportDurationMs() const;

    /** @brief 获取最近一次导出耗时 @return 最近导出耗时(ms)，未导出过返回0 */
    qint64 lastExportDurationMs() const;

    /** @brief 获取最近一次导出的行数 @return 最近导出行数 */
    quint64 lastExportRowCount() const;

    /** @brief 获取最近一次导出的字节数 @return 最近导出字节数 */
    quint64 lastExportByteCount() const;

    void resetStats();                    ///< 重置所有统计

signals:
    void exportError(const QString& filePath, const QString& errorString); ///< 导出失败信号

    /**
     * @brief 导出完成信号 — 每次成功导出后发射，携带本次操作的统计摘要
     * @param filePath 导出文件路径
     * @param format 导出格式
     * @param rowCount 导出行数
     * @param byteCount 导出字节数
     * @param durationMs 导出耗时(毫秒)
     */
    void exportCompleted(const QString& filePath, Format format,
                         quint64 rowCount, quint64 byteCount, qint64 durationMs);

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

    /** @brief 写入CSV BOM头(如果启用) @param file 已打开的文件对象 @param path 文件路径(错误报告) @return true=成功或不需要BOM */
    bool writeCsvBom(QFile& file, const QString& path);

    /** @brief 生成CSV表头行 @return 表头字符串(不含尾随换行) */
    QString csvHeader() const;

    static QString toAsciiString(const QByteArray& data);     ///< 不可打印→'.'
    static QString escapeCsvField(const QString& field);      ///< CSV转义
    static QByteArray concatData(const QVector<TerminalLine>& lines); ///< 拼接所有行数据
    static QString formatHexDumpLine(const QByteArray& data, quint64 addr); ///< HexDump格式化

    // EDL格式常量
    static constexpr const char* kEdlMagic = "EDL";
    static constexpr quint8 kEdlVersion = 1;
    static constexpr int kEdlHeaderSize = 8;
    static constexpr quint32 kEdlMaxRecordSize = 1024*1024;

    // 会话统计成员
    int m_lastExportRangeCount = 0;
    quint64 m_totalExports = 0;
    quint64 m_totalBytesExported = 0;
    quint64 m_totalRowsExported = 0;
    quint64 m_totalErrors = 0;
    quint64 m_totalCsvExports = 0;
    quint64 m_totalHexDumpExports = 0;
    quint64 m_totalJsonExports = 0;
    quint64 m_totalBinExports = 0;
    qint64 m_totalExportDurationMs = 0;   ///< 累计导出总耗时(毫秒)
    qint64 m_lastExportDurationMs = 0;    ///< 最近一次导出耗时(毫秒)
    quint64 m_lastExportRowCount = 0;     ///< 最近一次导出行数
    quint64 m_lastExportByteCount = 0;    ///< 最近一次导出字节数
    QElapsedTimer m_exportTimer;          ///< 当前导出操作计时器

    // CSV配置成员
    QChar m_csvDelimiter = QLatin1Char(',');  ///< CSV列分隔符
    bool m_csvBomEnabled = true;               ///< 是否写入UTF-8 BOM头
};

#endif // DATA_EXPORTER_H
