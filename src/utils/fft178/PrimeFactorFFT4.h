/**
 * @file PrimeFactorFFT4.h
 * @brief 素因子FFT(Good-Thomas算法+互质分解+置换索引) — Prime Factor (Good-Thomas) FFT with Coprime Factorization and Permuted Indexing
 *
 * 功能: 实现素因子FFT(Good-Thomas算法)，支持互质因子分解、
 *       中国剩余定理索引映射和分块DFT计算。
 *
 * 协作: ChirpZ4(Chirp-z) / SplitRadixFFT4(分裂基) / BluesteinFFT4(Bluestein)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 素因子FFT处理器(Good-Thomas算法)
 */
class PrimeFactorFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int inputSize = 0;
        int factor1 = 0;
        int factor2 = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrimeFactorFFT4(QObject *parent = nullptr);
    ~PrimeFactorFFT4() override;

    /** @brief 执行素因子FFT */
    QPair<QVector<double>, QVector<double>> transform(
        const QVector<double>& inputReal,
        const QVector<double>& inputImag = {});

    /** @brief 执行逆变换 */
    QPair<QVector<double>, QVector<double>> inverseTransform(
        const QVector<double>& re, const QVector<double>& im);

    /** @brief 分解N为互质因子 */
    QPair<int, int> factorize(int N) const;

    /** @brief 中国剩余定理索引映射 */
    int crtMap(int k, int n1, int n2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int N, int f1, int f2);

private:
    /** @brief 小N点DFT(直接计算) */
    void shortDFT(QVector<double>& re, QVector<double>& im,
                  int N, bool inverse) const;

    /** @brief 转置二维矩阵 */
    static void transpose(QVector<double>& data, int rows, int cols);

    Stats m_stats;
    double m_timeSum = 0.0;
};
