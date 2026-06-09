/**
 * @file Periodogram6.h
 * @brief 周期图(Slepian锥形多窗估计+自适应加权的减谱泄漏功率谱) — Periodogram with Multitaper Estimation Using Slepian Tapers and Adaptive Weighting for Reduced Spectral Leakage
 *
 * 功能: 实现周期图(Periodogram)功率谱估计，使用Slepian锥形(Slepian tapers)
 *       多窗估计(multitaper estimation)降低谱泄漏(spectral leakage)，通过
 *       自适应加权(adaptive weighting)融合多窗结果提高估计可靠性。
 *
 * 协作: WelchPSD6(Welch功率谱) / NoiseProfile6(噪声分析) / SlidingDFT8(滑动DFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期图(Slepian锥形多窗估计+自适应加权)
 */
class Periodogram6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int fftSize = 0;
        int numTapers = 0;
        double bandwidth = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Periodogram6(QObject *parent = nullptr);
    ~Periodogram6() override;

    /** @brief Set FFT size (0 = auto to next power of 2) */
    void setFftSize(int size);

    /** @brief Set number of Slepian tapers */
    void setNumTapers(int num);

    /** @brief Set time-bandwidth product NW */
    void setBandwidth(double nw);

    /** @brief Compute multitaper periodogram */
    QVector<double> estimate(const QVector<double>& signal);

    /** @brief Get frequency axis values */
    QVector<double> frequencies(int signalLength) const;

    /** @brief Compute Slepian (DPSS) tapers */
    QVector<QVector<double>> computeSlepianTapers(int N, int K, double NW) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimationCompleted(int fftSize, int numTapers, double timeMs);

private:
    int m_fftSize = 0;
    int m_numTapers = 5;
    double m_nw = 4.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Complex type */
    using Complex = QPair<double, double>;

    /** @brief Simple DFT for tapered signal (used for periodogram) */
    QVector<Complex> dft(const QVector<double>& input, int fftSize) const;

    /** @brief Power spectrum magnitude squared */
    QVector<double> powerSpectrum(const QVector<Complex>& spectrum) const;

    /** @brief Tridiagonal solver for DPSS eigenproblem */
    QVector<double> tridiagEigenvector(const QVector<double>& diag,
                                        const QVector<double>& offDiag,
                                        int index) const;
};
