/**
 * @file DataExporter.h
 * @brief 数据导出器 - 将终端数据导出为 Plain/HexDump/CSV/Timestamped/Bin 五种格式
 *
 * 两种导出模式: exportToFile(全量+时间过滤) / exportStreamed(流式批量)
 * 设计模式: 策略模式简化实现（枚举 + switch 分发）
 */
#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <functional>
#include "terminal/TerminalTypes.h"

/**
 * @brief 数据导出器 - 支持多格式的终端数据导出
 *
 * 协作: TerminalModel(数据源) / HexConverter(HEX编码) / RecordingController(调用方)
 * 依赖方向: 基础设施层 <- 数据层(TerminalTypes)
 */
class DataExporter : public QObject {
    Q_OBJECT

public:
    /** @brief 导出格式: Plain(纯文本) / HexDump(地址|HEX|ASCII) / CSV(带表头) / Timestamped(带时间戳) / Bin(原始字节) / Json(JSON结构) */
    enum Format {
        Plain,       ///< 纯文本 - [时间戳] [方向] HEX | ASCII
        HexDump,     ///< 十六进制转储 - 地址|HEX|ASCII（经典格式，16字节/行）
        Csv,         ///< CSV - 带表头(timestamp,direction,data_hex,data_ascii)
        Timestamped, ///< 带时间戳 - 每行前缀精确时间戳 + HEX
        Bin,         ///< 二进制 - 仅原始字节
        Json         ///< JSON - 结构化JSON格式(含export_time/total_lines/lines数组)
    };

    /** @brief 行数据提供回调: 从 offset 开始返回 count 条记录，调用者负责线程安全 */
    using LineProvider = std::function<QVector<TerminalLine>(int offset, int count)>;

    explicit DataExporter(QObject* parent = nullptr);

    /**
     * @brief 全量导出 - 支持 from/to 时间范围过滤
     * @param filePath  输出文件路径
     * @param format    导出格式
     * @param lines     完整的行数据
     * @param from      起始时间过滤（无效值=不限制）
     * @param to        结束时间过滤（无效值=不限制）
     * @return true 成功，false 失败（空数据/文件无法打开）
     */
    bool exportToFile(const QString& filePath, Format format,
                      const QVector<TerminalLine>& lines,
                      const QDateTime& from = QDateTime(),
                      const QDateTime& to = QDateTime());

    /**
     * @brief 流式导出 - 分批拉取数据，不支持时间过滤
     * @param filePath      输出文件路径
     * @param format        导出格式
     * @param lineProvider  行数据回调
     * @param totalLines    数据总行数
     * @param batchSize     每批行数，默认 1000
     * @return true 成功，false 失败
     */
    bool exportStreamed(const QString& filePath, Format format,
                        LineProvider lineProvider,
                        int totalLines, int batchSize = 1000);

    /**
     * @brief EDL范围导出 - 从录制文件中提取指定时间范围的记录并导出
     * @param edlPath    EDL录制文件路径
     * @param format     导出格式
     * @param outPath    输出文件路径
     * @param fromMs     起始时间偏移（毫秒，距录制开始，-1=不限制）
     * @param toMs       结束时间偏移（毫秒，距录制开始，-1=不限制）
     * @return true 成功，false 失败（文件无法打开/无匹配数据）
     */
    bool exportRange(const QString& edlPath, Format format,
                     const QString& outPath,
                     qint64 fromMs = -1, qint64 toMs = -1);

    /**
     * @brief 获取上次exportRange调用导出的记录数量
     * @return 导出的记录条数，未调用过返回0
     */
    int lastExportRangeCount() const;

signals:
    /** @brief 导出失败信号 @param filePath 文件路径 @param errorString 错误描述 */
    void exportError(const QString& filePath, const QString& errorString);

private:
    // ---- 全量导出方法（按格式分发） ----

    /** @brief 纯文本导出: [时间戳] [方向] HEX | ASCII */
    bool exportPlain(const QString& path, const QVector<TerminalLine>& lines);
    /** @brief 十六进制转储导出: 地址 | HEX(16字节/行) | ASCII */
    bool exportHexDump(const QString& path, const QVector<TerminalLine>& lines);
    /** @brief CSV导出: 带表头，逗号分隔 */
    bool exportCsv(const QString& path, const QVector<TerminalLine>& lines);
    /** @brief 时间戳导出: 每行前缀精确时间戳 + HEX数据 */
    bool exportTimestamped(const QString& path, const QVector<TerminalLine>& lines);
    /** @brief 二进制导出: 仅原始字节 */
    bool exportBin(const QString& path, const QVector<TerminalLine>& lines);
    /** @brief JSON导出: 结构化JSON，含导出时间/总行数/每行数据(timestamp/direction/hex/ascii) */
    bool exportJson(const QString& path, const QVector<TerminalLine>& lines);

    // ---- 流式导出方法（按格式分发） ----

    /** @brief 流式纯文本导出 */
    bool exportStreamedPlain(const QString& path, LineProvider provider,
                             int totalLines, int batchSize);
    /** @brief 流式十六进制转储导出 */
    bool exportStreamedHexDump(const QString& path, LineProvider provider,
                               int totalLines, int batchSize);
    /** @brief 流式CSV导出 */
    bool exportStreamedCsv(const QString& path, LineProvider provider,
                           int totalLines, int batchSize);
    /** @brief 流式时间戳导出 */
    bool exportStreamedTimestamped(const QString& path, LineProvider provider,
                                   int totalLines, int batchSize);
    /** @brief 流式二进制导出 */
    bool exportStreamedBin(const QString& path, LineProvider provider,
                           int totalLines, int batchSize);
    /** @brief 流式JSON导出: 分批构建JSON数组，适合大数据量场景 */
    bool exportStreamedJson(const QString& path, LineProvider provider,
                            int totalLines, int batchSize);

    // ---- 辅助方法 ----

    /** @brief 打开文本文件并设置UTF8编码，失败时发射exportError */
    bool openTextFile(QFile& file, QTextStream& out, const QString& path);
    /** @brief 刷新文本流并检查文件写入错误，失败时发射 exportError 信号 */
    bool flushAndCheck(QFile& file, QTextStream& out, const QString& path);
    /** @brief 按时间范围过滤行数据，from/to 均可选 */
    QVector<TerminalLine> filterByTime(const QVector<TerminalLine>& lines,
                                        const QDateTime& from,
                                        const QDateTime& to) const;

    // ---- EDL文件格式常量（与DataLogger一致） ----
    static constexpr const char* kEdlMagic = "EDL";       ///< EDL文件魔数（3字节）
    static constexpr quint8 kEdlVersion = 1;              ///< EDL文件版本号
    static constexpr int kEdlHeaderSize = 8;              ///< 头部大小: magic(3)+version(1)+padding(4)
    static constexpr quint32 kEdlMaxRecordSize = 1024*1024; ///< 单条记录数据上限(1MB)，防御性校验

    /**
     * @brief 从EDL文件中读取指定时间范围的记录
     * @param edlPath  EDL文件路径
     * @param fromMs   起始时间偏移（毫秒，-1=不限制）
     * @param toMs     结束时间偏移（毫秒，-1=不限制）
     * @return 过滤后的TerminalLine列表，空列表表示无匹配或读取失败
     */
    QVector<TerminalLine> readEdlRange(const QString& edlPath,
                                        qint64 fromMs, qint64 toMs);

    /** @brief 不可打印字符替换为 '.' */
    static QString toAsciiString(const QByteArray& data);

    /** @brief CSV字段转义: 包含逗号/双引号/换行时用双引号包裹，内部双引号翻倍 */
    static QString escapeCsvField(const QString& field);

    /** @brief 拼接所有行数据为连续字节数组（HexDump用） */
    static QByteArray concatData(const QVector<TerminalLine>& lines);

    /** @brief 格式化单行HexDump: 地址 | HEX(16字节) | ASCII，不足16字节空格补齐 */
    static QString formatHexDumpLine(const QByteArray& data, quint64 address);

    // ---- 成员变量 ----

    int m_lastExportRangeCount = 0;  ///< 上次exportRange导出的记录数量
};

#endif // DATA_EXPORTER_H
