/**
 * @file Correlator6.h
 * @brief 互相关器(重叠保留频域法+归一化互相关模式匹配) — Cross-Correlator with Overlap-Save Frequency Domain Method and Normalized Cross-Correlation for Pattern Matching
 *
 * 功能: 实现互相关器(cross-correlator)，采用重叠保留频域法(overlap-save frequency domain
 *       method)高效计算互相关，支持归一化互相关(normalized cross-correlation)用于
 *       模式匹配(pattern matching)，自动检测峰值位置和相似度。
 *
 * 协作: FFT引擎(FFT) / Convolver5(卷积器) / MatchedFilter4(匹配滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 互相关器(重叠保留频域法+归一化互相关模式匹配)
 */
class Correlator6 : public QObject {
    Q_OBJECT

public:
    /** @brief Correlation peak result */
    struct PeakResult {
        int lag = 0;
        double value = 0.0;
        double normalizedValue = 0.0;
        double confidence = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int patternLength = 0;
        int fftSize = 0;
        int numBlocks = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator6(QObject *parent = nullptr);
    ~Correlator6() override;

    /** @brief Set FFT block size (power of 2, >= 2*patternLength) */
    void setBlockSize(int size);

    /** @brief Compute cross-correlation of two signals */
    QVector<double> correlate(const QVector<double>& signal,
                              const QVector<double>& pattern);

    /** @brief Compute normalized cross-correlation */
    QVector<double> normalizedCorrelate(const QVector<double>& signal,
                                        const QVector<double>& pattern);

    /** @brief Find best match (peak) in signal for pattern */
    PeakResult findPeak(const QVector<double>& signal,
                        const QVector<double>& pattern);

    /** @brief Find all peaks above threshold */
    QVector<PeakResult> findPeaks(const QVector<double>& signal,
                                  const QVector<double>& pattern,
                                  double threshold = 0.5) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(int len, double peakLag, double timeMs);
    void peakFound(int lag, double value, double confidence);

private:
    int m_blockSize = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief FFT (radix-2) in-place */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse);

    /** @brief Next power of 2 */
    int nextPow2(int n) const;

    /** @brief Overlap-save correlation for one block */
    QVector<double> overlapSaveBlock(const QVector<double>& sigBlock,
                                     const QVector<double>& patFFT_re,
                                     const QVector<double>& patFFT_im,
                                     int fftSize) const;

    /** @brief Compute local mean and std for normalization */
    void localStats(const QVector<double>& sig, int start, int len,
                    double& mean, double& std) const;

    /** @brief Compute normalized correlation at specific lag */
    double nccAtLag(const QVector<double>& sig, const QVector<double>& pat,
                    int lag) const;

    /** @brief Parabolic interpolation for sub-sample peak */
    double interpolatePeak(const QVector<double>& corr, int idx) const;
};
