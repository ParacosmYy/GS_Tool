/**
 * @file DCT4.h
 * @brief 离散余弦变换IV型(半移位DFT+快速递推) — Discrete Cosine Transform Type-IV via Half-shift DFT and Fast Recursion
 *
 * 功能: 实现DCT-IV变换，支持半移位DFT方法、快速递推算法、
 *       正交归一化和批量变换处理。
 *
 * 协作: DCT1(DCT-I) / MDCT6(MDCT) / FFTEngine(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DCT-IV处理器(半移位DFT+快速递推)
 */
class DCT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        bool normalized = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DCT4(QObject *parent = nullptr);
    ~DCT4() override;

    void setNormalized(bool enabled);

    /** @brief 正向DCT-IV变换 */
    QVector<double> forward(const QVector<double>& input);

    /** @brief 逆向DCT-IV变换(DCT-IV是自逆的) */
    QVector<double> inverse(const QVector<double>& input);

    /** @brief 通过半移位DFT计算DCT-IV */
    QVector<double> viaHalfShiftDFT(const QVector<double>& input) const;

    /** @brief 快速递推DCT-IV */
    QVector<double> fastRecurse(const QVector<double>& input) const;

    /** @brief 生成DCT-IV变换矩阵 */
    QVector<QVector<double>> transformMatrix(int N) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    bool m_normalized = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Next power of 2 */
    int nextPow2(int n) const;

    /** @brief Radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, bool inv) const;
};
