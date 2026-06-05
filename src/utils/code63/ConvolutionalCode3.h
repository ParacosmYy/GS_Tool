#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ConvolutionalCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit ConvolutionalCode3(QObject* parent = nullptr);
    void setGenerators(const QVector<int>& gens);
    void setConstraintLength(int k);
    void setTrellisTermination(bool term);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& softBits);
    void setTracebackDepth(int depth);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeCompleted(int inLen, int outLen);
    void decodeCompleted(int bitErrors);
private:
    int m_constraint = 7; QVector<int> m_gens; bool m_terminate = true; int m_traceback = 30;
    int branchMetric(double received, int expected) const;
    Stats m_stats; double m_timeSum = 0.0;
};
