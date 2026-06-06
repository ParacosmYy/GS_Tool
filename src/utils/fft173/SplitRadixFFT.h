/**
 * @file SplitRadixFFT.h
 * @brief 分裂基FFT(L型蝶形运算+最小算术复杂度) — Split-Radix FFT with L-Shaped Butterfly for N = 2^k Achieving Minimal Arithmetic Count
 *
 * 功能: 实现分裂基FFT算法，利用L型蝶形将N点DFT分解为N/2和两个N/4变换，
 *       达到N=2^k中最低的乘法/加法次数。
 *
 * 协作: PrimeFactorFFT(素因子FFT) / Radix4FFT3(基4 FFT) / FftEngine(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分裂基FFT处理器
 */
class SplitRadixFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;      ///< 累计变换次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int lastN = 0;                    ///< 最近变换长度
    };

    explicit SplitRadixFFT(QObject *parent = nullptr);
    ~SplitRadixFFT() override;

    /**
     * @brief 正向FFT
     * @param real 实部输入/输出
     * @param imag 虚部输入/输出
     * @return true=成功
     */
    bool transform(QVector<double>& real, QVector<double>& imag);

    /**
     * @brief 逆向FFT
     * @param real 实部输入/输出
     * @param imag 虚部输入/输出
     * @return true=成功
     */
    bool inverseTransform(QVector<double>& real, QVector<double>& imag);

    /** @brief 检查N是否为2的幂 */
    static bool isPowerOfTwo(int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n);

private:
    /** @brief 递归分裂基蝶形 */
    void splitRadixCore(double* real, double* imag, int n, int stride);

    /** @brief 位反转重排 */
    static void bitReverse(QVector<double>& data);

    /** @brief 计算2的整数次幂对数 */
    static int log2Int(int n);

    Stats m_stats;
    double m_timeSum = 0.0;
};
