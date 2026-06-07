/**
 * @file SlidingDFT5.h
 * @brief 滑动DFT(递推更新+稳定频点跟踪+Goertzel稀疏谱回退) — Sliding DFT with Recursive Update, Stable Bin Tracking and Goertzel Fallback for Sparse Spectra
 *
 * 功能: 实现滑动DFT，支持逐样本递推更新、频点稳定性监控、
 *       Goertzel算法稀疏谱回退和窗函数补偿。
 *
 * 协作: Goertzel4(Goertzel算法) / DistributedArithmetic5(分布式算术) / FftEngine3(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 滑动DFT(递推更新+稳定频点跟踪)
 */
class SlidingDFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        int numBins = 0;
        int unstableBins = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SlidingDFT5(QObject *parent = nullptr);
    ~SlidingDFT5() override;

    void setBlockSize(int N);
    void setSampleRate(double sr);
    void setStabilityThreshold(double threshold);
    void setTargetBins(const QVector<int>& bins);

    /** @brief Update DFT with a single new sample (push oldest out) */
    void updateSample(double sample);

    /** @brief Get magnitude spectrum for all bins */
    QVector<double> magnitudes() const;

    /** @brief Get phase spectrum for all bins */
    QVector<double> phases() const;

    /** @brief Goertzel fallback: compute specific bins only */
    QVector<QPair<int, double>> goertzelSpectrum(
        const QVector<double>& block, const QVector<int>& bins) const;

    /** @brief Get frequency for a given bin index */
    double binFrequency(int bin) const;

    /** @brief Check stability of all bins */
    QVector<bool> checkStability() const;

    /** @brief Reset sliding window */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void spectrumUpdated(int numBins, double timeMs);

private:
    int m_blockSize = 256;
    double m_sampleRate = 44100.0;
    double m_stabThreshold = 1e-6;

    // Complex state per bin: F[k] = real[k] + j*imag[k]
    QVector<double> m_real;
    QVector<double> m_imag;

    // Circular input buffer
    QVector<double> m_inputBuf;
    int m_bufPos = 0;

    // Twiddle factors: exp(j * 2*pi*k/N)
    QVector<double> m_cosW;
    QVector<double> m_sinW;

    // Target bins for sparse mode (empty = all bins)
    QVector<int> m_targetBins;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute twiddle factors */
    void computeTwiddles();

    /** @brief Full recomputation to prevent drift */
    void recomputeFull();
};
