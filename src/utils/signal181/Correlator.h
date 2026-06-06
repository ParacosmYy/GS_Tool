/**
 * @file Correlator.h
 * @brief 互相关/自相关器(FFT加速+时延估计+归一化相关系数) — Cross/Auto-correlator with FFT Acceleration, Time-lag Estimation and Normalized Correlation Coefficient
 *
 * 功能: 实现互相关和自相关计算，支持FFT加速、峰值时延估计、
 *       归一化相关系数和频域互功率谱。
 *
 * 协作: WinogradFFT4(Winograd FFT) / RaderFFT4(Rader FFT) / FIRFilter4(FIR滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 互相关/自相关处理器(FFT加速)
 */
class Correlator : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalCorrelations = 0;
        int signalLength = 0;
        double peakLag = 0.0;
        double peakCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Correlator(QObject *parent = nullptr);
    ~Correlator() override;

    void setSampleRate(double rate);
    void setNormalized(bool enabled);

    /** @brief 计算互相关 xcorr(x, y) */
    QVector<double> crossCorrelate(const QVector<double>& x,
                                    const QVector<double>& y);

    /** @brief 计算自相关 xcorr(x, x) */
    QVector<double> autoCorrelate(const QVector<double>& x);

    /** @brief 估计时延(采样点偏移) */
    int estimateLag(const QVector<double>& xcorr) const;

    /** @brief 估计时延(毫秒) */
    double estimateLagMs(const QVector<double>& xcorr) const;

    /** @brief 归一化相关系数(Pearson) */
    double normalizedCoefficient(const QVector<double>& x,
                                  const QVector<double>& y) const;

    /** @brief 互功率谱密度 */
    QPair<QVector<double>, QVector<double>> crossPowerSpectrum(
        const QVector<double>& x, const QVector<double>& y);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void correlationCompleted(int length, double peakLag);

private:
    double m_sampleRate = 44100.0;
    bool m_normalized = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Next power of 2 >= n */
    int nextPow2(int n) const;

    /** @brief Simple radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /** @brief Direct correlation for small signals */
    QVector<double> directCorrelate(const QVector<double>& x,
                                      const QVector<double>& y) const;
};
