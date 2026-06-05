#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GoertzelAlgorithm3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit GoertzelAlgorithm3(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setBlockSize(int n);
    void addTarget(double freq);
    QVector<double> compute(const QVector<double>& signal);
    double magnitudeAt(double freq, const QVector<double>& signal) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void detectionCompleted(double peakFreq, double peakMag);
private:
    double m_sampleRate = 44100.0; int m_blockSize = 256;
    QVector<double> m_targets;
    Stats m_stats; double m_timeSum = 0.0;
};
