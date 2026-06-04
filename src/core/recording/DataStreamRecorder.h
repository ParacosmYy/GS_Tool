/**
 * @file DataStreamRecorder.h
 * @brief 数据流记录器 -- 原始数据流捕获/标注/多格式保存
 *
 * 职责: 将原始串口数据以最小开销写入磁盘 / 支持3种输出格式(原始二进制/HEX转储/时间戳CSV) /
 *       精确时间戳+方向标记 / 用户标注嵌入 / 文件大小达到上限自动分割
 *
 * 与 RecordingController 的区别:
 *   - RecordingController: 高层录制/回放管理器，委托DataLogger处理
 *   - DataStreamRecorder:  底层原始数据捕获，面向长时间数据记录和分析场景
 *
 * 协作: ConnectionController(连接数据源) / MainWindow(UI触发)
 */
#ifndef DATASTREAMRECORDER_H
#define DATASTREAMRECORDER_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QElapsedTimer>
#include <QMutex>

/**
 * @brief 数据流记录器 - 轻量级原始数据捕获+标注+自动分割
 *
 * 设计要点:
 *   - recordData() 无锁快速路径，适合高频串口数据流
 *   - 自动分割: 达到 maxFileSizeMB 自动切换新文件，序号递增
 *   - 标注系统: addAnnotation() / addMarker() 嵌入到输出流中
 *   - 统计追踪: 累计字节数/包数/分割次数/峰值吞吐量
 */
class DataStreamRecorder : public QObject {
    Q_OBJECT

public:
    /** @brief 输出文件格式 */
    enum class FileFormat {
        Raw,              ///< 原始二进制(.bin)，无时间戳无方向
        HexDump,          ///< HEX转储(.txt)，每行16字节+ASCII
        TimestampedCsv    ///< 时间戳CSV(.csv)，timestamp,dir,size,hex
    };
    Q_ENUM(FileFormat)

    /** @brief 录制配置参数 */
    struct RecordingConfig {
        FileFormat format = FileFormat::TimestampedCsv;  ///< 输出格式
        bool includeTimestamps = true;                   ///< 是否包含时间戳
        bool includeDirection = true;                    ///< 是否包含方向标记(TX/RX)
        int maxFileSizeMB = 100;                         ///< 单文件最大容量(MB)，0=不限制
        bool autoSplit = true;                           ///< 达到上限时自动分割
        QString filePath;                                ///< 输出文件路径(不含序号后缀)
    };

    /** @brief 当前录制会话统计 */
    struct RecordingStats {
        quint64 totalBytesRecorded = 0;    ///< 本次会话已录制字节总数
        quint64 totalPacketsRecorded = 0;  ///< 本次会话已录制数据包总数
        quint64 totalAnnotations = 0;      ///< 本次会话已添加标注总数
        quint64 totalSplits = 0;           ///< 本次会话文件分割次数
        quint64 totalStarts = 0;           ///< 本次会话 start() 调用次数
        quint64 totalStops = 0;            ///< 本次会话 stop() 调用次数
        double recordingDurationSec = 0.0; ///< 本次会话已录制时长(秒)
        double avgBytesPerSec = 0.0;       ///< 本次会话平均吞吐量(B/s)
        quint64 currentFileSize = 0;       ///< 当前文件大小(字节)
    };

    /** @brief 累计全局统计 */
    struct Stats {
        quint64 totalRecordingSessions = 0;        ///< 累计录制会话数
        quint64 totalBytesWritten = 0;              ///< 累计写入字节总数
        quint64 totalFilesCreated = 0;              ///< 累计创建文件总数
        double totalRecordingTimeSec = 0.0;         ///< 累计录制时长(秒)
        double peakThroughputBytesPerSec = 0.0;     ///< 历史峰值吞吐量(B/s)
    };

    // ---- 构造 ----

    /** @brief 构造数据流记录器 @param parent 父对象 */
    explicit DataStreamRecorder(QObject* parent = nullptr);

    /** @brief 析构，若仍在录制则自动停止 */
    ~DataStreamRecorder();

    // ---- 录制控制 ----

    /** @brief 开始录制 @param config 录制配置 @return true=启动成功 */
    bool start(const RecordingConfig& config);

    /** @brief 停止录制，刷新并关闭文件 */
    void stop();

    /** @brief 查询是否正在录制 @return true=录制中 */
    bool isRecording() const;

    // ---- 数据写入 ----

    /** @brief 录制一帧数据 @param data 原始数据 @param isReceived true=接收(RX), false=发送(TX) */
    void recordData(const QByteArray& data, bool isReceived);

    /** @brief 添加文本标注(嵌入到输出流) @param text 标注内容 */
    void addAnnotation(const QString& text);

    /** @brief 添加位置标记(带时间戳的书签) @param label 标记标签 */
    void addMarker(const QString& label);

    // ---- 配置/状态查询 ----

    /** @brief 获取当前录制配置 @return 配置副本 */
    RecordingConfig config() const;

    /** @brief 获取当前录制会话统计 @return 统计快照 */
    RecordingStats recordingStats() const;

    /** @brief 获取累计全局统计(只读引用) @return 统计常量引用 */
    const Stats& stats() const;

    /** @brief 重置累计全局统计计数器 */
    void resetStatistics();

signals:
    /** @brief 录制已启动 @param filePath 第一个文件路径 */
    void recordingStarted(const QString& filePath);

    /** @brief 录制已停止 @param totalBytes 本次会话总字节数 */
    void recordingStopped(qint64 totalBytes);

    /** @brief 数据已记录 @param bytes 本帧字节数 */
    void dataRecorded(int bytes);

    /** @brief 标注已添加 @param text 标注文本 */
    void annotationAdded(const QString& text);

    /** @brief 文件大小达到上限 @param path 当前文件路径 */
    void fileSizeLimitReached(const QString& path);

    /** @brief 发生错误 @param message 错误信息 */
    void errorOccurred(const QString& message);

private:
    // ---- 内部写入方法 ----
    void writeHeader();   ///< 写入文件头(格式标识/配置元数据)
    void writeRaw(const QByteArray& data, bool isReceived);        ///< 原始二进制写入
    void writeTimestamped(const QByteArray& data, bool isReceived); ///< 时间戳CSV写入
    void writeHexDump(const QByteArray& data, bool isReceived);     ///< HEX转储写入
    bool splitFile();      ///< 分割到新文件
    QString generateFilePath();  ///< 生成带序号的文件路径

    // ---- 成员变量 ----
    RecordingConfig m_config;          ///< 当前录制配置
    QFile m_file;                      ///< 当前输出文件
    QDataStream m_stream;              ///< 数据流(用于Raw格式的二进制写入)
    bool m_recording = false;          ///< 是否正在录制
    QElapsedTimer m_recordingTimer;    ///< 录制会话计时器
    qint64 m_sessionBytes = 0;         ///< 当前会话已写入字节
    qint64 m_sessionPackets = 0;       ///< 当前会话已写入数据包数
    int m_fileIndex = 0;               ///< 当前文件序号(用于分割)
    RecordingStats m_recordingStats;   ///< 当前录制会话统计
    Stats m_stats;                     ///< 累计全局统计
};

#endif // DATASTREAMRECORDER_H
