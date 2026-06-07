/**
 * @file Correlator4.h
 * @brief 互相关器(广义互相关GCC-PHAT+子采样抛物线插值) — Cross-Correlator with Generalized Cross-Correlation GCC-PHAT and Sub-Sample Parabolic Interpolation
 *
 * 功能: 实现互相关器，支持广义互相关GCC-PHAT、
 *       子采样抛物线插值和时延估计。
 *
 * 协作: FftEngine4(FFT引擎) / Chorus3(合唱效果) / BiquadFilter7(双二阶滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 互相关器(广义互相关GCC-PHAT+子采样抛物线插值)
 */
class Correlator4 : public QObject {
    Q_OBJECT

public:
    /** @brief GCC weighting type */
    enum GccWeighting { None = 0, Phat = 1, Scot = 2, Ml = 3 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastSignalLen = 0;
        int lastFftSize = 0;
        int lastPeakLag = 0;
        double lastPeakDelay = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator4(QObject *parent = nullptr);
    ~Correlator4() override;

    void setSampleRate(double rate);
    void setWeighting(GccWeighting w);

    /** @brief Compute cross-correlation between two signals */
    QVector<double> correlate(const QVector<double>& x,
                               const QVector<double>& y) const;

    /** @brief Compute GCC-PHAT and return with lag indices */
    QVector<QPair<int, double>> gcc(const QVector<double>& x,
                                     const QVector<double>& y) const;

    /** @brief Find time delay with sub-sample parabolic interpolation */
    double estimateDelay(const QVector<double>& x,
                          const QVector<double>& y) const;

    /** @brief Find peak with sub-sample interpolation */
    double findSubsamplePeak(const QVector<double>& corr, int peakIdx) const;

    /** @brief Compute autocorrelation */
    QVector<double> autocorrelate(const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(int len, int peakLag, double delayMs, double timeMs);

private:
    double m_sampleRate = 44100.0;
    GccWeighting m_weighting = Phat;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Next power of 2 */
    static int nextPow2(int n);

    /** @brief In-place DFT (naive, for small N) */
    static void dft(QVector<QPair<double, double>>& data, bool inverse);

    /** @brief Compute magnitude spectrum */
    static QVector<double> magnitude(const QVector<QPair<double, double>>& spec);

    /** @brief Apply GCC weighting to cross-spectrum */
    void applyWeighting(QVector<QPair<double, double>>& crossSpec,
                         const QVector<QPair<double, double>>& specX,
                         const QVector<QPair<double, double>>& specY) const;
};
