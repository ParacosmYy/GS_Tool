/**
 * @file SpectralGate6.h
 * @brief 频谱门控(Wiener滤波估计+噪声轮廓学习自适应谱减) — Spectral Gate with Wiener Filter Estimation and Noise Profile Learning for Adaptive Spectral Subtraction
 *
 * 功能: 实现频谱门控(Spectral Gate)，使用Wiener滤波估计(Wiener filter
 *       estimation)从噪声轮廓学习(noise profile learning)获取噪声功率谱，
 *       自适应谱减(adaptive spectral subtraction)实现实时降噪。
 *
 * 协作: KalmanFilter4(卡尔曼滤波) / IIRFilter3(IIR滤波) / FIRFilter2(FIR滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱门控(Wiener滤波+噪声轮廓学习)
 */
class SpectralGate6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        int numFrames = 0;
        int noiseFramesLearned = 0;
        double noiseFloor = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralGate6(QObject *parent = nullptr);
    ~SpectralGate6() override;

    /** @brief Set FFT size (must be power of 2) */
    void setFFTSize(int size);

    /** @brief Set noise threshold multiplier */
    void setThreshold(double threshold);

    /** @brief Learn noise profile from given frames */
    void learnNoiseProfile(const QVector<QVector<double>>& noiseFrames);

    /** @brief Process a single frame, return cleaned frame */
    QVector<double> process(const QVector<double>& frame);

    /** @brief Process entire signal */
    QVector<QVector<double>> processSignal(const QVector<QVector<double>>& frames);

    /** @brief Get current noise power spectrum estimate */
    QVector<double> noiseProfile() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void noiseProfileUpdated(int numFrames, double noiseFloor);
    void frameProcessed(int index, double snr, double timeMs);

private:
    int m_fftSize = 512;
    double m_threshold = 2.0;
    double m_floor = 0.01;     // Spectral floor to avoid zero
    int m_noiseCount = 0;

    QVector<double> m_noisePSD;      // Estimated noise power spectrum
    QVector<double> m_noiseSum;      // Accumulated noise power
    QVector<double> m_window;        // Analysis window (Hann)
    QVector<double> m_prevPhase;     // For overlap-add phase continuity

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build Hann window */
    void buildWindow();

    /** @brief Compute magnitude spectrum from real signal */
    void computeSpectrum(const QVector<double>& frame,
                          QVector<double>& magnitude,
                          QVector<double>& phase) const;

    /** @brief Compute Wiener gain for each bin */
    QVector<double> wienerGain(const QVector<double>& signalPSD) const;

    /** @brief Reconstruct time-domain from magnitude and phase */
    QVector<double> reconstruct(const QVector<double>& magnitude,
                                 const QVector<double>& phase) const;

    /** @brief In-place DFT for real signal (radix-2) */
    void radix2FFT(QVector<double>& re, QVector<double>& im) const;

    /** @brief In-place inverse DFT */
    void radix2IFFT(QVector<double>& re, QVector<double>& im) const;
};
