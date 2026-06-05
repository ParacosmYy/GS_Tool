#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Limiter3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit Limiter3(QObject* parent = nullptr);
    void setThreshold(double thresh);
    void setReleaseTime(double ms);
    void setLookahead(int samples);
    QVector<double> process(const QVector<double>& input);
    double peakLevel() const { return m_peak; }
    double gainReduction() const { return m_gainReduction; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double peakGain);
private:
    double m_threshold = -0.3; double m_release = 50.0; int m_lookahead = 64;
    double m_peak = 0.0; double m_gainReduction = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
