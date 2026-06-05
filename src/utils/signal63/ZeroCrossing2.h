#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ZeroCrossing2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit ZeroCrossing2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setThreshold(double thresh);
    void setMinDistance(int samples);
    QVector<double> detect(const QVector<double>& signal);
    double frequency() const { return m_freq; }
    int crossingCount() const { return m_count; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void frequencyEstimated(double freq);
private:
    double m_sampleRate = 44100.0; double m_threshold = 0.0; int m_minDist = 10;
    double m_freq = 0.0; int m_count = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
