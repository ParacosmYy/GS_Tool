#include "ConstantQ6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file ConstantQ6.cpp
 * @brief 常数Q变换(Constant-Q Transform)实现
 *
 * CQT在对数频率尺度上等Q值分析:
 * Q = f_k / delta_f_k = 常数
 * 低频段频率分辨率高，高频段时间分辨率高，
 * 符合音乐信号的感知特性。
 */

/**
 * @brief 构造函数，初始化默认频率范围
 * @param parent 父QObject对象指针
 */
ConstantQ6::ConstantQ6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置最低分析频率
 * @param freq 最低频率(Hz)，如27.5(A0)
 */
void ConstantQ6::setMinFreq(double freq)
{
    m_minFreq = qMax(1.0, freq);
}

/**
 * @brief 设置最高分析频率
 * @param freq 最高频率(Hz)，如4186(C8)
 */
void ConstantQ6::setMaxFreq(double freq)
{
    m_maxFreq = qMax(m_minFreq, freq);
}

/**
 * @brief 设置每八度的频率箱数
 * @param binsPerOctave 每八度内的频率箱数(如24)
 */
void ConstantQ6::setBins(int binsPerOctave)
{
    m_bins = qMax(1, binsPerOctave);
}

/**
 * @brief 计算常数Q频谱
 *
 * CQT计算步骤:
 * 1. 计算总频率箱数: totalBins = bins * log2(fmax/fmin)
 * 2. 对每个频率箱k计算中心频率: f_k = fmin * 2^(k/bins)
 * 3. 计算窗长度: N_k = Q * fs / f_k (Q为品质因子)
 * 4. 对每段加窗信号计算与复指数的内积
 *
 * @param samples 输入时域采样数据
 * @return CQT系数向量(复数幅度)
 */
QVector<double> ConstantQ6::compute(const QVector<double>& samples)
{
    if (samples.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const double sampleRate = 44100.0;

    // 计算总频率箱数
    const int totalBins = static_cast<int>(
        std::ceil(m_bins * std::log2(m_maxFreq / m_minFreq)));

    // Q值
    const double Q = 1.0 / (std::pow(2.0, 1.0 / m_bins) - 1.0);

    QVector<double> result(totalBins, 0.0);

    for (int k = 0; k < totalBins; ++k) {
        // 计算中心频率
        const double fk = m_minFreq * std::pow(2.0, static_cast<double>(k) / m_bins);

        // 计算窗长度
        const int windowLen = qMin(static_cast<int>(Q * sampleRate / fk), N);

        if (windowLen <= 0) continue;

        // 计算与复指数的内积(汉宁窗 + 复指数)
        const int offset = qMax(0, N / 2 - windowLen / 2);
        double re = 0.0, im = 0.0;

        for (int n = 0; n < windowLen; ++n) {
            // 汉宁窗
            const double win = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / windowLen));
            // 复指数
            const double angle = 2.0 * M_PI * fk * n / sampleRate;
            const int idx = offset + n;
            if (idx < N) {
                re += samples[idx] * win * std::cos(angle);
                im -= samples[idx] * win * std::sin(angle);
            }
        }

        // 归一化
        const double norm = windowLen * 0.5;
        result[k] = std::sqrt(re * re + im * im) / norm;
    }

    // 更新统计信息
    m_stats.totalComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computationCompleted(totalBins);
    return result;
}

/**
 * @brief 重置所有统计信息
 */
void ConstantQ6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
