/**
 * @file ProtocolDuplicator.h
 * @brief 协议流量复制器 -- 录制/回放/修改/合并/比较/导入导出
 *
 * 协作: IConnection(数据通道)/ProtocolEngine(协议解析)/自动化测试框架
 * 设计: 策略模式 -- 调度策略可运行时切换
 */

#ifndef PROTOCOLDUPLICATOR_H
#define PROTOCOLDUPLICATOR_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>

#include "connection/interface/IConnection.h"

/** @brief 协议流量复制器 — 录制/回放/修改/合并/比较协议数据流 */
class ProtocolDuplicator : public QObject {
    Q_OBJECT

public:
    enum Direction { Tx = 0, Rx = 1 };
    Q_ENUM(Direction)

    enum ScheduleMode { Once = 0, RepeatN = 1, TimedInterval = 2, ContinuousLoop = 3 };
    Q_ENUM(ScheduleMode)

    /** @brief 单条流量记录 */
    struct TrafficEntry {
        Direction direction = Tx;
        QByteArray data;
        qint64 timestampMs = 0;
        qint64 relativeOffsetMs = 0;
        qint64 sequenceIndex = 0;
    };

    /** @brief 一次完整的录制 */
    struct Recording {
        QString id;
        QString label;
        qint64 startTimestampMs = 0;
        qint64 endTimestampMs = 0;
        QList<TrafficEntry> entries;
        QString sourceName;
        QString targetName;
    };

    /** @brief 字段修改规则 -- 回放时对数据做字节级替换 */
    struct ModificationRule {
        qint64 entryIndex = -1;
        int byteOffset = 0;
        QByteArray oldValue;
        QByteArray newValue;
    };

    /** @brief 比较结果 */
    struct ComparisonResult {
        bool identical = false;
        qint64 matchingEntries = 0;
        qint64 differentEntries = 0;
        qint64 onlyInA = 0;
        qint64 onlyInB = 0;
        QList<int> diffIndices;
        QString summary;
    };

    /** @brief 运行统计 */
    struct DuplicationStats {
        quint64 totalRecorded = 0;
        quint64 totalDuplicated = 0;
        quint64 totalBytesRecorded = 0;
        quint64 totalBytesDuplicated = 0;
        quint64 duplicationCount = 0;
        double avgDelayMs = 0.0;
    };

    explicit ProtocolDuplicator(QObject* parent = nullptr);
    ~ProtocolDuplicator() override;

    void setSource(IConnection* source);
    void setTarget(IConnection* target);
    IConnection* source() const;
    IConnection* target() const;

    void startRecording(const QString& label = QString());
    void stopRecording();
    bool isRecording() const;
    Recording currentRecording() const;

    void startDuplication(const Recording& recording, ScheduleMode mode = Once,
                          int repeatCount = 1, int intervalMs = 0);
    void stopDuplication();
    bool isDuplicating() const;
    void setModificationRules(const QList<ModificationRule>& rules);
    QList<ModificationRule> modificationRules() const;

    QList<Recording> recordings() const;
    void addRecording(const Recording& recording);
    void removeRecording(const QString& id);
    void clearRecordings();

    Recording mergeRecordings(const QList<Recording>& recordings) const;
    ComparisonResult compareRecordings(const Recording& a, const Recording& b) const;

    QJsonObject recordingToJson(const Recording& recording) const;
    Recording recordingFromJson(const QJsonObject& obj) const;
    bool exportRecording(const Recording& recording, const QString& filePath) const;
    Recording importRecording(const QString& filePath) const;

    const DuplicationStats& stats() const;
    void resetStatistics();

signals:
    void recordingStarted();
    void recordingStopped();
    void duplicationStarted();
    void duplicationComplete();
    void error(const QString& errorMsg);

private:
    void onSourceDataReceived(const QByteArray& data);
    void onSourceBytesWritten(qint64 bytes);
    QByteArray applyModifications(const QByteArray& data, qint64 entryIndex) const;
    void scheduleNextEntry();
    void sendEntry(const TrafficEntry& entry);
    void connectSourceSignals();
    void disconnectSourceSignals();

    IConnection* m_source = nullptr;
    IConnection* m_target = nullptr;
    bool m_recording = false;
    bool m_duplicating = false;
    QElapsedTimer m_recordTimer;
    Recording m_currentRecording;

    Recording m_playbackRecording;
    ScheduleMode m_scheduleMode = Once;
    int m_repeatCount = 1;
    int m_intervalMs = 0;
    int m_currentRepeatIndex = 0;
    qint64 m_playbackEntryIndex = 0;
    QTimer* m_playbackTimer = nullptr;
    QList<ModificationRule> m_modificationRules;

    QList<Recording> m_recordings;
    DuplicationStats m_stats;
    QList<double> m_delaySamples;
};

#endif // PROTOCOLDUPLICATOR_H
