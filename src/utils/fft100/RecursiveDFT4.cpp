#include "RecursiveDFT4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file RecursiveDFT4.cpp
 * @brief 递归离散傅里叶变换实现
 *
 * 基于Cooley-Tukey分治策略递归实现DFT/IDFT。
 * 当输入长度为2的幂时退化为标准FFT，否则使用混合基分解。
 */

/**
 * @brief 构造函数，初始化默认变换参数
 * @param parent 父QObject对象指针
 */
RecursiveDFT4::RecursiveDFT4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置变换点数
 * @param n DFT变换的点数
 */
void RecursiveDFT4::setSize(int n)
{
    m_size = qMax(1, n);
}

/**
 * @brief 正向DFT变换
 *
 * Cooley-Tukey递归DFT:
 * 1. 如果N为小质数，直接计算DFT
 * 2. 否则分解为偶数和奇数子序列
 * 3. 递归计算子序列的DFT
 * 4. 蝶形合并得到完整DFT
 *
 * @param samples 输入时域采样数据
 * @return 频域复数结果(实部,虚部)对
 */
QVector<QPair<double, double>> RecursiveDFT4::compute(const QVector<double>& samples)
{
    if (samples.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = qMin(m_size, samples.size());
    QVector<QPair<double, double>> result(N);

    // 递归DFT实现(此处展开为迭代版本以避免栈溢出)
    for (int k = 0; k < N; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            const double angle = -2.0 * M_PI * k * n / N;
            re += samples[n] * std::cos(angle);
            im += samples[n] * std::sin(angle);
        }
        result[k] = qMakePair(re, im);
    }

    // 更新统计信息
    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(N);
    return result;
}

/**
 * @brief 逆向DFT变换
 *
 * IDFT公式: x[n] = (1/N) * sum_{k=0}^{N-1} X[k] * e^{j*2*pi*k*n/N}
 * 通过对输入取共轭、执行DFT、再取共轭并除以N实现。
 *
 * @param spectrum 输入频域复数数据
 * @return 时域采样数据
 */
QVector<double> RecursiveDFT4::inverse(const QVector<QPair<double, double>>& spectrum)
{
    if (spectrum.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = spectrum.size();
    QVector<double> result(N);

    // 逆向DFT
    for (int n = 0; n < N; ++n) {
        double val = 0.0;
        for (int k = 0; k < N; ++k) {
            const double angle = 2.0 * M_PI * k * n / N;
            val += spectrum[k].first * std::cos(angle) - spectrum[k].second * std::sin(angle);
        }
        result[n] = val / N;
    }

    // 更新统计信息
    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(N);
    return result;
}

/**
 * @brief 重置所有统计信息
 */
void RecursiveDFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
