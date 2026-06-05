/**
 * @file StreamCaptureRecorder.h
 * @brief 流捕获录制器 -- 微秒精度时间戳的串口数据流捕获/标记/触发/导出
 *
 * 职责: 微秒精度RX/TX双向记录、内存缓冲+文件流式双模式、命名标记/书签、
 *       时间范围提取、方向过滤、模式触发自动录制、JSON/CSV/Binary导出。
 *
 * 与 core/recording/StreamCaptureRecorder 的区别:
 *   - core版本: 文件导向原始数据写入器(Raw/HexDump/CSV)
 *   - 本版本:   内存导向流分析录制器(微秒时间戳/双向过滤/标记/触发/搜索)
 *
 * 协作: ConnectionController(数据源) / TerminalWidget(回放) / DataExporter(导出)
 */
#ifndef UTILS_RECORDER_STREAMCAPTURERECORDER_H
#define UTILS_RECORDER_STREAMCAPTURERECORDER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QFile>
#include <QList>
#include <QMutex>
#include <QRegularExpression>
#include <QStringList>

/**
 * @class StreamCaptureRecorder
 * @brief 微秒精度双向流捕获录制器
 *
 * 核心: QElapsedTimer微秒时间戳 / 内存缓冲+文件流式双模式 /
 *       模式触发自动录制 / 命名标记书签 / 多格式导出
 */
class StreamCaptureRecorder : public QObject {
    Q_OBJECT

public:
    /** @brief 数据方向 */
    enum class Direction { RX, TX };
    Q_ENUM(Direction)

    /** @brief 录制记录条目 */
    struct RecordEntry {
        quint64    timestampUs = 0;       ///< 微秒时间戳(相对录制起始)
        Direction  direction = Direction::RX; ///< 数据方向
        QByteArray data;                  ///< 原始数据
    };

    /** @brief 命名标记 */
    struct Marker {
        quint64 timestampUs = 0;          ///< 标记时刻微秒时间戳
        QString name;                     ///< 标记名称
    };

    /** @brief 触发规则: 匹配字节模式后自动开始/停止 */
    struct TriggerRule {
        QByteArray pattern;               ///< 匹配字节模式
        bool startOnMatch = true;         ///< true=匹配开始, false=匹配停止
    };

    /** @brief 导出格式 */
    enum class ExportFormat { Json, Csv, Binary };
    Q_ENUM(ExportFormat)

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalRecords       = 0;   ///< 总记录数
        quint64 totalBytesRecorded = 0;   ///< 总字节数
        quint64 totalRxRecords     = 0;   ///< RX记录数
        quint64 totalTxRecords     = 0;   ///< TX记录数
        quint64 totalMarkers       = 0;   ///< 标记总数
        double  recordingDurationMs = 0.0; ///< 录制时长(ms)
        quint64 avgRecordSize      = 0;   ///< 平均记录大小
        double  peakBytesPerSec    = 0.0; ///< 峰值吞吐量(B/s)
    };

    // ── 构造/析构 ──
    explicit StreamCaptureRecorder(QObject* parent = nullptr);
    ~StreamCaptureRecorder() override;

    // ── 录制控制 ──

    /** @brief 开始内存缓冲模式录制 @param maxBufferSize 最大条目数, 0=无限制 */
    void start(quint64 maxBufferSize = 0);

    /** @brief 开始文件流式模式录制 @param filePath 输出文件路径 */
    void startStreaming(const QString& filePath);

    /** @brief 停止录制 */
    void stop();

    /** @brief 是否正在录制 */
    bool isRecording() const;

    // ── 数据记录 ──

    /** @brief 记录一帧数据 @param data 原始字节 @param direction 方向 */
    void recordData(const QByteArray& data, Direction direction);

    // ── 标记 ──

    /** @brief 插入命名标记 @param name 标记名称 */
    void insertMarker(const QString& name);

    /** @brief 获取所有标记 */
    QList<Marker> markers() const;

    // ── 查询/提取 ──

    /** @brief 按时间范围获取记录 [startUs, endUs] */
    QList<RecordEntry> getRecordsByTimeRange(quint64 startUs, quint64 endUs) const;

    /** @brief 按方向获取记录 */
    QList<RecordEntry> getRecordsByDirection(Direction direction) const;

    /** @brief 获取所有内存记录的副本 */
    QList<RecordEntry> allRecords() const;

    /** @brief 缓冲区记录条数 */
    quint64 recordCount() const;

    // ── 搜索 ──

    /** @brief 搜索包含指定字节模式的记录 */
    QList<RecordEntry> search(const QByteArray& pattern) const;

    /** @brief 搜索包含指定正则文本的记录 */
    QList<RecordEntry> searchByText(const QRegularExpression& regex) const;

    // ── 触发器 ──

    /** @brief 设置触发规则(自动录制) */
    void setTriggerRule(const TriggerRule& rule);

    /** @brief 清除触发规则 */
    void clearTriggerRule();

    // ── 导出 ──

    /** @brief 导出缓冲区记录到文件 @param format JSON/CSV/Binary */
    bool exportToFile(const QString& filePath, ExportFormat format) const;

    // ── 统计 ──

    /** @brief 获取统计快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    void recordAdded(quint64 timestamp);       ///< 新记录已添加
    void markerInserted(const QString& name);  ///< 标记已插入
    void bufferUsageChanged(double percentage); ///< 缓冲区使用率变化(0~100)
    void recordingStarted();                   ///< 录制已开始
    void recordingStopped();                   ///< 录制已停止

private:
    void checkTrigger(const QByteArray& data); ///< 检查触发规则
    void updateLiveStats(int dataSize);        ///< 更新实时统计
    void emitBufferUsage();                    ///< 计算并发射缓冲区使用率

    // 内部导出
    bool exportJson(const QString& filePath) const;
    bool exportCsv(const QString& filePath) const;
    bool exportBinary(const QString& filePath) const;

    // ── 成员变量 ──
    bool               m_recording = false;     ///< 是否正在录制
    bool               m_streamingMode = false; ///< true=文件流式, false=内存缓冲
    QElapsedTimer      m_elapsedTimer;          ///< 微秒精度计时器
    quint64            m_maxBufferSize = 0;     ///< 最大缓冲条目数, 0=无限制

    QList<RecordEntry> m_buffer;                ///< 内存缓冲区(FIFO)
    mutable QMutex     m_bufferMutex;           ///< 缓冲区互斥锁

    QFile              m_streamFile;            ///< 流式写入文件
    QList<Marker>      m_markers;               ///< 标记列表

    TriggerRule        m_triggerRule;           ///< 触发规则
    bool               m_triggerActive = false; ///< 触发器启用标志

    Stats              m_stats;                 ///< 全局统计
    QElapsedTimer      m_statsTimer;            ///< 吞吐量计算计时器
    quint64            m_statsWindowBytes = 0;  ///< 统计窗口字节数
};

#endif // UTILS_RECORDER_STREAMCAPTURERECORDER_H
