/**
 * @file WaveletDenoiser16.h
 * @brief 小波去噪器(贝叶斯收缩与尺度间依赖建模信号保留噪声去除) — Wavelet Denoiser with Bayesian Shrinkage and Inter-scale Dependency Modeling for Signal-preserving Noise Removal
 *
 * 功能: 实现小波去噪器(Wavelet denoiser)，采用贝叶斯收缩(Bayesian shrinkage)
 *       与尺度间依赖建模(inter-scale dependency modeling)实现信号保留噪声去除(signal-preserving noise removal)。
 *
 * 协作: FIRFilter12(FIR滤波器) / AdaptiveFilter11(自适应滤波) / MedianFilter9(中值滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(贝叶斯收缩与尺度间依赖建模)
 */
class WaveletDenoiser16 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numLevels = 0;
        double inputSNR = 0.0;
        double outputSNR = 0.0;
        double noiseStd = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WaveletDenoiser16(QObject *parent = nullptr);
    ~WaveletDenoiser16() override;

    /** @brief Set decomposition levels */
    void setLevels(int levels);

    /** @brief Set wavelet type (0=Haar, 1=DB2, 2=DB4) */
    void setWaveletType(int type);

    /** @brief Estimate noise standard deviation from coefficients */
    void estimateNoise(const QVector<double>& signal);

    /** @brief Set noise level manually */
    void setNoiseStd(double sigma);

    /** @brief Denoise signal, returns cleaned signal */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief Get wavelet coefficients at given level */
    QVector<double> coefficients(int level) const;

    /** @brief Get reconstruction from denoised coefficients */
    QVector<double> reconstructed() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoisingDone(int length, double inputSNR, double outputSNR, double timeMs);

private:
    int m_levels = 4;
    int m_waveletType = 0;       // 0=Haar, 1=DB2, 2=DB4
    double m_noiseStd = 0.0;

    QVector<QVector<double>> m_detailCoeffs;   // Per-level detail coefficients
    QVector<double> m_approxCoeffs;
    QVector<double> m_reconstructed;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get wavelet filter coefficients */
    void getFilterCoeffs(QVector<double>& loDec, QVector<double>& hiDec,
                          QVector<double>& loRec, QVector<double>& hiRec) const;

    /** @brief Forward wavelet transform */
    void forwardDWT(const QVector<double>& signal);

    /** @brief Inverse wavelet transform */
    QVector<double> inverseDWT();

    /** @brief Bayesian shrinkage with inter-scale dependency */
    void bayesianShrinkage();

    /** @brief Estimate noise sigma via MAD of finest detail coefficients */
    double estimateSigma(const QVector<double>& coeffs) const;

    /** @brief Compute signal power */
    double signalPower(const QVector<double>& s) const;
};
