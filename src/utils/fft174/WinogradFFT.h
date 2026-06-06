/**
 * @file WinogradFFT.h
 * @brief Winograd FFT(小N最小乘法+大N嵌套) — Winograd FFT for Small-N (2,3,4,5,7) with Minimal Multiplications and Large-N via Nesting
 *
 * 功能: 实现Winograd FFT算法，支持小N(2,3,4,5,7)最小乘法DFT、
 *       大N嵌套分解、素因子映射。
 *
 * 协作: SplitRadixFFT(分裂基FFT) / PrimeFactorFFT(素因子FFT) / FftEngine(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Winograd FFT处理器
 */
class WinogradFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;      ///< 累计变换次数
        int lastN = 0;                    ///< 最近变换长度
        int lastMultiplies = 0;           ///< 最近乘法次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit WinogradFFT(QObject *parent = nullptr);
    ~WinogradFFT() override;

    /**
     * @brief 正向Winograd FFT
     * @param real 实部输入/输出
     * @param imag 虚部输入/输出
     * @return true=成功
     */
    bool transform(QVector<double>& real, QVector<double>& imag);

    /**
     * @brief 逆向Winograd FFT
     * @param real 实部输入/输出
     * @param imag 虚部输入/输出
     * @return true=成功
     */
    bool inverseTransform(QVector<double>& real, QVector<double>& imag);

    /** @brief 计算Winograd乘法次数 */
    static int multiplyCount(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, int multiplies);

private:
    /** @brief W-DFT size 2 (1 mul) */
    void dft2(double* r, double* i) const;

    /** @brief W-DFT size 3 (2 mul) */
    void dft3(double* r, double* i) const;

    /** @brief W-DFT size 4 (3 mul) */
    void dft4(double* r, double* i) const;

    /** @brief W-DFT size 5 (5 mul) */
    void dft5(double* r, double* i) const;

    /** @brief W-DFT size 7 (8 mul) */
    void dft7(double* r, double* i) const;

    /** @brief Factor N into supported primes */
    QVector<int> factorize(int n) const;

    /** @brief Nested Winograd for composite N */
    void nestedTransform(double* r, double* i, int n);

    Stats m_stats;
    int m_lastMuls = 0;
    double m_timeSum = 0.0;
};
