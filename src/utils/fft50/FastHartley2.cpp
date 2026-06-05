/**
 * @file FastHartley2.cpp
 * @brief 快速Hartley变换(FHT)实现
 *
 * 实现离散Hartley变换(DHT)的快速算法，支持任意2的幂次长度。
 * DHT将实数序列变换为实数频谱，相比FFT避免了复数运算。
 * 提供正变换、逆变换和快速卷积功能。使用QElapsedTimer计时。
 */

#include "utils/fft50/FastHartley2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @class FastHartley2
 * @brief 快速Hartley变换器，基于Cooley-Tukey蝶形结构
 *
 * Hartley变换的核心基函数为 cas(v) = cos(v) + sin(v)。
 * 正变换和逆变换使用完全相同的算法，区别仅在于逆变换
 * 需要除以N进行归一化。
 */

/**
 * @brief 构造函数，初始化默认变换长度
 * @param parent 父QObject指针
 */
FastHartley2::FastHartley2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换长度（自动对齐到最近的2的幂）
 * @param n 目标变换长度，实际使用 >= n 的最小2的幂
 */
void FastHartley2::setSize(int n)
{
    m_n = 1;
    while (m_n < n) m_n *= 2;
    if (m_n < 4) m_n = 4;
}

/**
 * @brief 执行正向Hartley变换
 *
 * 输入序列经比特反转重排后，通过Cooley-Tukey蝶形运算
 * 得到Hartley频谱 H[k] = sum_n x[n] * cas(2*pi*n*k/N)。
 *
 * @param input 输入时域序列
 * @return Hartley变换频谱（实数序列）
 */
QVector<double> FastHartley2::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> data(n, 0.0);
    for (int i = 0; i < qMin(input.size(), n); ++i) {
        data[i] = input[i];
    }

    /* 执行FHT */
    fht(data, n);

    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(n);
    return data;
}

/**
 * @brief 执行逆向Hartley变换
 *
 * 逆变换使用相同的FHT算法，结果除以N归一化。
 *
 * @param transformed Hartley频谱
 * @return 重建的时域序列
 */
QVector<double> FastHartley2::inverse(const QVector<double>& transformed)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> data(n, 0.0);
    for (int i = 0; i < qMin(transformed.size(), n); ++i) {
        data[i] = transformed[i];
    }

    /* 逆变换 = 正变换 / N */
    fht(data, n);
    for (int i = 0; i < n; ++i) {
        data[i] /= n;
    }

    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(n);
    return data;
}

/**
 * @brief 基于Hartley变换的快速卷积
 *
 * 利用Hartley域的卷积定理：两个序列的Hartley变换经过
 * 特定的蝶形组合后逆变换得到时域卷积结果。
 *
 * @param a 第一个序列
 * @param b 第二个序列
 * @return 卷积结果（长度为m_n）
 */
QVector<double> FastHartley2::convolution(const QVector<double>& a, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;

    /* 零填充并分别进行Hartley变换 */
    QVector<double> ha(n, 0.0), hb(n, 0.0);
    for (int i = 0; i < qMin(a.size(), n); ++i) ha[i] = a[i];
    for (int i = 0; i < qMin(b.size(), n); ++i) hb[i] = b[i];

    fht(ha, n);
    fht(hb, n);

    /* Hartley域卷积：利用 H_c = 0.5*(Ha*Hb + Ha[N-]*Hb[N+] + Ha[N+]*Hb[N-]) */
    QVector<double> hc(n);
    for (int k = 0; k < n; ++k) {
        int km = (n - k) % n;     /* H_b[k-] */
        int kp = (k == 0) ? 0 : n - k;  /* H_b[k+] 的索引 */
        hc[k] = 0.5 * (ha[k] * hb[k]
                      + ha[km] * hb[kp]
                      + ha[kp] * hb[km]);
    }

    /* 逆变换得到卷积结果 */
    fht(hc, n);
    for (int i = 0; i < n; ++i) {
        hc[i] /= n;
    }

    m_stats.totalTransforms += 2;  /* 两次正变换 + 一次逆变换 */
    m_stats.totalPoints += n * 3;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    return hc;
}

/**
 * @brief 重置统计数据
 */
void FastHartley2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 执行快速Hartley变换的核心算法
 *
 * 采用Cooley-Tukey蝶形结构，先进行比特反转重排，
 * 然后逐级执行蝶形运算。Hartley变换的蝶形运算符为：
 *   H_even[k] + H_odd[k]*cos(2*pi*k/N) + H_odd[N-k]*sin(2*pi*k/N)
 *
 * @param data 输入/输出数据，长度必须为2的幂
 * @param n 数据长度
 */
void FastHartley2::fht(QVector<double>& data, int n) const
{
    /* 计算级数 */
    int log2n = 0;
    int temp = n;
    while (temp > 1) { temp >>= 1; log2n++; }

    /* 比特反转重排 */
    for (int i = 0; i < n; ++i) {
        int j = reverseBits(i, log2n);
        if (j > i) {
            std::swap(data[i], data[j]);
        }
    }

    /* 逐级蝶形运算 */
    for (int s = 1; s <= log2n; ++s) {
        int m = 1 << s;       /* 当前蝶形组大小 */
        int halfM = m >> 1;   /* 蝶形半组大小 */

        for (int r = 0; r < n; r += m) {
            for (int k = 0; k < halfM; ++k) {
                int idx1 = r + k;
                int idx2 = r + k + halfM;

                double angle = 2.0 * M_PI * k / m;
                double cosVal = qCos(angle);
                double sinVal = qSin(angle);

                double even = data[idx1];
                double odd = data[idx2];

                /* Hartley蝶形运算 */
                data[idx1] = even + odd * cosVal + odd * sinVal;

                /* 计算互补分量 */
                int compIdx = (m - k) % m;
                double angleComp = 2.0 * M_PI * compIdx / m;
                data[idx2] = even + odd * qCos(angleComp) + odd * qSin(angleComp);
            }
        }
    }
}

/**
 * @brief 反转整数的低bits位
 * @param x 输入整数
 * @param bits 要反转的位数
 * @return 反转后的整数
 */
int FastHartley2::reverseBits(int x, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}
