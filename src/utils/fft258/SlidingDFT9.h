/**
 * @file SlidingDFT9.h
 * @brief 滑动DFT(调制滑窗+周期分母重正化保证稳定性) — Sliding DFT with Modulated Sliding Window and Guaranteed Stability via Periodic Denominator Renormalization
 *
 * 功能: 实现滑动DFT(Sliding DFT)，采用调制滑窗(modulated sliding window)
 *       逐样点更新频谱，通过周期分母重正化(periodic denominator
 *       renormalization)保证数值稳定性。
 *
 * 协作: ZoomFFT6(缩放FFT) / FFT4(FFT) / Goertzel7(Goertzel算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 滑动DFT(调制滑窗+周期分母重正化保证稳定性)
 */
class SlidingDFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int fftSize = 0;
        int samplesProcessed = 0;
        int renormalizations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Bin state for one frequency */
    struct BinState {
        double real = 0.0;
        double imag = 0.0;
        double coeffReal = 0.0;   // Twiddle factor real part
        double coeffImag = 0.0;   // Twiddle factor imaginary part
    };

    explicit SlidingDFT9(QObject *parent = nullptr);
    ~SlidingDFT9() override;

    /** @brief Set DFT size (number of bins) */
    void setSize(int size);

    /** @brief Set renormalization period (in samples) */
    void setRenormPeriod(int period);

    /** @brief Set window type for modulated window */
    void setModulationDepth(double depth);

    /** @brief Push a single sample, returns updated spectrum */
    QVector<double> pushSample(double sample);

    /** @brief Push a block of samples */
    QVector<double> processBlock(const QVector<double>& samples);

    /** @brief Get current magnitude spectrum */
    QVector<double> magnitude() const;

    /** @brief Get current phase spectrum */
    QVector<double> phase() const;

    /** @brief Get frequency axis in Hz */
    QVector<double> frequencyAxis() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumUpdated(int bins, int samplesTotal, double timeMs);

private:
    int m_size = 256;
    int m_renormPeriod = 1024;
    double m_modDepth = 1.0;
    int m_sampleCounter = 0;
    int m_sampleRate = 44100;

    QVector<BinState> m_bins;
    QVector<double> m_circularBuf;
    int m_bufIdx = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize twiddle factors and state */
    void initBins();

    /** @brief Renormalize all bins to prevent drift */
    void renormalize();

    /** @brief Update single bin with new/old sample */
    void updateBin(int k, double newSample, double oldSample);
};
