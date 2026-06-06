/**
 * @file RaderFFT4.h
 * @brief Rader素数长度FFT(循环卷积+Chirp-z转换) — Rader's Prime-length FFT via Circular Convolution and Chirp-z Conversion
 *
 * 功能: 实现Rader算法，将素数长度DFT转换为循环卷积，
 *       支持原根生成、Chirp-z转换和素数长度快速频谱计算。
 *
 * 协作: PrimeFactorFFT4(素因子FFT) / BluesteinFFT4(Bluestein) / ChirpZ4(Chirp-z)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Rader FFT处理器(素数长度循环卷积)
 */
class RaderFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int primeSize = 0;
        int primitiveRoot = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RaderFFT4(QObject *parent = nullptr);
    ~RaderFFT4() override;

    /** @brief 执行Rader FFT(素数长度) */
    QPair<QVector<double>, QVector<double>> transform(
        const QVector<double>& inputReal,
        const QVector<double>& inputImag = {});

    /** @brief 逆变换 */
    QPair<QVector<double>, QVector<double>> inverseTransform(
        const QVector<double>& re, const QVector<double>& im);

    /** @brief 查找N的原根 */
    int primitiveRoot(int N) const;

    /** @brief 生成原根幂次排列 */
    QVector<int> generatePermutation(int N) const;

    /** @brief 检查是否为素数 */
    bool isPrime(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int N, int root);

private:
    /** @brief 循环卷积(直接法) */
    void circularConvolve(const QVector<double>& aRe,
                          const QVector<double>& aIm,
                          const QVector<double>& bRe,
                          const QVector<double>& bIm,
                          QVector<double>& outRe,
                          QVector<double>& outIm) const;

    /** @brief 小N点DFT */
    void directDFT(const QVector<double>& inRe, const QVector<double>& inIm,
                   QVector<double>& outRe, QVector<double>& outIm,
                   bool inverse) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
