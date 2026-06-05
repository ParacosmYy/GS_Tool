/**
 * @file Limiter.h
 * @brief 音频限幅器 — 峰值限制/ lookahead/增益平滑/砖墙限制
 */
#pragma once
#include <QObject>
#include <QVector>
class Limiter : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalClippingEvents = 0;
        double avgProcessingTimeMs = 0.0;
        double peakReductionDb = 0.0;
    };
    explicit Limiter(QObject* parent = nullptr);
    void setThreshold(double thresholdDb);
    void setReleaseTime(double ms);
    void setLookaheadSamples(int samples);
    void setCeiling(double ceilingDb);
    double processOne(double sample);
    QVector<double> process(const QVector<double>& input);
    double gainReduction() const;
    double peakLevel() const;
    void reset();
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void clippingDetected(double level);
private:
    double computeGainReduction(double inputLevel);
    double m_threshold = -3.0;
    double m_ceiling = -0.3;
    double m_release = 50.0;
    int m_lookahead = 0;
    double m_sampleRate = 44100.0;
    double m_gainReduction = 0.0;
    double m_peakLevel = 0.0;
    QVector<double> m_delayBuffer;
    int m_delayPos = 0;
    Stats m_stats;
    double m_timeSum = 0.0;
};
