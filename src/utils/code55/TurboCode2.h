#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class TurboCode2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit TurboCode2(QObject* parent = nullptr);
    void setBlockSize(int n);
    void setNumIterations(int iter);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& softBits);
    int blockSize() const { return m_blockSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int iterations, double llr);
private:
    int m_blockSize = 1024; int m_numIter = 8;
    QVector<int> m_interleaver;
    void generateInterleaver(int n);
    QVector<double> bcjrDecode(const QVector<double>& sys, const QVector<double>& par,
                                const QVector<double>& prior, bool terminated);
    Stats m_stats; double m_timeSum = 0.0;
};
