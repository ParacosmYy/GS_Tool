/**
 * @file Correlator2.h
 * @brief 相关器(广义互相关GCC-PHAT时延估计) — Correlator with Generalized Cross-Correlation (GCC-PHAT) for Time-Delay Estimation
 *
 * 功能: 实现广义互相关器，支持GCC-PHAT时延估计、
 *       多种加权函数和子采样精度峰值检测。
 *
 * 协作: FftEngine3(FFT引擎) / Beamformer2(波束成形) / FilterBank4(滤波器组)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 相关器(GCC-PHAT时延估计)
 */
class Correlator2 : public QObject {
    Q_OBJECT

public:
    /** @brief GCC weighting function */
    enum Weighting { None, PHAT, SCOT, ML, Eckart };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalCorrelations = 0;
        int lastLength = 0;
        double lastDelay = 0.0;
        double lastPeakValue = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator2(QObject *parent = nullptr);
    ~Correlator2() override;

    void setWeighting(Weighting w);
    void setSampleRate(double rate);

    /** @brief Compute cross-correlation via GCC */
    QVector<double> correlate(const QVector<double>& x,
                              const QVector<double>& y) const;

    /** @brief Estimate time delay in samples (sub-sample precision) */
    double estimateDelay(const QVector<double>& x,
                         const QVector<double>& y) const;

    /** @brief Estimate time delay in seconds */
    double estimateDelaySeconds(const QVector<double>& x,
                                const QVector<double>& y) const;

    /** @brief Compute auto-correlation */
    QVector<double> autoCorrelate(const QVector<double>& x) const;

    /** @brief Find correlation peak with parabolic interpolation */
    QPair<double, double> findPeak(const QVector<double>& corr) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(int n, double delay, double peak, double timeMs);

private:
    Weighting m_weighting = PHAT;
    double m_sampleRate = 44100.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief DFT via direct computation (O(n^2)) */
    QVector<QVector<double>> dft(const QVector<double>& x) const;

    /** @brief Inverse DFT */
    QVector<double> idft(const QVector<QVector<double>>& X) const;

    /** @brief Apply GCC weighting to cross-spectrum */
    QVector<QVector<double>> applyWeighting(
        const QVector<QVector<double>>& Gxy) const;

    /** @brief Next power of 2 >= n */
    static int nextPow2(int n);
};
