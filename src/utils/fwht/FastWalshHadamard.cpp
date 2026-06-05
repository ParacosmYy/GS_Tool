/**
 * @file FastWalshHadamard.cpp
 * @brief 快速Walsh-Hadamard变换实现
 */

#include "FastWalshHadamard.h"
#include <QElapsedTimer>
#include <cmath>

FastWalshHadamard::FastWalshHadamard(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> FastWalshHadamard::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            (m_stats.totalTransforms > 0) ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(0);
        return {};
    }

    /* 检查长度是否为2的幂，若不是则补零 */
    QVector<double> data = input;
    if (!isPowerOfTwo(data.size())) {
        int newSize = nextPowerOfTwo(data.size());
        data.resize(newSize, 0.0);
    }

    /* 执行FWHT蝶形运算并归一化 */
    transformInPlace(data, true);

    /* 统计更新 */
    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalTransforms > 0) ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(input.size());
    return data;
}

QVector<double> FastWalshHadamard::inverse(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_stats.totalTransforms++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            (m_stats.totalTransforms > 0) ? m_timeSum / m_stats.totalTransforms : 0.0;
        emit transformCompleted(0);
        return {};
    }

    /* 检查长度是否为2的幂，若不是则补零 */
    QVector<double> data = input;
    if (!isPowerOfTwo(data.size())) {
        int newSize = nextPowerOfTwo(data.size());
        data.resize(newSize, 0.0);
    }

    /* FWHT的逆变换与正变换相同，只需归一化 */
    transformInPlace(data, true);

    /* 统计更新 */
    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalTransforms > 0) ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(input.size());
    return data;
}

void FastWalshHadamard::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

void FastWalshHadamard::transformInPlace(QVector<double>& data,
                                          bool normalize) const
{
    int n = data.size();
    if (n <= 1) return;

    /* 蝶形运算: 与FFT类似的分治结构
     * 每一步将数据分为两半，执行蝶形运算:
     *   a' = a + b
     *   b' = a - b
     * 其中Hadamard矩阵的元素只有+1/-1 */
    for (int step = 1; step < n; step <<= 1) {
        for (int i = 0; i < n; i += (step << 1)) {
            for (int j = 0; j < step; ++j) {
                double a = data[i + j];
                double b = data[i + j + step];
                data[i + j] = a + b;
                data[i + j + step] = a - b;
            }
        }
    }

    /* 归一化: 除以sqrt(n)
     * 正变换和逆变换都需要此归一化 */
    if (normalize) {
        double invSqrtN = 1.0 / std::sqrt(static_cast<double>(n));
        for (int i = 0; i < n; ++i)
            data[i] *= invSqrtN;
    }
}

bool FastWalshHadamard::isPowerOfTwo(int n) const
{
    return n > 0 && (n & (n - 1)) == 0;
}

int FastWalshHadamard::nextPowerOfTwo(int n) const
{
    if (n <= 1) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}
