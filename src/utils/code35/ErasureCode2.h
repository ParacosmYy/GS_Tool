#pragma once
#include <QObject>
#include <QVector>
class ErasureCode2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalSymbolsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit ErasureCode2(QObject* parent = nullptr);
    void configure(int dataShards, int parityShards);
    QVector<QVector<quint8>> encode(const QVector<QVector<quint8>>& data) const;
    bool decode(QVector<QVector<quint8>>& shards, const QVector<bool>& present);
    int dataShards() const; int parityShards() const; int totalShards() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeComplete(int shards);
    void decodeComplete(int recovered);
private:
    int m_data = 4, m_parity = 2;
    Stats m_stats; double m_timeSum = 0.0;
};
