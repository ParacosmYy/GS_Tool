/**
 * @file PrimeFactorFFT9.h
 * @brief 素因子FFT(嵌套Winograd短N变换编译优化长度静态调度) — Prime Factor FFT with Static Schedule Computation for Nested Winograd Short-N Transforms at Compile-Optimized Lengths
 *
 * 功能: 实现素因子FFT算法(Prime Factor FFT)，将N分解为互素因子N1*N2*...，
 *       使用嵌套Winograd短N变换(nested Winograd short-N transforms)加速，
 *       静态调度计算(static schedule computation)在编译优化长度下运行。
 *
 * 协作: RaderFFT9(Rader FFT) / SplitRadixFFT8(分裂基FFT) / BluesteinFFT8(Bluestein FFT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 素因子FFT(嵌套Winograd短N变换+静态调度)
 */
class PrimeFactorFFT9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int transformSize = 0;
        int numFactors = 0;
        int numTransforms = 0;
        int winogradOps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrimeFactorFFT9(QObject *parent = nullptr);
    ~PrimeFactorFFT9() override;

    /** @brief Prepare transform for size N (must be product of pairwise coprime factors) */
    bool prepare(int n);

    /** @brief Forward FFT (complex interleaved: [re0,im0,re1,im1,...]) */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Inverse FFT */
    QVector<double> inverse(const QVector<double>& spectrum);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, bool forward, double timeMs);

private:
    int m_size = 0;

    /** @brief PFA schedule entry */
    struct PFASchedule {
        int n = 0;
        QVector<int> factors;          // Coprime factors of N
        QVector<int> ni;               // n_i for each factor
        QVector<int> mi;               // m_i = N/n_i for CRT reindexing
        QVector<QVector<int>> winogradSchedules; // Per-factor schedules
    };

    PFASchedule m_schedule;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Factor N into pairwise coprime factors */
    QVector<int> coprimeFactorize(int n) const;

    /** @brief Compute CRT reindexing maps */
    void buildSchedule(int n, const QVector<int>& factors);

    /** @brief Winograd short-N DFT for small prime lengths (2,3,5,7) */
    void winogradShortDFT(const double* inRe, const double* inIm,
                           double* outRe, double* outIm, int len) const;

    /** @brief Apply PFA via nested Winograd transforms */
    void applyPFA(double* re, double* im, bool inverse) const;

    /** @brief Modular inverse via extended Euclidean */
    static int modInverse(int a, int m);

    /** @brief Check if two numbers are coprime */
    static bool isCoprime(int a, int b);
};
