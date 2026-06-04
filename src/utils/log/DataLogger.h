/** @file DataLogger.h @brief 数据录制/回放管理器 -- 二进制日志文件(.edl)的读写和变速回放。管理串口/TCP数据流录制回放，支持暂停/恢复/变速/Seek/书签 */
#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "utils/data/DataBookmark.h"
#include <QObject>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>

/**
 * @brief 数据录制/回放管理器 -- 二进制日志文件(.edl)的读写和变速回放
 *
 * 使用自定义EDL格式(时间戳+方向+数据)，支持暂停/恢复/变速回放/Seek跳转/书签定位。
 * 所属层级: 数据层(纯文件I/O，不涉及UI)
 * 协作: RecordingController(上层控制器) / DataBookmark(书签管理)
 */
class DataLogger : public QObject {
    Q_OBJECT

public:
    enum class Direction : quint8 { Received = 0, Sent = 1 }; ///< RX/TX数据方向
    explicit DataLogger(QObject* parent = nullptr);
    ~DataLogger();
    // ---- 录制控制 ----
    bool startRecording(const QString& filePath); ///< 开始录制(打开文件写EDL头)
    void stopRecording();                         ///< 停止录制
    void pauseRecording();                        ///< 暂停录制
    void resumeRecording();                       ///< 恢复录制
    bool isRecording() const;                     ///< 是否正在录制
    bool isPaused() const;                        ///< 是否暂停
    void logData(const QByteArray& data, Direction dir); ///< 录制数据(外部调用)
    // ---- 回放控制 ----
    bool startPlayback(const QString& filePath);  ///< 开始回放(打开文件读EDL头)
    void stopPlayback();                          ///< 停止回放
    void pausePlayback();                         ///< 暂停回放
    void resumePlayback();                        ///< 恢复回放
    void setPlaybackSpeed(qreal speed);           ///< 设置回放速度倍率
    bool isPlaying() const;                       ///< 是否正在回放
    int recordCount() const;                      ///< 当前录制的记录数
    qint64 recordingDuration() const;             ///< 当前录制时长(ms)
    // ---- 跳转定位(Seek) ----
    bool seekToTimestamp(qint64 timestamp); ///< 跳转到指定时间戳(仅播放模式)
    bool seekToBookmark(int index);         ///< 跳转到指定书签(验证索引后委托seekToTimestamp)
    // ---- 书签管理 ----
    void addBookmark(const QString& label, const QString& streamId = QString()); ///< 在当前时间点添加书签
    QVector<DataBookmark> bookmarks() const; ///< 获取所有书签
    void removeBookmark(int index);          ///< 删除指定书签
    void clearBookmarks();                   ///< 清除所有书签
    // ---- 会话统计 ----
    quint64 totalLogsWritten() const;    ///< 累计写入的日志记录总数
    quint64 totalBookmarks() const;      ///< 累计添加的书签总数(含已删除)
    quint64 totalRecords() const;        ///< 累计录制的记录条数
    quint64 totalBytesRecorded() const;  ///< 累计录制的字节总数
    quint64 totalPlaybacks() const;      ///< 累计回放启动次数
    quint64 totalErrors() const;         ///< 累计发生的错误次数

    /** @brief 获取累计录制暂停次数 @return 暂停总数 */
    quint64 totalRecordingPauses() const { return m_totalRecordingPauses; }

    /** @brief 获取累计录制恢复次数 @return 恢复总数 */
    quint64 totalRecordingResumes() const { return m_totalRecordingResumes; }

    /** @brief 获取累计回放暂停次数 @return 暂停总数 */
    quint64 totalPlaybackPauses() const { return m_totalPlaybackPauses; }

    /** @brief 获取累计回放恢复次数 @return 恢复总数 */
    quint64 totalPlaybackResumes() const { return m_totalPlaybackResumes; }

    /** @brief 获取累计seek跳转操作次数 @return seek操作总数 */
    quint64 totalSeeks() const { return m_totalSeeks; }

    /** @brief 获取累计回放输出的字节总数 @return 回放字节总数 */
    quint64 totalBytesPlayedBack() const { return m_totalBytesPlayedBack; }

    /** @brief 获取累计回放速度变更次数 @return 速度变更次数 */
    quint64 totalSpeedChanges() const { return m_totalSpeedChanges; }

    /** @brief 获取累计录制启动次数 @return 录制启动次数 */
    quint64 totalRecordStarts() const { return m_totalRecordStarts; }

    void resetStats();                   ///< 重置所有会话统计计数器

signals:
    void recordingStarted();  ///< 录制已启动
    void recordingStopped(const QString& filePath, int recordCount, qint64 durationMs); ///< 录制已停止
    void playbackData(const QByteArray& data, qint64 direction);  ///< 回放输出一条记录
    void playbackProgress(qreal percent); ///< 回放进度(0.0~1.0)
    void playbackFinished();              ///< 回放结束
    void error(const QString& reason);    ///< 错误发生
    void bookmarksChanged();              ///< 书签列表变化(增/删/清空)
    void seekCompleted(qint64 timestamp); ///< seek操作完成

private slots:
    void onPlaybackTick();                 ///< 回放定时器回调

private:
    // EDL文件格式常量
    static constexpr const char* kMagic = "EDL";
    static constexpr quint8 kVersion = 1;
    static constexpr int kHeaderSize = 8;  // magic(3)+version(1)+padding(4)
    struct RecordHeader { quint64 timestamp; quint8 direction; quint32 length; };
    void writeHeader();
    void writeRecord(quint64 timestamp, Direction dir, const QByteArray& data);
    bool readNextRecord(RecordHeader& header, QByteArray& data);
    qint64 scanToTimestamp(qint64 targetTimestamp); ///< 线性扫描到目标时间戳
    // 录制相关
    QFile* m_recordFile = nullptr;
    QElapsedTimer m_recordTimer;
    qint64 m_pauseOffset = 0, m_pauseStartTime = 0;
    int m_recordCount = 0;
    bool m_recording = false, m_paused = false;
    // 回放相关
    QFile* m_playbackFile = nullptr;
    QTimer* m_playbackTimer = nullptr;
    QElapsedTimer m_playbackElapsed;
    qint64 m_playbackBaseTime = 0, m_nextRecordTime = 0, m_playbackOffset = 0;
    int m_playedRecords = 0;
    qreal m_playbackSpeed = 1.0;
    bool m_playing = false, m_playbackPaused = false;
    QMutex m_mutex;                        ///< 线程安全互斥锁
    QVector<DataBookmark> m_bookmarks;     ///< 书签列表
    // 会话统计
    quint64 m_totalLogsWritten = 0;   ///< 累计写入日志总数
    quint64 m_totalBookmarks = 0;     ///< 累计书签总数
    quint64 m_totalRecords = 0;       ///< 累计记录条数
    quint64 m_totalBytesRecorded = 0; ///< 累计字节总数
    quint64 m_totalPlaybacks = 0;     ///< 累计回放次数
    quint64 m_totalErrors = 0;        ///< 累计错误次数
    quint64 m_totalRecordingPauses = 0;  ///< 累计录制暂停次数
    quint64 m_totalRecordingResumes = 0; ///< 累计录制恢复次数
    quint64 m_totalPlaybackPauses = 0;   ///< 累计回放暂停次数
    quint64 m_totalPlaybackResumes = 0;  ///< 累计回放恢复次数
    quint64 m_totalSeeks = 0;            ///< 累计seek跳转操作次数
    quint64 m_totalBytesPlayedBack = 0;  ///< 累计回放输出的字节总数
    quint64 m_totalSpeedChanges = 0;     ///< 累计回放速度变更次数
    quint64 m_totalRecordStarts = 0;     ///< 累计录制启动次数
};

#endif // DATALOGGER_H
