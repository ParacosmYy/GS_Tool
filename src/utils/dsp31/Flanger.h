/**
 * @file Flanger.h
 * @brief 镶边器 — 延迟调制/反馈/LFO/立体声镶边
 */
#pragma once
#include <QObject>
#include <QVector>
class Flanger : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit Flanger(QObject* parent = nullptr);
    void setRate(double hz);
    void setDepth(double depth);
    void setFeedback(double fb);
    void setDelayRange(double minMs, double maxMs);
    void setSampleRate(double rate);
    double processOne(double sample);
    QVector<double> process(const QVector<double>& input);
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingComplete(int samples);
private:
    double interpolate(double frac, double a, double b) const;
    double allpassInterpolate(double frac, double a, double b) const;
    QVector<double> processWithMix(const QVector<double>& input, double dryWet);
    double currentDelayMs() const;
    double frequencyResponse(double freq) const;
    double m_rate = 0.3; double m_depth = 0.8;
    double m_feedback = 0.5; double m_minDelay = 0.5; double m_maxDelay = 5.0;
    double m_sampleRate = 44100.0; double m_phase = 0.0;
    QVector<double> m_delayBuf; int m_delayPos = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
