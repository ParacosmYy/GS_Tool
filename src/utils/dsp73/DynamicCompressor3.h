#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DynamicCompressor3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit DynamicCompressor3(QObject* parent = nullptr);
    void setThreshold(double thresh);
    void setRatio(double ratio);
    void setKnee(double db);
    void setAttack(double ms);
    void setRelease(double ms);
    QVector<double> process(const QVector<double>& input);
    double gainReduction() const { return m_gainReduction; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double reduction);
private:
    double m_threshold = -20.0; double m_ratio = 4.0; double m_knee = 6.0;
    double m_attack = 10.0; double m_release = 100.0; double m_gainReduction = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
