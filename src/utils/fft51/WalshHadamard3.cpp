/**
 * @file WalshHadamard3.cpp
 * @brief Walsh-Hadamard变换(WHT)实现
 *
 * 实现快速Walsh-Hadamard变换，适用于信号处理中的正交变换。
 * WHT仅使用+1/-1作为基函数，运算只需加减法，计算效率极高。
 * 支持正变换、逆变换和序谱(sequency spectrum)分析。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/fft51/WalshHadamard3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class WalshHadamard3
 * @brief Walsh-Hadamard变换器
 *
 * WHT矩阵由Hadamard矩阵递归定义：H_1 = [1], H_{2n} = [H_n, H_n; H_n, -H_n]。
 * 快速算法(FWT)的复杂度为O(N*logN)，与FFT类似但仅使用加减运算。
 * 正变换和逆变换使用相同算法，逆变换仅需额外除以N归一化。
 */

/**
 * @brief 构造函数，初始化默认变换长度
 * @param parent 父QObject指针
 */
WalshHadamard3::WalshHadamard3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换长度（自动对齐到最近的2的幂）
 * @param n 目标变换长度
 */
void WalshHadamard3::setSize(int n)
{
    m_n = 1;
    while (m_n < n) m_n *= 2;
    if (m_n < 2) m_n = 2;
}

/**
 * @brief 执行正向Walsh-Hadamard变换
 *
 * 使用快速Walsh-Hadamard变换(FWT)算法：
 * 逐级执行蝶形运算，每级将序列分成两半，
 * 前半加后半（偶数分量），前半减后半（奇数分量）。
 *
 * @param input 输入时域序列
 * @return WHT变换系数（整数序列除以归一化因子前为整数）
 */
QVector<double> WalshHadamard3::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> data(n, 0.0);
    for (int i = 0; i < qMin(input.size(), n); ++i) {
        data[i] = input[i];
    }

    /* 执行快速WHT */
    fwt(data, n);

    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(n);
    return data;
}

/**
 * @brief 执行逆向Walsh-Hadamard变换
 *
 * 逆变换使用相同的FWT算法（WHT是自逆的除以归一化因子），
 * 结果除以N进行归一化。
 *
 * @param transformed WHT系数
 * @return 重建的时域序列
 */
QVector<double> WalshHadamard3::inverse(const QVector<double>& transformed)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> data(n, 0.0);
    for (int i = 0; i < qMin(transformed.size(), n); ++i) {
        data[i] = transformed[i];
    }

    /* 逆变换 = 正变换 / N（WHT是自逆的） */
    fwt(data, n);
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
 * @brief 计算序谱(Sequency Spectrum)
 *
 * 序谱按Walsh函数的"序"(过零次数)排列，类似于FFT频谱
 * 按频率排列。序谱的物理意义更直观：低序对应平滑分量，
 * 高序对应快速变化分量。
 *
 * 算法：先执行WHT，然后按序重排系数，最后计算功率谱。
 *
 * @param signal 输入信号
 * @return 序谱功率密度
 */
QVector<double> WalshHadamard3::sequencySpectrum(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;

    /* 执行WHT */
    QVector<double> data(n, 0.0);
    for (int i = 0; i < qMin(signal.size(), n); ++i) {
        data[i] = signal[i];
    }
    fwt(data, n);

    /* 按序重排：计算每个Hadamard序号的序(过零次数) */
    QVector<QPair<int, double>> sequencyOrder(n);
    for (int i = 0; i < n; ++i) {
        /* 计算比特反转后的Gray码值作为序号 */
        int bits = 0;
        int temp = n;
        while (temp > 1) { temp >>= 1; bits++; }
        int rev = 0;
        int x = i;
        for (int b = 0; b < bits; ++b) { rev = (rev << 1) | (x & 1); x >>= 1; }
        /* Gray码转换得到序号 */
        int seq = rev ^ (rev >> 1);
        sequencyOrder[i] = {seq, data[i]};
    }

    /* 按序号排序 */
    std::sort(sequencyOrder.begin(), sequencyOrder.end(),
        [](const QPair<int,double>& a, const QPair<int,double>& b) {
            return a.first < b.first;
        });

    /* 计算功率谱 */
    QVector<double> spectrum(n);
    for (int i = 0; i < n; ++i) {
        double val = sequencyOrder[i].second / n;
        spectrum[i] = val * val;
    }

    m_stats.totalTransforms++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    return spectrum;
}

/**
 * @brief 重置统计数据
 */
void WalshHadamard3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 快速Walsh-Hadamard变换的核心算法
 *
 * 使用蝶形结构逐级分解：
 *   for stride = 1, 2, 4, ..., N/2:
 *     for each pair (i, i+stride):
 *       sum = data[i] + data[i+stride]
 *       diff = data[i] - data[i+stride]
 *       data[i] = sum
 *       data[i+stride] = diff
 *
 * @param data 输入/输出数据，长度必须为2的幂
 * @param n 数据长度
 */
void WalshHadamard3::fwt(QVector<double>& data, int n) const
{
    for (int stride = 1; stride < n; stride <<= 1) {
        for (int i = 0; i < n; i += stride * 2) {
            for (int j = 0; j < stride; ++j) {
                double a = data[i + j];
                double b = data[i + j + stride];
                data[i + j] = a + b;
                data[i + j + stride] = a - b;
            }
        }
    }
}
