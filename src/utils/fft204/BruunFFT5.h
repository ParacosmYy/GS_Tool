/**
 * @file BruunFFT5.h
 * @brief Bruun实数FFT(z^N+1多项式因子分解cos/sin分解) — Bruun's Real-Valued FFT with Polynomial Factorization mod (z^N+1) for Cos/Sin Decomposition
 *
 * 功能: 实现Bruun实数FFT算法，支持z^N+1多项式因子分解、
 *       cos/sin分量提取和纯实数运算蝶形网络。
 *
 * 协作: WinogradFFT6(Winograd FFT) / RaderFFT5(Rader FFT) / FFTW5(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Bruun实数FFT(z^N+1多项式因子分解cos/sin分解)
 */
class BruunFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BruunFFT5(QObject *parent = nullptr);
    ~BruunFFT5() override;

    /** @brief Set transform size (must be power of 2) */
    void setTransformSize(int n);

    /** @brief Forward real-valued FFT: returns cos/sin coefficients */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse transform from cos/sin coefficients */
    QVector<double> inverse(const QVector<double>& cosCoeff, const QVector<double>& sinCoeff);

    /** @brief Factorize z^N+1 into quadratic factors for Bruun decomposition */
    QVector<QPair<double, double>> quadraticFactors(int n) const;

    /** @brief Apply Bruun butterfly stage */
    void butterflyStage(QVector<double>& data, int stage) const;

    /** @brief Extract cos and sin components from butterfly output */
    void extractCosSin(const QVector<double>& butterflies,
                        QVector<double>& cosOut, QVector<double>& sinOut) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    int m_size = 64;

    // Precomputed cos/sin twiddle factors
    QVector<double> m_cosTwiddle;
    QVector<double> m_sinTwiddle;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Precompute twiddle factors for Bruun decomposition */
    void precompute(int n);
};
