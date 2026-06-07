/**
 * @file SpectralGate4.h
 * @brief 频谱门控(Wiener滤波抑制曲线+心理声学掩蔽阈值) — Spectral Gate with Wiener Filter Suppression Curve and Psychoacoustic Masking Threshold
 *
 * 功能: 实现频谱门控降噪，支持Wiener滤波抑制曲线计算、
 *       心理声学掩蔽阈值估计和自适应增益控制。
 *
 * 协作: WienerFilter3(Wiener滤波) / FftEngine1(FFT引擎) / WaveletDenoiser7(小波降噪)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 频谱门控(Wiener滤波抑制曲线+心理声学掩蔽阈值)
 */
class SpectralGate4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int frameSize = 0;
        int numFrames = 0;
        double avgNoiseFloor = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate4(QObject *parent = nullptr);
    ~SpectralGate4() override;

    void setFrameSize(int size);
    void setHopSize(int hop);
    void setNoiseEstimateFrames(int frames);
    void setMaskingOffset(double offset);

    /** @brief Estimate noise profile from initial silence frames */
    void estimateNoiseProfile(const QVector<double>& signal);

    /** @brief Compute psychoacoustic masking threshold per bin */
    QVector<double> computeMaskingThreshold(const QVector<QPair<double, double>>& spectrum) const;

    /** @brief Compute Wiener suppression curve */
    QVector<double> computeWienerGain(const QVector<QPair<double, double>>& spectrum) const;

    /** @brief Process full signal */
    QVector<double> process(const QVector<double>& signal);

    /** @brief Get noise profile */
    QVector<double> getNoiseProfile() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int numFrames, double timeMs);

private:
    int m_frameSize = 1024;
    int m_hopSize = 512;
    int m_noiseFrames = 10;
    double m_maskOffset = -6.0; // dB offset for masking

    QVector<double> m_noiseProfile;  // Per-bin noise magnitude estimate
    int m_sampleRate = 44100;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief In-place radix-2 FFT */
    static void fftImpl(QVector<QPair<double, double>>& data, bool inverse);

    /** @brief Apply Hann window */
    static QVector<double> hannWindow(int size);

    /** @brief Compute bark scale frequency for bin */
    double freqToBark(double freq) const;

    /** @brief Spread function for masking */
    QVector<double> spreadFunction(const QVector<double>& excitation) const;
};
