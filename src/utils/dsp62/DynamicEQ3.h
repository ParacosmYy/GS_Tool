#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DynamicEQ3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit DynamicEQ3(QObject* parent = nullptr);
    void setFrequency(double freq);
    void setBandwidth(double bw);
    void setThreshold(double thresh);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    QVector<double> process(const QVector<double>& input);
    double gainReduction() const { return m_gainReduction; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double reduction);
private:
    double m_freq = 1000.0; double m_bw = 1.0; double m_threshold = -10.0;
    double m_ratio = 4.0; double m_attack = 10.0; double m_release = 100.0;
    double m_gainReduction = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
