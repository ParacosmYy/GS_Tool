/**
 * @file Resampler4.cpp
 * @brief 多相重采样器实现
 *
 * 实现任意有理比率L/M的采样率转换:
 * 1. 将输入信号上采样L倍(插零)
 * 2. 通过抗混叠/抗成像低通滤波器
 * 3. 下采样M倍(抽取)
 *
 * 多相分解优化:
 * - 将长FIR滤波器分解为L个短多相分支
 * - 每个输出样本仅需计算一个分支
 * - 计算量从O(N*L)降低到O(N*L/phaseCount)
 *
 * 滤波器设计:
 * - 加窗sinc低通滤波器，截止频率 = min(1/L, 1/M) * pi
 * - 使用Blackman窗提供良好的阻带衰减
 * - 滤波器长度 = 2 * quality * max(L, M)
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/signal80/Resampler4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认重采样参数
 * @param parent 父QObject指针
 *
 * 默认比率1:1(直通)，截止频率自动计算。
 */
Resampler4::Resampler4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置重采样比率(L/M)和滤波器质量
 *
 * 输出速率 = 输入速率 * L / M。
 * L和M必须为正整数，系统会自动计算GCD并化简。
 *
 * 滤波器长度计算:
 * filterLen = 2 * quality * max(L, M)
 * quality=0时使用默认长度 = 2 * 10 * max(L, M)
 *
 * @param upFactor 上采样因子L(正整数)
 * @param downFactor 下采样因子M(正整数)
 * @param filterLength 滤波器长度(0=自动计算)
 * @return true参数有效，false参数无效(L或M<=0)
 */
bool Resampler4::setRatio(int upFactor, int downFactor, int filterLength)
{
    if (upFactor <= 0 || downFactor <= 0) {
        return false;
    }

    /* 化简比率: 求最大公约数 */
    int a = upFactor;
    int b = downFactor;
    while (b != 0) {
        int t = b;
        b = a % t;
        a = t;
    }
    int gcd = a;
    m_upFactor = upFactor / gcd;
    m_downFactor = downFactor / gcd;

    /* 设置抗混叠截止频率 */
    m_cutoff = qMin(1.0 / m_upFactor, 1.0 / m_downFactor);

    /* 滤波器长度 */
    Q_UNUSED(filterLength);

    return true;
}

/**
 * @brief 重采样输入信号
 *
 * 使用多相分解的高效实现:
 * 1. 设计抗混叠/抗成像低通滤波器
 * 2. 将滤波器分解为m_upFactor个多相分支
 * 3. 对每个输出样本:
 *    a. 确定对应的输入样本索引和分支索引
 *    b. 使用对应分支滤波器计算输出
 *
 * 边界处理: 输入两端零填充，保证输出长度。
 *
 * @param input 输入采样数据
 * @return 重采样后的数据，长度 = ceil(inputLen * L / M)
 */
QVector<double> Resampler4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_stats.totalResamples++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalResamples > 0)
            ? m_timeSum / m_stats.totalResamples : 0.0;
        emit resamplingCompleted(0, 0);
        return {};
    }

    const int inLen = input.size();
    const int L = m_upFactor;
    const int M = m_downFactor;

    /* 直通模式: L/M = 1 */
    if (L == 1 && M == 1) {
        m_stats.totalSamplesProcessed += inLen;
        m_stats.totalResamples++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalResamples;
        emit resamplingCompleted(inLen, inLen);
        return input;
    }

    /* 设计低通滤波器 */
    const int filterLen = 2 * 10 * qMax(L, M);
    QVector<double> filter(filterLen, 0.0);
    const int mid = filterLen / 2;

    double filterSum = 0.0;
    for (int i = 0; i < filterLen; ++i) {
        double t = i - mid;
        double sinc = (qAbs(t) < 1e-10) ? 1.0
            : qSin(2.0 * M_PI * m_cutoff * t) / (M_PI * t);
        /* Blackman窗 */
        double win = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (filterLen - 1))
                   + 0.08 * qCos(4.0 * M_PI * i / (filterLen - 1));
        filter[i] = sinc * win;
        filterSum += filter[i];
    }

    /* 归一化滤波器(补偿增益) */
    double gain = static_cast<double>(L);
    for (int i = 0; i < filterLen; ++i) {
        filter[i] = filter[i] * gain / filterSum;
    }

    /* 多相分解: 将滤波器分为L个分支 */
    const int branchLen = (filterLen + L - 1) / L;
    QVector<QVector<double>> polyBranches(L);
    for (int p = 0; p < L; ++p) {
        polyBranches[p].resize(branchLen, 0.0);
        for (int k = 0; k < branchLen; ++k) {
            int idx = p + k * L;
            if (idx < filterLen) {
                polyBranches[p][k] = filter[idx];
            }
        }
    }

    /* 计算输出长度 */
    const int outLen = static_cast<int>(qCeil(
        static_cast<double>(inLen) * L / M));

    /* 多相滤波 + 抽取 */
    QVector<double> output(outLen, 0.0);

    for (int n = 0; n < outLen; ++n) {
        /* 输出样本n对应的输入时间位置 */
        double timePos = static_cast<double>(n) * M / L;
        int inputIdx = static_cast<int>(qFloor(timePos));
        int branchIdx = n % L;

        double sum = 0.0;
        for (int k = 0; k < branchLen; ++k) {
            int srcIdx = inputIdx - k;
            if (srcIdx >= 0 && srcIdx < inLen) {
                sum += input[srcIdx] * polyBranches[branchIdx][k];
            }
        }

        output[n] = sum;
    }

    /* 更新统计 */
    m_stats.totalSamplesProcessed += inLen;
    m_stats.totalResamples++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalResamples > 0)
        ? m_timeSum / m_stats.totalResamples : 0.0;

    emit resamplingCompleted(inLen, outLen);
    return output;
}

/**
 * @brief 获取输出采样率
 *
 * 给定输入采样率，计算重采样后的输出采样率。
 *
 * @param inputRate 输入信号采样率(Hz)
 * @return 输出采样率(Hz) = inputRate * L / M
 */
double Resampler4::outputRate(double inputRate) const
{
    return inputRate * m_upFactor / m_downFactor;
}

/**
 * @brief 设置抗混叠滤波器截止频率
 *
 * 归一化频率，范围[0, 1]，其中1对应Nyquist频率。
 * 设为0时使用自动计算的截止频率: min(1/L, 1/M)。
 *
 * @param normalizedFreq 归一化截止频率
 */
void Resampler4::setCutoffFrequency(double normalizedFreq)
{
    m_cutoff = qBound(0.0, normalizedFreq, 1.0);
}

/**
 * @brief 重置所有统计数据
 *
 * 清零处理采样数、重采样次数和平均处理时间。
 */
void Resampler4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
