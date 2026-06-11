/**
 * @file WaveletDenoiser19.h
 * @brief 小波去噪器(循环平移与平移不变阈值实现减少吉布斯伪影信号恢复) — Wavelet Denoiser with Cycle-spinning and Translation-invariant Thresholding for Reduced Gibbs Artifact Signal Restoration
 *
 * 功能: 实现小波去噪器(Wavelet denoiser)，采用循环平移(cycle-spinning)
 *       与平移不变阈值(translation-invariant thresholding)实现减少吉布斯伪影信号恢复(reduced Gibbs artifact signal restoration)。
 *
 * 协作: WaveletTransform18(小波变换) / FIRFilter17(FIR滤波) / SignalDecomposer16(信号分解)
 */
#pragma once

#include <QObject>
#include <QVector>

class WaveletDenoiser19 : public QObject {
    Q_OBJECT

public:
    /** @brief Thresholding method */
    enum ThresholdMethod {
        SoftThreshold = 0,
        HardThreshold = 1,
        GarroteThreshold = 2
    };

    /** @brief Denoising result */
    struct DenoiseResult {
        QVector<double> signal;          // Denoised signal
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double noiseEstimate = 0.0;      // Estimated noise sigma
        int numCoeffsZeroed = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numScales = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser19(QObject *parent = nullptr);
    ~WaveletDenoiser19() override;

    void setWavelet(const QString& name);  // "haar", "db2", etc.
    void setDecompositionLevels(int levels);
    void setThresholdMethod(ThresholdMethod method);
    void setNumCycleSpins(int spins);     // Cycle-spinning count
    void setManualThreshold(double t);    // 0 = auto (universal threshold)

    /** @brief Denoise signal using cycle-spinning wavelet thresholding */
    DenoiseResult denoise(const QVector<double>& signal);

    /** @brief Estimate noise sigma using MAD of finest detail coefficients */
    double estimateNoiseSigma(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseDone(int len, double noiseEst, double timeMs);

private:
    QString m_waveletName = QStringLiteral("haar");
    int m_levels = 4;
    ThresholdMethod m_thresholdMethod = SoftThreshold;
    int m_numSpins = 8;
    double m_manualThreshold = 0.0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Haar wavelet forward transform (1 level) */
    void haarForward(const QVector<double>& in,
                      QVector<double>& approx, QVector<double>& detail) const;

    /** @brief Haar wavelet inverse transform (1 level) */
    void haarInverse(const QVector<double>& approx,
                      const QVector<double>& detail,
                      QVector<double>& out) const;

    /** @brief Multi-level forward wavelet transform */
    void forwardWT(const QVector<double>& signal,
                    QVector<QVector<double>>& details,
                    QVector<double>& finalApprox) const;

    /** @brief Multi-level inverse wavelet transform */
    QVector<double> inverseWT(const QVector<QVector<double>>& details,
                                const QVector<double>& finalApprox) const;

    /** @brief Apply thresholding to coefficients */
    void applyThreshold(QVector<double>& coeffs, double threshold) const;

    /** @brief Universal threshold (VisuShrink) */
    double universalThreshold(double sigma, int n) const;

    /** @brief Cycle-shift signal by k positions */
    QVector<double> cycleShift(const QVector<double>& signal, int k) const;

    /** @brief Reverse cycle-shift */
    QVector<double> cycleUnshift(const QVector<double>& signal, int k, int origLen) const;
};
