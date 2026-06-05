/**
 * @file DelayLine2.h
 * @brief Delay line enhanced - variable/fractional/multi-tap
 */
#pragma once
#include <QObject>
#include <QVector>
class DelayLine2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit DelayLine2(QObject* parent = nullptr);
    void setDelaySamples(double samples);
    void setMaxDelay(int maxSamples);
    void setSampleRate(double rate);
    void setInterpolation(int type);
    double processOne(double sample);
    QVector<double> process(const QVector<double>& input);
    double tap(int offset) const;
    QVector<double> multiTap(const QVector<int>& offsets) const;
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingComplete(int samples);
private:
    double interpolateRead(double fracPos) const;
    double m_delay = 0.0; int m_maxDelay = 44100; double m_sampleRate = 44100.0;
    int m_interpType = 0;
    QVector<double> m_buffer; int m_writePos = 0; double m_readPos = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};
