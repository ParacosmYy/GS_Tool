/**
 * @file WaveletDenoiser3.h
 * @brief 小波去噪(软/硬阈值+通用/BayesShrink自适应阈值+SNR估计) — Wavelet Denoiser with Soft/Hard Thresholding, Universal/BayesShrink Adaptive Threshold and SNR Estimation
 *
 * 功能: 实现小波去噪算法，支持软/硬阈值函数、VisuShrink通用阈值、
 *       BayesShrink自适应阈值、多级小波分解和信噪比估计。
 *
 * 协作: DaubechiesWavelet3(Daubechies小波) / HaarWavelet3(Haar小波) / WienerFilter3(Wiener滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(软/硬阈值+通用/BayesShrink)
 */
class WaveletDenoiser3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDenoiseOps = 0;
        int signalLength = 0;
        int decomposeLevels = 0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double threshold = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 阈值函数类型 */
    enum ThresholdType {
        Soft,   ///< Soft thresholding: sign(x)*max(|x|-T, 0)
        Hard    ///< Hard thresholding: x if |x|>T, else 0
    };

    /** @brief 阈值选择方法 */
    enum ThresholdMethod {
        VisuShrink,   ///< Universal threshold: sigma*sqrt(2*ln(N))
        BayesShrink,  ///< BayesShrink: sigma^2/sigma_x per subband
        Minimax       ///< Minimax threshold
    };

    explicit WaveletDenoiser3(QObject *parent = nullptr);
    ~WaveletDenoiser3() override;

    void setDecomposeLevels(int levels);
    void setThresholdType(ThresholdType type);
    void setThresholdMethod(ThresholdMethod method);
    void setWaveletLength(int len);

    /** @brief 去噪 */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief 多级小波分解(返回各级细节+最终近似) */
    QVector<QVector<double>> decompose(const QVector<double>& signal) const;

    /** @brief 多级小波重构 */
    QVector<double> reconstruct(const QVector<QVector<double>>& coefficients) const;

    /** @brief 估计噪声标准差(MAD of finest detail) */
    double estimateNoiseSigma(const QVector<double>& detailCoeffs) const;

    /** @brief VisuShrink通用阈值 */
    double visuThreshold(int N, double sigma) const;

    /** @brief BayesShrink自适应阈值 */
    double bayesThreshold(double sigma, const QVector<double>& coeffs) const;

    /** @brief 估计信噪比(SNR in dB) */
    double estimateSNR(const QVector<double>& signal,
                       const QVector<double>& noise) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseCompleted(double inputSNR, double outputSNR, double threshold);

private:
    int m_levels = 4;
    ThresholdType m_threshType = Soft;
    ThresholdMethod m_threshMethod = BayesShrink;
    int m_waveletLen = 4; // Daubechies-4

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Haar filter coefficients */
    void haarCoeffs(QVector<double>& lowDec, QVector<double>& highDec,
                    QVector<double>& lowRec, QVector<double>& highRec) const;

    /** @brief Single-level DWT */
    void dwtLevel(const QVector<double>& in, QVector<double>& approx,
                  QVector<double>& detail) const;

    /** @brief Single-level IDWT */
    QVector<double> idwtLevel(const QVector<double>& approx,
                               const QVector<double>& detail,
                               int targetLen) const;

    /** @brief Apply threshold to coefficients */
    QVector<double> applyThreshold(const QVector<double>& coeffs,
                                    double T) const;
};
