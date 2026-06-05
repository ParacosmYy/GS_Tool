#include "ConstantQ4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化常数Q变换处理器
 * @param parent 父QObject对象指针
 */
ConstantQ4::ConstantQ4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行恒Q变换(CQT)
 *
 * CQT在低频段具有更精细的频率分辨率（更多FFT bin），
 * 在高频段分辨率较粗，呈对数分布。这使得它比线性FFT
 * 更适合音乐信号分析，因为音阶本身就是对数分布的。
 *
 * 使用简化的频域计算方法，对每个频段计算能量。
 *
 * @param samples 输入采样数据
 * @param sampleRate 采样率(Hz)
 * @param minFreq 最低频率(Hz)
 * @param maxFreq 最高频率(Hz)
 * @param binsPerOctave 每八度的频带数，默认12(半音)
 * @return 各频带的能量值
 */
QVector<double> ConstantQ4::transform(const QVector<double>& samples, double sampleRate,
                                       double minFreq, double maxFreq, int binsPerOctave)
{
    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    if (N == 0 || minFreq <= 0.0 || maxFreq <= minFreq) return {};

    /// 计算总八度数和频带数
    const double octaves = std::log2(maxFreq / minFreq);
    const int totalBins = static_cast<int>(std::ceil(octaves * binsPerOctave));

    /// 预计算各频带的中心频率
    m_binFreqs.resize(totalBins);
    for (int k = 0; k < totalBins; ++k) {
        m_binFreqs[k] = minFreq * std::pow(2.0, static_cast<double>(k) / binsPerOctave);
    }

    /// 预计算FFT（简化DFT）
    QVector<double> magnitudes(totalBins);
    const double Q = 1.0 / (std::pow(2.0, 1.0 / binsPerOctave) - 1.0);

    for (int k = 0; k < totalBins; ++k) {
        double freq = m_binFreqs[k];
        int windowSize = qMin(N, static_cast<int>(Q * sampleRate / freq));

        /// 窗口边界
        int start = qMax(0, (N - windowSize) / 2);
        double real = 0.0, imag = 0.0;

        /// 计算该频带的DFT值（加汉宁窗）
        for (int i = 0; i < windowSize; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (windowSize - 1)));
            double sample = samples[start + i] * window;
            real += sample * std::cos(2.0 * M_PI * freq * t);
            imag -= sample * std::sin(2.0 * M_PI * freq * t);
        }

        magnitudes[k] = std::sqrt(real * real + imag * imag) / windowSize;
    }

    /// 更新统计信息
    m_stats.totalTransforms++;
    m_stats.totalBinsComputed += totalBins;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    double freqRange = maxFreq - minFreq;
    emit transformCompleted(totalBins, freqRange);
    return magnitudes;
}

/**
 * @brief 获取各频带的中心频率列表
 *
 * 在transform()之后调用，返回实际计算使用的频带中心频率。
 * 频率按对数等间隔分布。
 *
 * @return 频带中心频率向量(Hz)
 */
QVector<double> ConstantQ4::binFrequencies() const
{
    return m_binFreqs;
}

/**
 * @brief 获取当前统计数据
 * @return 包含变换次数、频带数和平均耗时的Stats结构
 */
ConstantQ4::Stats ConstantQ4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值，清除频带缓存
 */
void ConstantQ4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_binFreqs.clear();
}
