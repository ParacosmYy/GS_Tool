/**
 * @file DataLogger.h
 * @brief 数据录制/回放管理器 — 二进制日志文件的读写和回放接口
 */
#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "utils/data/DataBookmark.h"

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>

/**
 * @brief 数据录制/回放管理器 — 二进制日志文件(.edl)的读写和变速回放
 *
 * 管理串口/TCP数据流的录制和回放，使用自定义二进制格式(EDL)。
 * 录制时按时间戳+方向+数据记录，支持暂停/恢复。
 * 回放时按原始时间间隔逐条回放，支持变速、Seek跳转和书签定位。
 *
 * 协作关系:
 *   - RecordingController: 上层控制器，委托录制/回放交互逻辑
 *   - DataBookmark: 书签管理，支持在录制时间轴上标记关键节点
 *
 * 所属层级: 数据层（纯文件I/O，不涉及UI）
 */
class DataLogger : public QObject {
    Q_OBJECT

public:
    enum class Direction : quint8 {
        Received = 0,   // RX数据
        Sent = 1         // TX数据
    };

    explicit DataLogger(QObject* parent = nullptr);
    ~DataLogger();

    // ---- 录制控制 ----
    bool startRecording(const QString& filePath);
    void stopRecording();
    void pauseRecording();
    void resumeRecording();
    bool isRecording() const;
    bool isPaused() const;

    // 录制数据(由外部调用，传入方向和数据)
    void logData(const QByteArray& data, Direction dir);

    // ---- 回放控制 ----
    bool startPlayback(const QString& filePath);
    void stopPlayback();
    void pausePlayback();
    void resumePlayback();
    void setPlaybackSpeed(qreal speed);
    bool isPlaying() const;

    // 获取录制统计
    int recordCount() const;
    qint64 recordingDuration() const;

    // ---- 跳转定位(Seek) ----

    /**
     * @brief 跳转到指定时间戳位置（仅在播放模式下有效）
     *
     * 从文件头重新扫描所有记录，找到时间戳 <= timestamp 的最后一条记录，
     * 将播放位置调整到该记录之后。跳转完成后发射 seekCompleted() 信号。
     *
     * @param timestamp 目标时间戳（毫秒，相对于录制开始的偏移量）
     * @return true 跳转成功；false 不在播放模式或发生错误
     */
    bool seekToTimestamp(qint64 timestamp);

    /**
     * @brief 跳转到指定书签位置
     *
     * 验证 index 有效性后，调用 seekToTimestamp(bookmark.timestamp)。
     * 仅在播放模式下有效（由 seekToTimestamp 内部判断）。
     *
     * @param index 书签在 bookmarks() 列表中的索引，越界时返回 false
     * @return true 跳转成功；false 索引无效或不在播放模式
     */
    bool seekToBookmark(int index);

    // ---- 书签管理 ----

    /**
     * @brief 在当前时间点添加书签
     * 自动使用当前系统时间作为时间戳
     * @param label 书签标签文本
     * @param streamId 数据流标识（默认为空，表示全局书签）
     */
    void addBookmark(const QString& label, const QString& streamId = QString());

    /**
     * @brief 获取所有书签（按添加顺序）
     * @return 书签列表的只读引用
     */
    QVector<DataBookmark> bookmarks() const;

    /**
     * @brief 删除指定索引的书签
     * @param index 书签索引，越界时忽略
     */
    void removeBookmark(int index);

    /** @brief 清除所有书签 */
    void clearBookmarks();

signals:
    /** @brief 录制已启动 */
    void recordingStarted();
    /** @brief 录制已停止 @param filePath 录制文件路径 @param recordCount 总记录数 @param durationMs 录制时长(ms) */
    void recordingStopped(const QString& filePath, int recordCount, qint64 durationMs);
    /** @brief 回放输出一条记录数据 @param data 原始字节 @param direction 数据方向(RX/TX) */
    void playbackData(const QByteArray& data, qint64 direction);
    /** @brief 回放进度更新 @param percent 进度百分比 0.0~1.0 */
    void playbackProgress(qreal percent);
    /** @brief 回放结束 */
    void playbackFinished();
    /** @brief 错误发生 @param reason 错误原因描述 */
    void error(const QString& reason);

    /** @brief 书签列表变化信号（增/删/清空时发射） */
    void bookmarksChanged();

    /**
     * @brief seek操作完成信号
     * @param timestamp 实际跳转到的原始时间戳（可能不等于请求值，取最近匹配）
     */
    void seekCompleted(qint64 timestamp);

private slots:
    void onPlaybackTick();

private:
    // EDL文件格式常量
    static constexpr const char* kMagic = "EDL";
    static constexpr quint8 kVersion = 1;
    static constexpr int kHeaderSize = 8;  // magic(3) + version(1) + padding(4)

    // 录制状态
    struct RecordHeader {
        quint64 timestamp;   // 距录制开始的毫秒数
        quint8  direction;   // 0=RX, 1=TX
        quint32 length;      // 数据长度
    };

    void writeHeader();
    void writeRecord(quint64 timestamp, Direction dir, const QByteArray& data);
    bool readNextRecord(RecordHeader& header, QByteArray& data);

    /**
     * @brief 从文件头开始扫描，定位到目标时间戳最近的记录
     *
     * 遍历所有记录，找到 timestamp <= targetTimestamp 的最后一条。
     * 调用后文件指针位于该条记录之后，m_nextRecordTime 指向其下一条。
     *
     * @param targetTimestamp 目标时间戳（毫秒）
     * @return 实际定位到的时间戳；-1 表示无记录或文件错误
     */
    qint64 scanToTimestamp(qint64 targetTimestamp);

    // 录制相关
    QFile* m_recordFile = nullptr;
    QElapsedTimer m_recordTimer;
    qint64 m_pauseOffset = 0;       // 暂停期间的时间偏移
    qint64 m_pauseStartTime = 0;
    int m_recordCount = 0;
    bool m_recording = false;
    bool m_paused = false;

    // 回放相关
    QFile* m_playbackFile = nullptr;
    QTimer* m_playbackTimer = nullptr;
    QElapsedTimer m_playbackElapsed;
    qint64 m_playbackBaseTime = 0;  ///< 回放基准时间(累计已回放的原始时间)
    qint64 m_nextRecordTime = 0;    ///< 下一条记录的时间戳
    qint64 m_playbackOffset = 0;    ///< seek操作导致的时间偏移量（原始时间轴上的当前位置）
    int m_totalRecords = 0;
    int m_playedRecords = 0;
    qreal m_playbackSpeed = 1.0;
    bool m_playing = false;
    bool m_playbackPaused = false;

    /** @brief 线程安全互斥锁，保护 seek/录制/回放操作的原子性 */
    QMutex m_mutex;

    // 书签相关
    /** @brief 书签集合，按添加顺序存储 */
    QVector<DataBookmark> m_bookmarks;
};

#endif // DATALOGGER_H
