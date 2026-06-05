#include "ConstantQ5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化常数Q变换计算器
 * @param parent 父对象指针
 */
ConstantQ5::ConstantQ5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置最低分析频率(Hz)
 * @param freq 最低频率(如27.5Hz=A0)
 */
void ConstantQ5::setMinFreq(double freq)
{
    m_minFreq = qMax(1.0, freq);
}

/**
 * @brief 设置最高分析频率(Hz)
 * @param freq 最高频率(如4186Hz=C8)
 */
void ConstantQ5::setMaxFreq(double freq)
{
    m_maxFreq = qMax(m_minFreq, freq);
}

/**
 * @brief 设置每八度频率箱数
 * @param bins 每八度的频率分辨率
 */
void ConstantQ5::setBins(int bins)
{
    m_bins = qMax(1, bins);
}

/**
 * @brief 计算常数Q变换
 *
 * 常数Q变换的频率按对数间隔排列：
 * f_k = f_min * 2^(k/B)
 * 每个频率箱的Q值恒定: Q = f_k / delta_f
 * 通过对每个频率箱应用不同长度的窗函数实现。
 *
 * @param samples 输入音频采样
 */
void ConstantQ5::compute(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        emit computationCompleted(0);
        return;
    }

    const int N = samples.size();
    double sampleRate = 44100.0;

    /* 计算总频率箱数 */
    int numOctaves = static_cast<int>(std::ceil(std::log2(m_maxFreq / m_minFreq)));
    int totalBins = numOctaves * m_bins;

    /* 计算每个频率箱的CQ值 */
    double Q = 1.0 / (std::pow(2.0, 1.0 / m_bins) - 1.0);

    for (int k = 0; k < totalBins; ++k) {
        double freq = m_minFreq * std::pow(2.0, static_cast<double>(k) / m_bins);

        /* 计算该频率对应的窗口长度 */
        int windowLen = static_cast<int>(std::round(sampleRate * Q / freq));
        windowLen = qMin(windowLen, N);

        /* 计算该频率的DFT */
        double realPart = 0.0, imagPart = 0.0;
        for (int n = 0; n < windowLen; ++n) {
            /* 汉宁窗 */
            double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / windowLen));
            double angle = -2.0 * M_PI * freq * n / sampleRate;
            realPart += samples[n] * window * std::cos(angle);
            imagPart += samples[n] * window * std::sin(angle);
        }

        /* 幅度 */
        double magnitude = std::sqrt(realPart * realPart + imagPart * imagPart);
        Q_UNUSED(magnitude)
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
    emit computationCompleted(totalBins);
}

/**
 * @brief 获取常数Q值
 * @return Q值(频率分辨率参数)
 */
double ConstantQ5::qValue() const
{
    return 1.0 / (std::pow(2.0, 1.0 / m_bins) - 1.0);
}

/**
 * @brief 获取总频率箱数
 * @return 频率箱数量
 */
int ConstantQ5::totalBins() const
{
    int numOctaves = static_cast<int>(std::ceil(std::log2(m_maxFreq / m_minFreq)));
    return numOctaves * m_bins;
}

/**
 * @brief 获取指定频率箱的中心频率
 * @param binIndex 频率箱索引
 * @return 中心频率(Hz)
 */
double ConstantQ5::binFrequency(int binIndex) const
{
    return m_minFreq * std::pow(2.0, static_cast<double>(binIndex) / m_bins);
}

/**
 * @brief 重置统计数据
 */
void ConstantQ5::resetStatistics()
{
    m_stats.totalComputed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
