#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DelayLine4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit DelayLine4(QObject* parent = nullptr);
    void setDelayTime(double ms);
    void setSampleRate(double sr);
    void setFeedback(double fb);
    void setMix(double mix);
    void setMode(const QString& mode);
    QVector<double> process(const QVector<double>& input);
    double delayTime() const { return m_delayMs; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double delayMs);
private:
    double m_delayMs = 100.0; double m_sampleRate = 44100.0;
    double m_feedback = 0.3; double m_mix = 0.5; QString m_mode = "normal";
    QVector<double> m_buffer; int m_writePos = 0;
    double readSample(int delaySamples);
    void writeSample(double sample);
    Stats m_stats; double m_timeSum = 0.0;
};
