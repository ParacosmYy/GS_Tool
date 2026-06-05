/**
 * @file NoiseProfile2.h
 * @brief 噪声轮廓2 — 自适应噪声估计+谱掩码
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class NoiseProfile2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalUpdates = 0;
        int totalFramesAnalyzed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseProfile2(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setFFTSize(int fftSize);
    void setAdaptationRate(double rate);
    void update(const QVector<double>& spectrum, bool isNoise);
    QVector<double> noiseEstimate() const { return m_noiseEstimate; }
    QVector<double> noiseMask() const;
    double snr() const;

    int fftSize() const { return m_fftSize; }
    int framesLearned() const { return m_framesLearned; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void profileUpdated(int frames, double noiseFloor);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 2048;
    double m_adaptRate = 0.95;
    int m_framesLearned = 0;
    QVector<double> m_noiseEstimate;
    QVector<double> m_noisePower;
    QVector<double> m_signalPower;

    Stats m_stats;
    double m_timeSum = 0.0;
};
