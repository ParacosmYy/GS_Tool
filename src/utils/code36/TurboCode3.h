#pragma once
#include <QObject>
#include <QVector>
class TurboCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalBitsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit TurboCode3(QObject* parent = nullptr);
    void configure(int blockLen, int numIter = 8);
    QVector<int> encode(const QVector<int>& bits) const;
    QVector<int> decode(const QVector<double>& llr);
    void setEarlyTermination(bool enable);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeComplete(int iterations, bool crcPassed);
private:
    int m_blockLen = 6144; int m_numIter = 8; bool m_earlyStop = true;
    Stats m_stats; double m_timeSum = 0.0;
};
