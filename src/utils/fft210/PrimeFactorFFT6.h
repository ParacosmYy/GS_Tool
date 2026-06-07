/**
 * @file PrimeFactorFFT6.h
 * @brief 素因子FFT(分裂向量嵌套+自排序原位置换) — Prime Factor FFT with Split-Vector Nesting and Self-Sorting In-Place Permutation
 *
 * 功能: 实现素因子FFT算法，支持分裂向量嵌套分解、
 *       自排序原位排列和无twiddle因子DFT。
 *
 * 协作: BruunFFT6(Bruun FFT) / SplitRadixFFT5(分裂基FFT) / WinogradFFT4(Winograd FFT)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 素因子FFT(分裂向量嵌套+自排序原位置换)
 */
class PrimeFactorFFT6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int numFactors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrimeFactorFFT6(QObject *parent = nullptr);
    ~PrimeFactorFFT6() override;

    /** @brief Forward FFT using prime factor decomposition */
    QVector<QPair<double, double>> forward(const QVector<double>& input) const;

    /** @brief Forward FFT on complex input */
    QVector<QPair<double, double>> forwardComplex(
        const QVector<QPair<double, double>>& input) const;

    /** @brief Inverse FFT */
    QVector<QPair<double, double>> inverse(
        const QVector<QPair<double, double>>& spectrum) const;

    /** @brief Factor N into pairwise coprime factors */
    static QVector<int> factorize(int N);

    /** @brief Check if N is a valid PFA length */
    static bool isValidLength(int N);

    /** @brief Find next valid PFA length >= N */
    static int nextValidLength(int N);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, int factors, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief CRT-based index mapping */
    static int crtIndex(int i, const QVector<int>& factors, int N);

    /** @brief Inverse CRT mapping */
    static int inverseCrtIndex(int i, const QVector<int>& factors, int N);

    /** @brief Small-N DFT for prime factor */
    static void smallDFT(QVector<QPair<double, double>>& data,
                          int start, int stride, int len, bool inverse);

    /** @brief Self-sorting in-place permutation */
    static void selfSortPermute(QVector<QPair<double, double>>& data,
                                 const QVector<int>& factors, int N);

    /** @brief Complex multiply helper */
    static QPair<double, double> cmul(const QPair<double, double>& a,
                                        const QPair<double, double>& b);
};
