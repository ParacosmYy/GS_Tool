/**
 * @file Periodogram4.h
 * @brief 周期图(多窗Thomson估计+自适应加权降低谱泄漏) — Periodogram with Multitaper Thomson Estimation and Adaptive Weighting for Spectral Leakage Reduction
 *
 * 功能: 实现基于多窗法的Thomson谱估计，使用离散扁球序列作为数据窗，
 *       自适应加权组合各窗估计以降低谱泄漏。
 *
 * 协作: WelchPeriodogram3(Welch周期图) / BurgMethod3(Burg方法) / MUSIC4(MUSIC算法)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 周期图(多窗Thomson+自适应加权)
 */
class Periodogram4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int signalLength = 0;
        int numTapers = 0;
        int fftSize = 0;
        double bandwidth = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Periodogram4(QObject *parent = nullptr);
    ~Periodogram4() override;

    /** @brief Set parameters: FFT size, number of tapers (NW * 2 - 1), time-bandwidth product NW */
    void setParameters(int fftSize = 1024, int numTapers = 5, double nw = 3.0);

    /** @brief Compute multitaper power spectral density estimate */
    QVector<double> estimate(const QVector<double>& signal);

    /** @brief Compute adaptive weights for taper combination */
    QVector<QVector<double>> adaptiveWeights(
        const QVector<QVector<double>>& eigCoeffs,
        const QVector<double>& eigenvalues) const;

    /** @brief Get the taper windows (DPSS approximations) */
    QVector<QVector<double>> tapers() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimationCompleted(int fftSize, int numTapers, double timeMs);

private:
    int m_fftSize = 1024;
    int m_numTapers = 5;
    double m_nw = 3.0;

    QVector<QVector<double>> m_tapers;  // Precomputed taper sequences
    QVector<double> m_taperEigenvalues; // Eigenvalues of each taper

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute DPSS (Slepian) tapers via tridiagonal formulation */
    void computeTapers();

    /** @brief Simple DFT at specific frequency bin */
    QVector<double> dftBin(const QVector<double>& signal, int fftSize) const;

    /** @brief Tridiagonal eigensolver for DPSS */
    QVector<QVector<double>> tridiagEigen(int n, double nw) const;
};
