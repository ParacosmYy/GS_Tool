#include "MelSpectrogram5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Mel频谱图计算器
 * @param parent 父对象指针
 */
MelSpectrogram5::MelSpectrogram5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率(Hz)
 * @param rate 音频采样率
 */
void MelSpectrogram5::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设置Mel频率箱数
 * @param bins Mel滤波器组的频率箱数(通常40或128)
 */
void MelSpectrogram5::setBinCount(int bins)
{
    m_binCount = qMax(1, bins);
}

/**
 * @brief 将频率(Hz)转换为Mel频率
 * @param freq 频率(Hz)
 * @return Mel频率
 */
static double hzToMel(double freq)
{
    return 2595.0 * std::log10(1.0 + freq / 700.0);
}

/**
 * @brief 将Mel频率转换为频率(Hz)
 * @param mel Mel频率
 * @return 频率(Hz)
 */
static double melToHz(double mel)
{
    return 700.0 * (std::pow(10.0, mel / 2595.0) - 1.0);
}

/**
 * @brief 计算Mel频谱图特征
 *
 * 处理流程：
 * 1. 对输入信号计算DFT幅度谱
 * 2. 构建Mel三角滤波器组
 * 3. 将线性频谱通过Mel滤波器组映射
 * 4. 输出Mel频率能量分布
 *
 * @param samples 输入音频采样
 */
void MelSpectrogram5::compute(const QVector<double>& samples)
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
    int halfN = N / 2;

    /* 第一步：计算DFT幅度谱 */
    QVector<double> powerSpec(halfN, 0.0);
    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += samples[n] * std::cos(angle);
            im += samples[n] * std::sin(angle);
        }
        powerSpec[k] = (re * re + im * im) / (N * N);
    }

    /* 第二步：构建Mel滤波器组 */
    double lowFreq = 0.0;
    double highFreq = m_sampleRate / 2.0;
    double lowMel = hzToMel(lowFreq);
    double highMel = hzToMel(highFreq);

    /* 在Mel空间均匀分布中心点 */
    QVector<double> melPoints(m_binCount + 2);
    for (int i = 0; i < m_binCount + 2; ++i) {
        melPoints[i] = lowMel + (highMel - lowMel) * i / (m_binCount + 1);
    }

    /* 转回Hz空间并映射到频率bin索引 */
    QVector<int> binPoints(m_binCount + 2);
    for (int i = 0; i < m_binCount + 2; ++i) {
        double hz = melToHz(melPoints[i]);
        binPoints[i] = qMin(halfN - 1, static_cast<int>(hz * halfN / (m_sampleRate / 2.0)));
    }

    /* 第三步：应用三角滤波器 */
    QVector<double> melSpectrum(m_binCount, 0.0);
    for (int m = 0; m < m_binCount; ++m) {
        int left = binPoints[m];
        int center = binPoints[m + 1];
        int right = binPoints[m + 2];

        for (int k = left; k <= right; ++k) {
            double weight = 0.0;
            if (k <= center && center > left) {
                weight = static_cast<double>(k - left) / (center - left);
            } else if (k > center && right > center) {
                weight = static_cast<double>(right - k) / (right - center);
            }
            melSpectrum[m] += weight * powerSpec[k];
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
    emit computationCompleted(m_binCount);
}

/**
 * @brief 重置统计数据
 */
void MelSpectrogram5::resetStatistics()
{
    m_stats.totalComputed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
