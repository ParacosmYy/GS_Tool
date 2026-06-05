#pragma once
#include <QObject>
#include <QVector>
class Compressor2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; double peakReductionDb = 0.0; };
    explicit Compressor2(QObject* parent = nullptr);
    void setThreshold(double db); void setRatio(double r);
    void setKnee(double db); void setAttack(double ms);
    void setRelease(double ms); void setSampleRate(double rate);
    double processOne(double sample);
    QVector<double> process(const QVector<double>& input);
    double gainReduction() const;
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingComplete(int samples);
private:
    double m_threshold = -20.0; double m_ratio = 4.0; double m_knee = 6.0;
    double m_attack = 10.0; double m_release = 100.0; double m_sampleRate = 44100.0;
    double m_envelope = 0.0; double m_gainReduction = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
