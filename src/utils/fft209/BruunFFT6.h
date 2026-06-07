/**
 * @file BruunFFT6.h
 * @brief Bruun FFT(多项式残差分解+实值对称蝶形网络) — Bruun FFT with Polynomial Residue Factorization and Real-Valued Symmetric Butterfly Network
 *
 * 功能: 实现Bruun FFT算法，支持多项式残差因子分解、
 *       实值对称蝶形网络结构和纯实数输入优化。
 *
 * 协作: RaderFFT6(Rader FFT) / SplitRadixFFT5(分裂基FFT) / WinogradFFT4(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Bruun FFT(多项式残差分解+实值对称蝶形网络)
 */
class BruunFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int numButterflies = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT6(QObject *parent = nullptr);
    ~BruunFFT6() override;

    /** @brief Forward real-valued FFT using Bruun butterfly network */
    QVector<QPair<double, double>> forward(const QVector<double>& input) const;

    /** @brief Forward FFT on complex input */
    QVector<QPair<double, double>> forwardComplex(
        const QVector<QPair<double, double>>& input) const;

    /** @brief Inverse FFT */
    QVector<QPair<double, double>> inverse(
        const QVector<QPair<double, double>>& spectrum) const;

    /** @brief Compute Bruun butterfly twiddle factors */
    void precomputeTwiddles(int N);

    /** @brief Get number of butterfly stages for size N */
    static int numStages(int N);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int butterflies, double timeMs);

private:
    int m_cachedSize = 0;
    QVector<double> m_twiddleCos;
    QVector<double> m_twiddleSin;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Bruun real butterfly stage */
    static void bruunButterfly(QVector<QPair<double, double>>& data,
                                int stage, int totalStages,
                                const QVector<double>& twCos,
                                const QVector<double>& twSin);

    /** @brief Bit reverse permutation */
    static int bitReverse(int x, int bits);

    /** @brief Complex multiply helper */
    static QPair<double, double> cmul(const QPair<double, double>& a,
                                       const QPair<double, double>& b);
};
