/**
 * @file SpectralGate.h
 * @brief 频谱门 — 频域噪声抑制+谱减法增强
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SpectralGate : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate(QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setFFTSize(int fftSize);
    void setThreshold(double thresholdDb);
    void setReduction(double reductionDb);
    void learnNoiseProfile(const QVector<double>& noise);
    QVector<double> process(const QVector<double>& input);

    double noiseFloor() const { return m_noiseFloor; }
    bool profileLearned() const { return m_profileLearned; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double noiseReduction);

private:
    double m_sampleRate = 44100.0;
    int m_fftSize = 2048;
    int m_hopSize = 512;
    double m_threshold = -40.0;
    double m_reduction = -20.0;
    double m_noiseFloor = -60.0;
    bool m_profileLearned = false;
    QVector<double> m_noiseProfile;
    QVector<double> m_window;
    QVector<double> m_overlapBuffer;

    void buildWindow();
    void fft(QVector<double>& real, QVector<double>& imag) const;
    void ifft(QVector<double>& real, QVector<double>& imag) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
