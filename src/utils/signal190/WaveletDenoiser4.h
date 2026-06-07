/**
 * @file WaveletDenoiser4.h
 * @brief 小波去噪器(平稳小波变换SWT+SURE阈值估计) — Wavelet Denoiser with Stationary Wavelet Transform (SWT) and SURE Threshold Estimation
 *
 * 功能: 实现平稳小波去噪，支持SWT非抽取变换、SURE无偏阈值估计、
 *       软/硬阈值处理、多尺度分解重构和Haar/D4小波基。
 *
 * 协作: WaveletTransform3(连续小波) / SignalDenoiser3(时域去噪) / FftEngine2(频域分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 小波去噪器(SWT+SURE阈值估计)
 */
class WaveletDenoiser4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDenoiseOps = 0;
        int signalLength = 0;
        int decompositionLevels = 0;
        double threshold = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 阈值类型 */
    enum ThresholdType { SoftThreshold, HardThreshold };

    /** @brief 小波基类型 */
    enum WaveletBase { Haar, D4 };

    explicit WaveletDenoiser4(QObject *parent = nullptr);
    ~WaveletDenoiser4() override;

    void setDecompositionLevels(int levels);
    void setThresholdType(ThresholdType type);
    void setWaveletBase(WaveletBase base);

    /** @brief 对信号进行去噪 */
    QVector<double> denoise(const QVector<double>& signal);

    /** @brief SWT正向变换(返回各层细节+最终逼近) */
    QVector<QVector<double>> swtForward(const QVector<double>& signal) const;

    /** @brief SWT逆向重构 */
    QVector<double> swtInverse(const QVector<QVector<double>>& coeffs) const;

    /** @brief SURE阈值估计 */
    double estimateSureThreshold(const QVector<double>& detailCoeffs) const;

    /** @brief 应用阈值处理 */
    QVector<double> applyThreshold(const QVector<double>& coeffs,
                                   double threshold) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void denoiseCompleted(int levels, double threshold, double timeMs);

private:
    int m_levels = 4;
    ThresholdType m_thresholdType = SoftThreshold;
    WaveletBase m_waveletBase = D4;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Haar decomposition low/high pass */
    void haarFilter(const QVector<double>& in,
                    QVector<double>& approx, QVector<double>& detail) const;

    /** @brief D4 decomposition low/high pass */
    void d4Filter(const QVector<double>& in,
                  QVector<double>& approx, QVector<double>& detail) const;

    /** @brief Haar reconstruction */
    QVector<double> haarReconstruct(const QVector<double>& approx,
                                    const QVector<double>& detail) const;

    /** @brief D4 reconstruction */
    QVector<double> d4Reconstruct(const QVector<double>& approx,
                                  const QVector<double>& detail) const;

    /** @brief Circular shift for SWT translation invariance */
    QVector<double> circShift(const QVector<double>& in, int shift) const;
};
