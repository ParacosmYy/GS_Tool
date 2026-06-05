#include "WalshHadamard5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class WalshHadamard5
 * @brief Walsh-Hadamard变换实现
 *
 * Walsh-Hadamard变换(WHT)是一种广义的傅里叶变换，
 * 使用+1/-1的正交基函数，不需要乘法运算(仅加减法)。
 * 时间复杂度O(N log N)，适合信号处理和压缩。
 *
 * 变换矩阵通过递归构造:
 * H_1 = [1]
 * H_2n = [H_n,  H_n]
 *        [H_n, -H_n]
 *
 * 正变换无归一化，逆变换除以N归一化。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
WalshHadamard5::WalshHadamard5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算下一个大于等于n的2的幂
 * @param n 输入值
 * @return 大于等于n的最小2的幂
 */
static int nextPowerOf2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/**
 * @brief 正变换
 *
 * 快速Walsh-Hadamard变换(FWHT):
 * 类似FFT的蝶形结构，但仅使用加减法。
 * 逐层处理: 每层将数组分为两半，前半做和，后半做差。
 *
 * @param input 输入向量(长度自动补零到2的幂)
 * @return 变换结果向量
 */
QVector<double> WalshHadamard5::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /* 补零到2的幂 */
    int N = nextPowerOf2(input.size());
    QVector<double> output(N, 0.0);
    for (int i = 0; i < input.size(); ++i) {
        output[i] = input[i];
    }

    /* 快速Walsh-Hadamard变换 */
    for (int stride = 1; stride < N; stride <<= 1) {
        for (int i = 0; i < N; i += 2 * stride) {
            for (int j = 0; j < stride; ++j) {
                double a = output[i + j];
                double b = output[i + j + stride];
                output[i + j] = a + b;
                output[i + j + stride] = a - b;
            }
        }
    }

    /* 计算能量 */
    double energy = 0.0;
    for (int i = 0; i < N; ++i) {
        energy += output[i] * output[i];
    }

    m_stats.totalTransforms++;
    m_stats.totalElementsProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalTransforms);

    emit transformCompleted(N, energy);

    return output;
}

/**
 * @brief 逆变换(归一化)
 *
 * WHT的逆变换与正变换结构相同，但需要除以N归一化。
 * 因为H_N * H_N = N * I，所以逆变换 = (1/N) * H_N * x。
 *
 * @param input 变换域向量
 * @return 重建的原始信号向量
 */
QVector<double> WalshHadamard5::inverse(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    int N = nextPowerOf2(input.size());
    QVector<double> output(N, 0.0);
    for (int i = 0; i < input.size(); ++i) {
        output[i] = input[i];
    }

    /* 执行正变换(结构相同) */
    for (int stride = 1; stride < N; stride <<= 1) {
        for (int i = 0; i < N; i += 2 * stride) {
            for (int j = 0; j < stride; ++j) {
                double a = output[i + j];
                double b = output[i + j + stride];
                output[i + j] = a + b;
                output[i + j + stride] = a - b;
            }
        }
    }

    /* 归一化: 除以N */
    for (int i = 0; i < N; ++i) {
        output[i] /= N;
    }

    /* 计算能量 */
    double energy = 0.0;
    for (int i = 0; i < N; ++i) {
        energy += output[i] * output[i];
    }

    m_stats.totalTransforms++;
    m_stats.totalElementsProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalTransforms);

    emit transformCompleted(N, energy);

    return output;
}

/**
 * @brief 重置所有统计数据
 *
 * 将变换计数、元素处理计数和计时归零。
 */
void WalshHadamard5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
