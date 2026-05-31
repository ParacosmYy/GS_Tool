#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "utils/DataBookmark.h"

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>

// 数据日志记录器 - 录制和回放串口/TCP数据流
// 二进制格式(.edl): Header + Records(timestamp + direction + data)
// 支持暂停/恢复录制，支持变速回放
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
    void recordingStarted();
    void recordingStopped(const QString& filePath, int recordCount, qint64 durationMs);
    void playbackData(const QByteArray& data, qint64 direction);
    void playbackProgress(qreal percent);
    void playbackFinished();
    void error(const QString& reason);

    /** @brief 书签列表变化信号（增/删/清空时发射） */
    void bookmarksChanged();

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
    qint64 m_playbackBaseTime = 0;  // 回放基准时间(累计已回放的原始时间)
    qint64 m_nextRecordTime = 0;    // 下一条记录的时间戳
    int m_totalRecords = 0;
    int m_playedRecords = 0;
    qreal m_playbackSpeed = 1.0;
    bool m_playing = false;
    bool m_playbackPaused = false;

    // 书签相关
    /** @brief 书签集合，按添加顺序存储 */
    QVector<DataBookmark> m_bookmarks;
};

#endif // DATALOGGER_H
