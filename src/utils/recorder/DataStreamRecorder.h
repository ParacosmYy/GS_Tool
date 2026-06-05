/**
 * @file DataStreamRecorder.h
 * @brief 数据流录制器 -- 微秒精度时间戳的串口数据流捕获/标记/触发/导出
 *
 * 职责: 以微秒精度记录串口RX/TX双向数据，支持内存缓冲+文件流式双模式、
 *       命名标记/书签、时间范围提取、方向过滤、模式触发自动录制、多格式导出。
 *
 * 与 core/recording/DataStreamRecorder 的区别:
 *   - core版本: 文件导向的原始数据写入器(Raw/HexDump/CSV)
 *   - 本版本:   内存导向的流分析录制器(微秒时间戳/双向过滤/标记/触发/搜索)
 *
 * 协作: ConnectionController(连接数据源) / TerminalWidget(终端回放) / DataExporter(导出)
 */
#ifndef UTILS_RECORDER_DATASTREAMRECORDER_H
#define UTILS_RECORDER_DATASTREAMRECORDER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QFile>
#include <QList>
#include <QMutex>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

/**
 * @class DataStreamRecorder
 * @brief 数据流录制器 -- 微秒精度双向数据流捕获与分析
 *
 * 核心设计:
 *   - QElapsedTimer 提供微秒级相对时间戳，零系统调用开销
 *   - 内存缓冲模式: 适合短时录制+即时分析(搜索/过滤/导出)
 *   - 文件流式模式: 适合长时间录制，内存占用恒定
 *   - 模式触发: 检测到指定字节序列后自动开始/停止录制
 *   - 标记系统: 在录制期间插入命名书签，便于事后快速定位
 */
class DataStreamRecorder : public QObject {
    Q_OBJECT

public:
    /** @brief 数据方向 */
    enum class Direction {
        RX,  ///< 接收方向
        TX   ///< 发送方向
    };
    Q_ENUM(Direction)

    /** @brief 录制记录条目 */
    struct RecordEntry {
        quint64    timestampUs = 0;  ///< 微秒时间戳(相对录制起始)
        Direction  direction = Direction::RX;  ///< 数据方向
        QByteArray data;             ///< 原始数据
    };

    /** @brief 命名标记 */
    struct Marker {
        quint64    timestampUs = 0;  ///< 标记插入时刻的微秒时间戳
        QString    name;             ///< 标记名称
    };

    /** @brief 触发规则: 匹配到指定字节模式后自动开始/停止录制 */
    struct TriggerRule {
        QByteArray pattern;    ///< 匹配的字节模式
        bool       startOnMatch = true;  ///< true=匹配时开始录制, false=匹配时停止
    };

    /** @brief 导出格式 */
    enum class ExportFormat {
        Json,    ///< JSON数组格式
        Csv,     ///< CSV表格格式
        Binary   ///< 二进制紧凑格式
    };
    Q_ENUM(ExportFormat)

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalRecords       = 0;  ///< 已录制的总记录数
        quint64 totalBytesRecorded = 0;  ///< 已录制的总字节数
        quint64 totalRxRecords     = 0;  ///< RX方向记录数
        quint64 totalTxRecords     = 0;  ///< TX方向记录数
        quint64 totalMarkers       = 0;  ///< 已插入的标记总数
        double  recordingDurationMs = 0.0;  ///< 录制总时长(毫秒)
        quint64 avgRecordSize      = 0;  ///< 平均记录大小(字节)
        double  peakBytesPerSec    = 0.0;  ///< 峰值吞吐量(字节/秒)
    };

    // ── 构造/析构 ──

    /** @brief 构造数据流录制器 @param parent 父对象 */
    explicit DataStreamRecorder(QObject* parent = nullptr);

    /** @brief 析构，若仍在录制则自动停止 */
    ~DataStreamRecorder() override;

    // ── 录制控制 ──

    /**
     * @brief 开始内存缓冲模式录制
     *
     * 数据保存到内存缓冲区，可配置最大缓冲条目数。超出时自动丢弃最旧记录。
     * @param maxBufferSize 最大缓冲条目数，0=无限制
     */
    void start(quint64 maxBufferSize = 0);

    /**
     * @brief 开始文件流式模式录制
     *
     * 数据直接写入磁盘文件，内存占用恒定。适合长时间录制场景。
     * @param filePath 输出文件路径
     */
    void startStreaming(const QString& filePath);

    /** @brief 停止录制，更新统计 */
    void stop();

    /** @brief 查询是否正在录制 @return true=录制中 */
    bool isRecording() const;

    // ── 数据记录 ──

    /**
     * @brief 记录一帧数据(带微秒时间戳)
     * @param data 原始字节
     * @param direction 数据方向(RX/TX)
     */
    void recordData(const QByteArray& data, Direction direction);

    // ── 标记/书签 ──

    /**
     * @brief 在当前录制位置插入命名标记
     * @param name 标记名称
     */
    void insertMarker(const QString& name);

    /** @brief 获取所有标记 @return 标记列表 */
    QList<Marker> markers() const;

    // ── 查询/提取 ──

    /**
     * @brief 获取时间范围内的记录
     * @param startUs 起始微秒时间戳(含)
     * @param endUs 结束微秒时间戳(含)
     * @return 范围内的记录列表
     */
    QList<RecordEntry> getRecordsByTimeRange(quint64 startUs, quint64 endUs) const;

    /**
     * @brief 获取指定方向的记录
     * @param direction 数据方向
     * @return 该方向的所有记录列表
     */
    QList<RecordEntry> getRecordsByDirection(Direction direction) const;

    /**
     * @brief 获取所有内存缓冲区中的记录
     * @return 记录列表的副本
     */
    QList<RecordEntry> allRecords() const;

    /** @brief 获取当前缓冲区中的记录条数 @return 记录数量 */
    quint64 recordCount() const;

    // ── 搜索 ──

    /**
     * @brief 搜索包含指定字节模式的记录
     * @param pattern 要搜索的字节模式
     * @return 匹配的记录列表
     */
    QList<RecordEntry> search(const QByteArray& pattern) const;

    /**
     * @brief 搜索包含指定文本(正则)的记录
     * @param regex 正则表达式
     * @return 匹配的记录列表
     */
    QList<RecordEntry> searchByText(const QRegularExpression& regex) const;

    // ── 触发器 ──

    /**
     * @brief 设置触发规则(自动录制)
     *
     * 当数据流中出现匹配 pattern 的字节序列时，根据 startOnMatch 自动开始或停止录制。
     * @param rule 触发规则
     */
    void setTriggerRule(const TriggerRule& rule);

    /** @brief 清除触发规则，停止自动录制行为 */
    void clearTriggerRule();

    // ── 导出 ──

    /**
     * @brief 将缓冲区记录导出到文件
     * @param filePath 目标文件路径
     * @param format 导出格式(JSON/CSV/Binary)
     * @return true=导出成功
     */
    bool exportToFile(const QString& filePath, ExportFormat format) const;

    // ── 统计 ──

    /** @brief 获取统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 新记录已添加 @param timestamp 记录的微秒时间戳 */
    void recordAdded(quint64 timestamp);

    /** @brief 标记已插入 @param name 标记名称 */
    void markerInserted(const QString& name);

    /** @brief 缓冲区使用率变化 @param percentage 百分比(0.0~100.0) */
    void bufferUsageChanged(double percentage);

    /** @brief 录制已开始 */
    void recordingStarted();

    /** @brief 录制已停止 */
    void recordingStopped();

private:
    /**
     * @brief 检查触发规则是否匹配
     * @param data 待检查的数据
     */
    void checkTrigger(const QByteArray& data);

    /**
     * @brief 更新实时统计(吞吐量/平均记录大小等)
     * @param dataSize 本条记录的字节数
     */
    void updateLiveStats(int dataSize);

    /** @brief 计算并发射缓冲区使用率 */
    void emitBufferUsage();

    // ── 内部导出方法 ──
    bool exportJson(const QString& filePath) const;     ///< JSON格式导出
    bool exportCsv(const QString& filePath) const;      ///< CSV格式导出
    bool exportBinary(const QString& filePath) const;   ///< 二进制格式导出

    // ── 成员变量 ──
    bool               m_recording = false;     ///< 是否正在录制
    bool               m_streamingMode = false; ///< true=文件流式模式, false=内存缓冲模式
    QElapsedTimer      m_elapsedTimer;          ///< 微秒精度计时器
    quint64            m_maxBufferSize = 0;     ///< 最大缓冲条目数, 0=无限制

    // ── 内存缓冲区 ──
    QList<RecordEntry> m_buffer;                ///< 内存缓冲区(先进先出)
    mutable QMutex     m_bufferMutex;           ///< 缓冲区互斥锁

    // ── 文件流式 ──
    QFile              m_streamFile;            ///< 流式写入文件

    // ── 标记列表 ──
    QList<Marker>      m_markers;               ///< 已插入的标记列表

    // ── 触发器 ──
    TriggerRule        m_triggerRule;           ///< 当前触发规则
    bool               m_triggerActive = false; ///< 是否启用了触发器

    // ── 统计 ──
    Stats              m_stats;                 ///< 全局统计
    QElapsedTimer      m_statsTimer;            ///< 秒级吞吐量计算计时器
    quint64            m_statsWindowBytes = 0;  ///< 当前统计窗口字节数
};

#endif // UTILS_RECORDER_DATASTREAMRECORDER_H
