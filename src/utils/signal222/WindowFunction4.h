/**
 * @file WindowFunction4.h
 * @brief 窗函数库(Slepian DPSS多锥生成+时间带宽积优化) — Window Function Library with Slepian DPSS Multi-Taper Generation and Time-Bandwidth Product Optimization
 *
 * 功能: 实现窗函数库，包含Slepian DPSS多锥序列生成，
 *       通过时间带宽积优化实现最优频谱估计。
 *
 * 协作: Goertzel7(频率检测) / MultibandGate4(多频段门控) / SplitRadixFFT6(FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 窗函数库(Slepian DPSS多锥+时间带宽积优化)
 */
class WindowFunction4 : public QObject {
    Q_OBJECT

public:
    /** @brief Supported window types */
    enum WindowType {
        Rectangular = 0, Hann, Hamming, Blackman, BlackmanHarris,
        Kaiser, Gaussian, FlatTop, DPSS
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int windowLength = 0;
        int numTapers = 0;
        double timeBandwidthProduct = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WindowFunction4(QObject *parent = nullptr);
    ~WindowFunction4() override;

    /** @brief Generate a window of given type and length */
    QVector<double> generate(WindowType type, int length, double parameter = 0.0) const;

    /** @brief Generate Slepian DPSS multi-taper sequences */
    QVector<QVector<double>> generateDPSS(int length, int numTapers,
                                            double timeBandwidth) const;

    /** @brief Compute optimal time-bandwidth product for given length and num tapers */
    double optimizeTimeBandwidth(int length, int numTapers) const;

    /** @brief Apply window to signal */
    QVector<double> apply(const QVector<double>& signal, WindowType type,
                           double parameter = 0.0) const;

    /** @brief Apply multi-taper to signal, returns tapered signals */
    QVector<QVector<double>> applyMultiTaper(const QVector<double>& signal,
                                              int numTapers,
                                              double timeBandwidth) const;

    /** @brief Compute coherence loss of a window */
    double coherenceLoss(WindowType type, int length, double parameter = 0.0) const;

    /** @brief Compute equivalent noise bandwidth (ENBW) */
    double enbw(WindowType type, int length, double parameter = 0.0) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(int length, int type, double timeMs);
    void dpssGenerated(int tapers, double timeBandwidth, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Kaiser window with beta parameter */
    QVector<double> kaiserWindow(int length, double beta) const;

    /** @brief Modified Bessel function I0(x) for Kaiser window */
    double besselI0(double x) const;

    /** @brief Gaussian window with sigma parameter */
    QVector<double> gaussianWindow(int length, double sigma) const;

    /** @brief Slepian DPSS via tridiagonal eigenproblem (symmetric) */
    QVector<QVector<double>> slepianDPSS(int length, int numTapers,
                                           double timeBandwidth) const;

    /** @brief Solve tridiagonal eigenproblem using QR iteration */
    QVector<double> tridiagEigenvalues(int n, const QVector<double>& diag,
                                        const QVector<double>& offDiag) const;

    /** @brief Inverse iteration for eigenvector from eigenvalue */
    QVector<double> inverseIteration(int n, const QVector<double>& diag,
                                      const QVector<double>& offDiag,
                                      double eigenvalue) const;

    /** @brief Compute DPSS eigenvalue concentration ratio */
    double dpssConcentration(int length, double eigenvalue) const;
};
