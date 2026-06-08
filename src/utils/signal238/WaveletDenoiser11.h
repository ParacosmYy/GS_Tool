/**
 * @file WaveletDenoiser11.h
 * @brief 小波去噪器(双树复小波变换+跨尺度依赖双变量收缩) — Wavelet Denoiser with Dual-Tree Complex Wavelet Transform and Bivariate Shrinkage with Interscale Dependency
 *
 * 功能: 实现小波去噪器(Wavelet denoiser)，采用双树复小波变换(Dual-tree complex wavelet
 *       transform, DTCWT)具有近似平移不变性，结合跨尺度依赖(inter-scale dependency)
 *       的双变量收缩(bivariate shrinkage)阈值策略进行信号降噪。
 *
 * 协作: WaveletTransform9(小波变换) / KalmanFilter10(卡尔曼滤波) / SavitzkyGolay7(SG滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(双树复小波变换+跨尺度依赖双变量收缩)
 */
class WaveletDenoiser11 : public QObject {
    Q_OBJECT

public:
    /** @brief Denoising result */
    struct DenoiseResult {
        QVector<double> signal;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double noiseEstimate = 0.0;
        int levelsUsed = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int decompLevels = 0;
        double avgProcessingTimeMs = 0.0;
        double avgNoiseReductionDb = 0.0;
    };

    explicit WaveletDenoiser11(QObject *parent = nullptr);
    ~WaveletDenoiser11() override;

    /** @brief Set decomposition levels (0 = auto) */
    void setDecompositionLevels(int levels);

    /** @brief Set noise estimation method threshold */
    void setNoiseThreshold(double threshold);

    /** @brief Set mother wavelet type index (0=Haar, 1=DB2, 2=DB4) */
    void setWaveletType(int type);

    /** @brief Denoise input signal */
    DenoiseResult denoise(const QVector<double>& signal);

    /** @brief Estimate noise standard deviation via MAD */
    double estimateNoiseStd(const QVector<double>& detailCoeffs) const;

    /** @brief Get wavelet coefficients */
    QVector<QVector<double>> detailCoefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseCompleted(int length, double snrImprovement, double timeMs);

private:
    int m_levels = 0;
    double m_noiseThreshold = 3.0;
    int m_waveletType = 1;

    QVector<double> m_approxCoeffs;
    QVector<QVector<double>> m_detailCoeffs;  // per level

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Auto-select decomposition levels */
    int autoLevels(int signalLen) const;

    /** @brief Dual-tree CWT forward (Tree A and Tree B) */
    void dtcwtForward(const QVector<double>& signal);

    /** @brief Dual-tree CWT inverse */
    QVector<double> dtcwtInverse();

    /** @brief Single-tree wavelet decomposition one level */
    void decomposeOneLevel(const QVector<double>& input,
                            QVector<double>& approx,
                            QVector<double>& detail) const;

    /** @brief Single-tree wavelet reconstruction one level */
    QVector<double> reconstructOneLevel(const QVector<double>& approx,
                                         const QVector<double>& detail) const;

    /** @brief Bivariate shrinkage with interscale dependency */
    void bivariateShrinkage(QVector<double>& childDetail,
                             const QVector<double>& parentDetail,
                             double noiseStd);

    /** @brief Compute signal SNR */
    static double computeSNR(const QVector<double>& signal, const QVector<double>& noise);

    /** @brief Get wavelet filter coefficients */
    QVector<double> lowPassFilter() const;
    QVector<double> highPassFilter() const;
};
