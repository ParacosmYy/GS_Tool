/**
 * @file Periodogram3.h
 * @brief Lomb-Scargle周期图(非均匀采样时间序列) — Lomb-Scargle Periodogram for Unevenly-Sampled Time Series
 *
 * 功能: 实现Lomb-Scargle周期图分析，支持非均匀采样数据、
 *       频率网格搜索和假警报概率估计。
 *
 * 协作: Goertzel6(频率检测) / NoiseGate4(噪声门) / SplitRadixFFT5(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Lomb-Scargle周期图(非均匀采样时间序列)
 */
class Periodogram3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalAnalyses = 0;
        int numPoints = 0;
        int numFreqs = 0;
        double peakPower = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Periodogram3(QObject *parent = nullptr);
    ~Periodogram3() override;

    void setMinFreq(double f);
    void setMaxFreq(double f);
    void setNumFreqs(int n);
    void setOversampling(int factor);

    /** @brief Compute Lomb-Scargle periodogram */
    QVector<QPair<double, double>> compute(const QVector<double>& times,
                                            const QVector<double>& values) const;

    /** @brief Compute Lomb-Scargle power at a single frequency */
    double lombScarglePower(double freq, const QVector<double>& t,
                             const QVector<double>& y, double yMean) const;

    /** @brief Build frequency grid with oversampling */
    QVector<double> buildFreqGrid(const QVector<double>& times) const;

    /** @brief Estimate false alarm probability for a given power */
    double falseAlarmProbability(double power, int n) const;

    /** @brief Find peak frequency and its power */
    QPair<double, double> findPeak(const QVector<QPair<double, double>>& periodogram) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(double peakFreq, double peakPower, double timeMs);

private:
    double m_minFreq = 0.0;
    double m_maxFreq = 0.0;
    int m_numFreqs = 1024;
    int m_oversampling = 4;

    Stats m_stats;
    double m_timeSum = 0.0;
};
