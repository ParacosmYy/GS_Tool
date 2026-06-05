/**
 * @file WalshHadamard2.cpp
 * @brief Walsh-Hadamard变换实现 — 快速WHT与频谱分析
 *
 * 实现Walsh-Hadamard变换(WHT)，一种基于±1正交基的信号变换。
 * 与FFT类似，但使用方波(Hadamard矩阵)代替正弦波作为基函数。
 *
 * 特性:
 * - 快速Walsh-Hadamard变换(FWHT): O(N log N)
 * - 正变换与逆变换(逆变换仅需除以N)
 * - 序率排序(Walsh序)转换
 * - Walsh频谱能量分析
 * - 输入长度自动补零到2的幂次
 * - 统计变换次数、处理采样点数、平均耗时
 */

#include "utils/fft39/WalshHadamard2.h"

#include <QElapsedTimer>
#include <QtMath>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 初始化Walsh-Hadamard变换器
 * @param parent QObject父对象
 */
WalshHadamard2::WalshHadamard2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 正向Walsh-Hadamard变换
 *
 * 快速WHT算法(蝶形运算):
 * 1. 将输入长度补零到2的幂次N
 * 2. 执行log2(N)级蝶形运算
 * 3. 每级将数据分为若干对，计算和/差
 *
 * @param input 输入信号
 * @return WHT变换结果(自然序/Hadamard序)
 */
QVector<double> WalshHadamard2::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        updateTimeStats(0);
        return QVector<double>();
    }

    /* 补零到2的幂次 */
    int n = nextPowerOfTwo(input.size());
    QVector<double> data = padToLength(input, n);

    /* 快速WHT蝶形运算 */
    for (int step = 1; step < n; step *= 2) {
        for (int i = 0; i < n; i += 2 * step) {
            for (int j = 0; j < step; ++j) {
                double a = data[i + j];
                double b = data[i + j + step];
                data[i + j] = a + b;
                data[i + j + step] = a - b;
            }
        }
    }

    /* 更新统计信息 */
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += input.size();
    updateTimeStats(timer.elapsed());

    emit transformComplete(n);
    return data;
}

/**
 * @brief 逆Walsh-Hadamard变换
 *
 * 逆WHT与正变换结构相同，最后除以N即可。
 * 因为WHT矩阵H满足 H * H' = N * I
 *
 * @param input WHT变换系数
 * @return 逆变换后的原始信号
 */
QVector<double> WalshHadamard2::inverse(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        updateTimeStats(0);
        return QVector<double>();
    }

    /* 补零到2的幂次 */
    int n = nextPowerOfTwo(input.size());
    QVector<double> data = padToLength(input, n);

    /* 快速WHT蝶形运算(与正变换相同) */
    for (int step = 1; step < n; step *= 2) {
        for (int i = 0; i < n; i += 2 * step) {
            for (int j = 0; j < step; ++j) {
                double a = data[i + j];
                double b = data[i + j + step];
                data[i + j] = a + b;
                data[i + j + step] = a - b;
            }
        }
    }

    /* 逆变换: 除以N */
    double invN = 1.0 / n;
    for (int i = 0; i < n; ++i) {
        data[i] *= invN;
    }

    /* 截断到原始长度 */
    data.resize(input.size());

    /* 更新统计信息 */
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += input.size();
    updateTimeStats(timer.elapsed());

    emit transformComplete(n);
    return data;
}

/**
 * @brief 序率排序转换 — 将自然序(Hadamard序)转换为Walsh序
 *
 * Walsh序按"符号变化次数"(序率/sequency)排列，
 * 类似于FFT按频率排列。通过位反转置换实现排序。
 *
 * @param input 自然序WHT系数
 * @return Walsh序(序率排序)WHT系数
 */
QVector<double> WalshHadamard2::sequencyOrder(const QVector<double>& input) const
{
    if (input.isEmpty()) return QVector<double>();

    int n = nextPowerOfTwo(input.size());
    QVector<double> data = padToLength(input, n);
    QVector<double> result(n, 0.0);

    /* Gray码序到自然序的映射 */
    /* Walsh序等价于Gray码序 */
    for (int i = 0; i < n; ++i) {
        int grayCode = i ^ (i >> 1);
        if (grayCode < n) {
            result[grayCode] = data[i];
        }
    }

    /* 二进制位反转进一步调整 */
    QVector<double> finalResult(n, 0.0);
    int log2n = 0;
    int temp = n;
    while (temp > 1) { temp >>= 1; log2n++; }

    for (int i = 0; i < n; ++i) {
        int rev = bitReverse(i, log2n);
        finalResult[i] = result[rev];
    }

    finalResult.resize(input.size());
    return finalResult;
}

/**
 * @brief Walsh频谱分析 — 计算WHT系数的功率谱
 *
 * 计算每个WHT系数的归一化功率(模平方/N)，
 * 用于分析信号在各个序率分量上的能量分布。
 *
 * @param input 输入信号
 * @return Walsh功率谱(每个分量的归一化功率)
 */
QVector<double> WalshHadamard2::walshSpectrum(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        updateTimeStats(0);
        return QVector<double>();
    }

    /* 先执行正向WHT */
    QVector<double> wht = forward(input);

    /* 计算功率谱 */
    int n = wht.size();
    QVector<double> spectrum(n, 0.0);
    double invN = 1.0 / n;

    for (int i = 0; i < n; ++i) {
        spectrum[i] = (wht[i] * wht[i]) * invN;
    }

    /* 更新统计信息 */
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += input.size();
    updateTimeStats(timer.elapsed());

    return spectrum;
}

/**
 * @brief 重置统计信息
 */
void WalshHadamard2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ===== 私有方法实现 ===== */

/**
 * @brief 计算大于等于n的最小2的幂次
 * @param n 输入值
 * @return 大于等于n的最小2的幂次
 */
int WalshHadamard2::nextPowerOfTwo(int n) const
{
    if (n <= 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/**
 * @brief 将向量补零到指定长度
 * @param input 输入向量
 * @param len 目标长度
 * @return 补零后的向量
 */
QVector<double> WalshHadamard2::padToLength(const QVector<double>& input, int len) const
{
    QVector<double> result = input;
    if (result.size() < len) {
        result.resize(len, 0.0);
    }
    return result;
}

/**
 * @brief 位反转 — 将整数的二进制位反转
 * @param value 输入值
 * @param bits 有效位数
 * @return 位反转后的值
 */
int WalshHadamard2::bitReverse(int value, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

/**
 * @brief 更新时间统计
 * @param elapsed 本轮耗时(ms)
 */
void WalshHadamard2::updateTimeStats(double elapsed) const
{
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs =
        (m_stats.totalTransforms > 0) ? m_timeSum / m_stats.totalTransforms : 0.0;
}
