/**
 * @file PrimeFactorFFT5.h
 * @brief 素因子FFT(Rader算法集成+素数长度处理) — Prime Factor FFT with Rader's Algorithm Integration for Prime-Factor Handling
 *
 * 功能: 实现素因子分解FFT算法，集成Rader算法处理素数长度变换、
 *       支持任意长度的高效频域计算。
 *
 * 协作: ChirpZ5(线性调频Z变换) / Goertzel5(Goertzel算法) / FftEngine3(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 素因子FFT(Rader算法+素数长度处理)
 */
class PrimeFactorFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int numPrimeFactors = 0;
        bool usedRader = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrimeFactorFFT5(QObject *parent = nullptr);
    ~PrimeFactorFFT5() override;

    /** @brief Compute forward FFT on complex data [re,im,re,im,...] */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Compute inverse FFT */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Factor n into coprime factors */
    QVector<int> factorize(int n) const;

    /** @brief Check if n is prime */
    bool isPrime(int n) const;

    /** @brief Compute FFT of prime length using Rader's algorithm */
    QVector<QVector<double>> raderFFT(
        const QVector<QVector<double>>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, int factors, bool rader, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find primitive root modulo p */
    int primitiveRoot(int p) const;

    /** @brief Power mod */
    int powMod(int base, int exp, int mod) const;

    /** @brief PFA (Prime Factor Algorithm) inner loop */
    QVector<QVector<double>> pfaCore(
        const QVector<QVector<double>>& x,
        const QVector<int>& factors) const;

    /** @brief Small-N DFT kernel */
    QVector<QVector<double>> smallDFT(
        const QVector<QVector<double>>& x, int N, bool inverse) const;

    /** @brief Interleave complex pairs to flat array */
    static QVector<double> toFlat(const QVector<QVector<double>>& c);
    static QVector<QVector<double>> fromFlat(const QVector<double>& f);
};
