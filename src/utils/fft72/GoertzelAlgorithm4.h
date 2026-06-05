#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GoertzelAlgorithm4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalBins = 0; double avgProcessingTimeMs = 0.0; };
    explicit GoertzelAlgorithm4(QObject* parent = nullptr);
    void setBlockSize(int n);
    void setSampleRate(double sr);
    void addTargetFreq(double freq);
    void clearTargets();
    QMap<double, double> compute(const QVector<double>& signal);
    double computeSingle(const QVector<double>& signal, double freq) const;
    int blockSize() const { return m_blockSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int targets, int blockSize);
private:
    int m_blockSize = 256; double m_sampleRate = 44100.0;
    QVector<double> m_targets;
    Stats m_stats; double m_timeSum = 0.0;
};
