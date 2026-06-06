/**
 * @file WaveletDenoiser2.h
 * @brief 小波去噪(软/硬阈值+多层分解重构) — Wavelet Denoising with Soft/Hard Thresholding and Multi-Level Decomposition/Reconstruction
 *
 * 功能: 实现小波去噪，支持多级小波分解/重构、软阈值(Soft)和硬阈值(Hard)函数、
 *       Daubechies/haar/Symlet多种小波基。
 *
 * 协作: SignalFilter5(信号滤波) / FftEngine(FFT引擎) / NoiseEstimator(噪声估计)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器
 */
class WaveletDenoiser2 : public QObject {
    Q_OBJECT

public:
    /** @brief 小波基类型 */
    enum WaveletType { Haar = 0, Db2 = 1, Db4 = 2, Sym4 = 3 };

    /** @brief 阈值函数类型 */
    enum ThresholdType { Soft = 0, Hard = 1 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDenoise = 0;          ///< 累计去噪次数
        int lastLevels = 0;                ///< 最近分解层数
        double lastThreshold = 0.0;        ///< 最近使用阈值
        double inputSnr = 0.0;             ///< 输入信噪比估算
        double outputSnr = 0.0;            ///< 输出信噪比估算
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit WaveletDenoiser2(QObject *parent = nullptr);
    ~WaveletDenoiser2() override;

    void setWaveletType(WaveletType type);
    void setDecompositionLevels(int levels);
    void setThresholdType(ThresholdType type);
    void setThreshold(double threshold);

    /**
     * @brief 执行小波去噪
     * @param signal 输入信号
     * @return 去噪后信号
     */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief 多级小波分解 */
    void decompose(const QVector<double>& signal,
                   QVector<QVector<double>>& detailCoeffs,
                   QVector<double>& approxCoeffs) const;

    /** @brief 多级小波重构 */
    QVector<double> reconstruct(
        const QVector<QVector<double>>& detailCoeffs,
        const QVector<double>& approxCoeffs) const;

    /** @brief 自动估计噪声标准差(最高频子带MAD) */
    double estimateNoiseStd(const QVector<double>& detailCoeffs) const;

    /** @brief 计算通用阈值(VisuShrink) */
    double universalThreshold(int n, double sigma) const;

    /** @brief 获取小波滤波器系数 */
    QVector<double> waveletCoeffs() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseCompleted(int levels, double threshold, double snr);

private:
    /** @brief 获取分解低通/高通滤波器 */
    void getFilters(QVector<double>& lowDec, QVector<double>& highDec) const;

    /** @brief 获取重构低通/高通滤波器 */
    void getReconFilters(QVector<double>& lowRec, QVector<double>& highRec) const;

    /** @brief 单级小波变换 */
    void dwtStep(const QVector<double>& input,
                 QVector<double>& approx, QVector<double>& detail) const;

    /** @brief 单级逆小波变换 */
    QVector<double> idwtStep(const QVector<double>& approx,
                              const QVector<double>& detail) const;

    /** @brief 应用阈值函数 */
    double applyThreshold(double value, double threshold) const;

    WaveletType m_wavelet = Db4;
    int m_levels = 4;
    ThresholdType m_threshType = Soft;
    double m_threshold = 0.0;     ///< 0=自动估计

    Stats m_stats;
    double m_timeSum = 0.0;
};
