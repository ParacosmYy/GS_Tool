/**
 * @file WinogradFFT4.h
 * @brief Winograd短卷积FFT(2/3/5点Toom-Cook嵌套) — Winograd Short-length Convolution FFT with 2/3/5-point Toom-Cook Nesting
 *
 * 功能: 实现Winograd短长度DFT算法，通过2/3/5点Toom-Cook卷积模块
 *       嵌套构建小N点FFT，最少乘法次数。
 *
 * 协作: RaderFFT4(Rader FFT) / BluesteinFFT4(Bluestein) / PrimeFactorFFT4(素因子)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Winograd短卷积FFT处理器(2/3/5点Toom-Cook嵌套)
 */
class WinogradFFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        int numMultiplications = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WinogradFFT4(QObject *parent = nullptr);
    ~WinogradFFT4() override;

    /** @brief Winograd FFT (supports N = 2,3,4,5,6,8,9,10,12,15,16,20...) */
    QPair<QVector<double>, QVector<double>> transform(
        const QVector<double>& inputReal,
        const QVector<double>& inputImag = {});

    /** @brief 逆变换 */
    QPair<QVector<double>, QVector<double>> inverseTransform(
        const QVector<double>& re, const QVector<double>& im);

    /** @brief 分解N为素因子 */
    QVector<int> factorize(int N) const;

    /** @brief 计算Winograd乘法次数 */
    int multiplicationCount(int N) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int N, int mults);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Winograd 2-point DFT (1 mult) */
    void wft2(const double* inR, const double* inI,
              double* outR, double* outI, bool inv) const;

    /** @brief Winograd 3-point DFT (via Toom-Cook, 2 mults) */
    void wft3(const double* inR, const double* inI,
              double* outR, double* outI, bool inv) const;

    /** @brief Winograd 5-point DFT (via Toom-Cook, 5 mults) */
    void wft5(const double* inR, const double* inI,
              double* outR, double* outI, bool inv) const;

    /** @brief General small-N via direct DFT fallback */
    void directDFT(const QVector<double>& inR, const QVector<double>& inI,
                   QVector<double>& outR, QVector<double>& outI,
                   bool inv) const;

    /** @brief Nested Winograd for composite N */
    void winogradNested(const QVector<double>& inR, const QVector<double>& inI,
                         QVector<double>& outR, QVector<double>& outI,
                         bool inv) const;

    /** @brief Twiddle factor table */
    double twiddle(int N, int k, bool inv) const;
};
