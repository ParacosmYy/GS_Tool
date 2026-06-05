/**
 * @file DataStreamMerger.h
 * @brief 多流数据合并器 -- 按策略交错合并多个命名数据源
 */

#ifndef DATASTREAMMERGER_H
#define DATASTREAMMERGER_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class DataStreamMerger : public QObject {
    Q_OBJECT

public:
    enum class MergePolicy { RoundRobin, Priority, TimestampOrder, Fifo };

    struct StreamEntry {
        QByteArray data;
        QString sourceName;
        qint64 timestampMs = 0;
        int priority = 0;
    };

    struct Stats {
        quint64 totalEntriesMerged = 0;
        quint64 totalBytesMerged = 0;
        quint64 totalSourcesRegistered = 0;
        quint64 totalSourcesRemoved = 0;
        quint64 mergeCycles = 0;
        quint64 starvationEvents = 0;
    };

    explicit DataStreamMerger(QObject* parent = nullptr);

    void setPolicy(MergePolicy policy);
    MergePolicy policy() const;
    void registerSource(const QString& name, int priority = 0);
    void removeSource(const QString& name);
    void feed(const QString& sourceName, const QByteArray& data);
    StreamEntry mergeNext();
    QList<StreamEntry> mergeAll();
    bool hasData() const;
    int pendingCount() const;
    int sourceCount() const;
    void clear();
    Stats stats() const;
    void resetStatistics();

signals:
    void entryMerged(const StreamEntry& entry);
    void sourceStarved(const QString& sourceName);
    void sourceRegistered(const QString& name);
    void sourceRemoved(const QString& name);

private:
    MergePolicy m_policy = MergePolicy::Fifo;
    QMap<QString, QList<StreamEntry>> m_streams;
    QMap<QString, int> m_priorities;
    QStringList m_rrOrder;
    int m_rrIndex = 0;
    QElapsedTimer m_timer;
    Stats m_stats;
    StreamEntry mergeRoundRobin();
    StreamEntry mergePriority();
    StreamEntry mergeTimestamp();
    StreamEntry mergeFifo();
};

#endif // DATASTREAMMERGER_H
