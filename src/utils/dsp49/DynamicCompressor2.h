#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DynamicCompressor2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessCalls = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit DynamicCompressor2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setThreshold(double db);
    void setRatio(double r);
    void setAttack(double ms);
    void setRelease(double ms);
    void setKnee(double db);
    QVector<double> process(const QVector<double>& input);
    double gainReduction() const { return m_gainReduction; }
    double envelopeLevel() const { return m_envelope; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double reduction);
private:
    double m_sampleRate = 44100.0; double m_threshold = -20.0;
    double m_ratio = 4.0; double m_attack = 10.0; double m_release = 100.0;
    double m_knee = 6.0; double m_envelope = 0.0; double m_gainReduction = 0.0;
    double computeGain(double inputDb) const;
    Stats m_stats; double m_timeSum = 0.0;
};
