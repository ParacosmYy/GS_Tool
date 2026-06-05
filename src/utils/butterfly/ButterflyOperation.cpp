/**
 * @file ButterflyOperation.cpp
 * @brief 蝶形运算实现 — 基2 FFT/IFFT
 */

#include "utils/butterfly/ButterflyOperation.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
ButterflyOperation::ButterflyOperation(QObject* parent)
    : QObject(parent)
{
}

/** @brief 计算比特逆序索引 */
int ButterflyOperation::reverseBits(int x, int log2n) const
{
    int result = 0;
    for (int i = 0; i < log2n; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/** @brief 比特逆序重排 */
QVector<std::complex<double>> ButterflyOperation::bitReverse(
    const QVector<std::complex<double>>& data) const
{
    int n = data.size();
    if (n <= 1) return data;

    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    QVector<std::complex<double>> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = data[reverseBits(i, log2n)];

    return result;
}

/** @brief FFT/IFFT蝶形运算 */
QVector<std::complex<double>> ButterflyOperation::compute(
    const QVector<std::complex<double>>& data, bool inverse)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n <= 1) return data;

    /* 检查是否为2的幂 */
    if ((n & (n - 1)) != 0) return data;

    int log2n = 0;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; log2n++; }

    QVector<std::complex<double>> x(n);
    for (int i = 0; i < n; ++i)
        x[i] = data[reverseBits(i, log2n)];

    double sign = inverse ? 1.0 : -1.0;

    /* 逐级蝶形运算 */
    for (int s = 1; s <= log2n; ++s) {
        int m = 1 << s;
        double angle = sign * 2.0 * M_PI / m;
        std::complex<double> wm(std::cos(angle), std::sin(angle));

        for (int k = 0; k < n; k += m) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < m / 2; ++j) {
                std::complex<double> t = w * x[k + j + m / 2];
                std::complex<double> u = x[k + j];
                x[k + j] = u + t;
                x[k + j + m / 2] = u - t;
                w *= wm;
            }
        }
    }

    /* IFFT需要除以N */
    if (inverse) {
        for (int i = 0; i < n; ++i)
            x[i] /= static_cast<double>(n);
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n);
    return x;
}

/** @brief 重置统计 */
void ButterflyOperation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
