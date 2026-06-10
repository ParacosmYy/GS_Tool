/**
 * @file Periodogram8.h
 * @brief 周期图(Bartlett平均周期图与Welch重叠段方差降低谱估计) — Periodogram with Bartlett's Method Averaged Periodogram and Welch's Overlapped Segment for Variance-reduced Spectral Estimation
 *
 * 功能: 实现周期图(Periodogram)，采用Bartlett平均周期图(Bartlett's method averaged periodogram)
 *       与Welch重叠段(Welch's overlapped segment)实现方差降低谱估计
 *       (variance-reduced spectral estimation)。
 *
 * 协作: WelchPSD7(Welch PSD) / LombScargle8(Lomb-Scargle) / BlackmanTukey6(Blackman-Tukey)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期图(Bartlett平均周期图与Welch重叠段)
 */
class Periodogram8 : public QObject {
    Q_OBJECT

public:
    /** @brief Window function type */
    enum Window { Rectangular = 0, Hanning, Hamming, Blackman, Bartlett };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int segmentLength = 0;
        int numSegments = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Periodogram8(QObject *parent = nullptr);
    ~Periodogram8() override;

    /** @brief Set segment length for Bartlett/Welch methods */
    void setSegmentLength(int len);

    /** @brief Set overlap ratio for Welch method [0, 0.9] */
    void setOverlapRatio(double ratio);

    /** @brief Set window function */
    void setWindow(Window win);

    /** @brief Set FFT size (0 = auto to next power of 2) */
    void setFFTSize(int n);

    /** @brief Compute raw periodogram (no averaging) */
    QVector<double> rawPeriodogram(const QVector<double>& signal, double sampleRate);

    /** @brief Bartlett's method: non-overlapping averaged periodogram */
    QVector<double> bartlett(const QVector<double>& signal, double sampleRate);

    /** @brief Welch's method: overlapping averaged periodogram */
    QVector<double> welch(const QVector<double>& signal, double sampleRate);

    /** @brief Get frequency bins for last computation */
    QVector<double> frequencyBins() const;

    /** @brief Get power spectral density in dB */
    QVector<double> psdDb(const QVector<double>& psd) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimationDone(int numSegments, int fftSize, double timeMs);

private:
    int m_segmentLen = 256;
    double m_overlapRatio = 0.5;
    Window m_window = Hanning;
    int m_fftSize = 0;

    QVector<double> m_freqBins;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate window coefficients of given length */
    QVector<double> generateWindow(int len) const;

    /** @brief Compute FFT of real signal (Cooley-Tukey radix-2) */
    void fftReal(QVector<double>& re, QVector<double>& im) const;

    /** @brief Bit-reversal permutation */
    void bitReverse(QVector<double>& re, QVector<double>& im) const;

    /** @brief Compute next power of 2 >= n */
    int nextPow2(int n) const;
};
