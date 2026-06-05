/**
 * @file NoiseProfile.h
 * @brief 噪声轮廓分析 — 背景噪声估计/频谱减法/噪声门/自适应阈值
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class NoiseProfile : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProfiles = 0; int totalFramesAnalyzed = 0; double avgProcessingTimeMs = 0.0; };
    explicit NoiseProfile(QObject* parent = nullptr);
    void setSampleRate(double rate);
    void setFFTSize(int size);
    void estimateNoise(const QVector<double>& noiseSamples);
    QVector<double> reduceNoise(const QVector<double>& input);
    QVector<double> noiseSpectrum() const;
    double snr() const;
    void setReductionStrength(double strength);
    void setSmoothing(double smooth);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void noiseEstimated(int frames);
    void reductionComplete(int samples, double estimatedSNR);
private:
    void forwardFFT(QVector<double>& real, QVector<double>& imag);
    double m_sampleRate = 44100.0; int m_fftSize = 2048;
    double m_strength = 1.0; double m_smoothing = 0.95;
    QVector<double> m_noiseSpectrum; QVector<double> m_prevGain;
    Stats m_stats; double m_timeSum = 0.0;
};
