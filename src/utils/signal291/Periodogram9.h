/**
 * @file Periodogram9.h
 * @brief 周期图(多锥Slepian序列与自适应加权降低频谱泄漏功率估计) — Periodogram with Multi-taper Slepian Sequences and Adaptive Weighting for Reduced Spectral Leakage Power Estimation
 *
 * 功能: 实现周期图(Periodogram)，采用多锥Slepian序列(multi-taper Slepian sequences)
 *       与自适应加权(adaptive weighting)降低频谱泄漏功率估计(reduced spectral leakage power estimation)。
 *
 * 协作: FFT8(FFT) / WelchPeriodogram9(Welch周期图) / STFT10(短时傅里叶变换)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期图(多锥Slepian序列与自适应加权降低频谱泄漏功率估计)
 */
class Periodogram9 : public QObject {
    Q_OBJECT

public:
    /** @brief Spectrum result */
    struct SpectrumResult {
        QVector<double> frequencies;     // Frequency bins (Hz)
        QVector<double> powerSpectrum;   // Power spectral density
        QVector<double> phase;           // Phase spectrum
        int n = 0;
        int numTapers = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastN = 0;
        int lastNW = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Periodogram9(QObject *parent = nullptr);
    ~Periodogram9() override;

    void setSampleRate(double sr);
    void setNumTapers(int nw);    // Number of Slepian tapers (time-bandwidth product)
    void setAdaptive(bool on);    // Use adaptive weighting

    /** @brief Compute multi-taper periodogram */
    SpectrumResult compute(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computeDone(int n, int numTapers, double timeMs);

private:
    double m_sampleRate = 44100.0;
    int m_numTapers = 5;     // NW parameter
    bool m_adaptive = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Slepian (DPSS) tapers via tridiagonal eigenproblem */
    QVector<QVector<double>> computeSlepianTapers(int n, int nw, int k) const;

    /** @brief Solve tridiagonal eigenvalue problem for DPSS */
    void dpssEigen(int n, double w, QVector<double>& eigvals,
                   QVector<QVector<double>>& eigvecs, int k) const;

    /** @brief Simple radix-2 FFT for internal use */
    void fft(QVector<double>& re, QVector<double>& im, int n, bool inverse) const;

    /** @brief Next power of 2 */
    static int nextPow2(int n);
};
