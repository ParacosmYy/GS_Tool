#include "Tonality4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Tonality4.cpp
 * @brief 音调性分析器实现
 *
 * 通过自相关函数(ACF)和频谱峰值检测分析信号的音调特性。
 * 音调性指标范围[0,1]: 0=纯噪声, 1=纯音调。
 */

/**
 * @brief 构造函数，初始化默认分析参数
 * @param parent 父QObject对象指针
 */
Tonality4::TonalityTonality4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析帧大小
 * @param size 每帧的采样点数
 */
void Tonality4::setFrameSize(int size)
{
    m_frameSize = qMax(2, size);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void Tonality4::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 计算音调性指标
 *
 * 处理流程:
 * 1. 计算信号的自相关函数(ACF)
 * 2. 在基频范围内搜索ACF峰值
 * 3. 峰值与零延迟ACF值之比作为音调性指标
 * 4. 辅以频谱峰值验证
 *
 * @param samples 输入音频采样数据
 * @return 音调性值，范围[0,1]
 */
double Tonality4::compute(const QVector<double>& samples)
{
    if (samples.size() < 4) return 0.0;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const int maxLag = qMin(N / 2, static_cast<int>(m_sampleRate / 50.0));
    const int minLag = qMax(2, static_cast<int>(m_sampleRate / m_frameSize));

    // 步骤1: 计算自相关函数
    QVector<double> acf(maxLag + 1, 0.0);
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            sum += samples[i] * samples[i + lag];
        }
        acf[lag] = sum / (N - lag);
    }

    // 步骤2: 在基频范围内搜索ACF峰值
    double peakAcf = 0.0;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        // 检查是否为局部峰值
        if (lag > minLag && lag < maxLag) {
            if (acf[lag] > acf[lag - 1] && acf[lag] > acf[lag + 1]) {
                peakAcf = qMax(peakAcf, acf[lag]);
            }
        }
    }

    // 步骤3: 计算音调性
    double tonality = 0.0;
    if (std::fabs(acf[0]) > 1e-12) {
        tonality = qBound(0.0, peakAcf / acf[0], 1.0);
    }

    // 步骤4: 频谱峰值增强
    double spectralPeak = 0.0;
    double spectralMean = 0.0;
    for (int k = 0; k < N / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            const double angle = -2.0 * M_PI * k * n / N;
            re += samples[n] * std::cos(angle);
            im += samples[n] * std::sin(angle);
        }
        const double mag = std::sqrt(re * re + im * im);
        spectralPeak = qMax(spectralPeak, mag);
        spectralMean += mag;
    }
    spectralMean /= (N / 2);

    // 综合音调性指标
    if (spectralMean > 1e-12) {
        const double spectralTonality = qBound(0.0,
            1.0 - spectralMean / spectralPeak, 1.0);
        tonality = 0.6 * tonality + 0.4 * spectralTonality;
    }

    tonality = qBound(0.0, tonality, 1.0);

    // 更新统计信息
    m_stats.totalFrames++;
    const double prevAvg = m_stats.avgTonality;
    m_stats.avgTonality = prevAvg + (tonality - prevAvg) / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit computed(tonality);
    return tonality;
}

/**
 * @brief 重置所有统计信息
 */
void Tonality4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
