/**
 * @file DHT4.h
 * @brief 离散哈特利变换(cas(x)基函数+自逆性质+快速递归) — Discrete Hartley Transform via cas(x) Basis with Self-Inverse Property and Fast Recursion
 *
 * 功能: 实现离散哈特利变换(DHT)，支持cas(x)=cos(x)+sin(x)基函数、
 *       自逆性质(正反变换相同)和快速递归FHT算法。
 *
 * 协作: Goertzel5(Goertzel) / FftEngine3(FFT) / DCT4(离散余弦)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 离散哈特利变换(cas基+自逆+快速递归)
 */
class DHT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int radixUsed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DHT4(QObject *parent = nullptr);
    ~DHT4() override;

    /** @brief Compute forward DHT (same as inverse due to self-inverse) */
    QVector<double> transform(const QVector<double>& input);

    /** @brief Compute forward DHT in-place */
    void transformInPlace(QVector<double>& data);

    /** @brief Extract real part (equivalent to DCT-related) */
    QVector<double> extractReal(const QVector<double>& hartley) const;

    /** @brief Extract imaginary part (equivalent to DST-related) */
    QVector<double> extractImag(const QVector<double>& hartley) const;

    /** @brief Compute DHT via direct O(N^2) definition */
    QVector<double> directDHT(const QVector<double>& input) const;

    /** @brief Pad to next power-of-2 and transform */
    QVector<double> transformPadded(const QVector<double>& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Fast Hartley Transform (radix-2 decimation-in-time) */
    void fht(QVector<double>& data, int n);

    /** @brief cas(x) = cos(x) + sin(x) */
    static double cas(double x);

    /** @brief Bit-reverse permutation */
    void bitReverse(QVector<double>& data, int n);

    /** @brief Next power of 2 >= n */
    static int nextPow2(int n);
};
