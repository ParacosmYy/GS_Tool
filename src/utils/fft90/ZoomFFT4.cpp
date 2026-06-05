#include "ZoomFFT4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Zoom FFT分析器
 * @param parent 父对象指针
 */
ZoomFFT4::ZoomFFT4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置中心频率(Hz)
 * @param freqHz 分析频段的中心频率
 */
void ZoomFFT4::setCenterFreq(double freqHz)
{
    m_centerFreq = qMax(0.0, freqHz);
}

/**
 * @brief 设置分析带宽(Hz)
 * @param bwHz 分析频段的带宽
 */
void ZoomFFT4::setBandwidth(double bwHz)
{
    m_bandwidth = qMax(1.0, bwHz);
}

/**
 * @brief 对输入信号执行Zoom FFT细化分析
 *
 * 通过数字下变频(频率搬移)、低通滤波和降采样，
 * 在指定频段内获得比标准FFT更高的频率分辨率。
 *
 * @param input 输入时域信号
 * @return 细化后的频谱幅度序列
 */
QVector<double> ZoomFFT4::compute(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> spectrum;
    if (input.size() < 4) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computationCompleted(0);
        return spectrum;
    }

    const int N = input.size();
    const double fs = 44100.0; /* 假设采样率 */

    /* 第一步：频率搬移 - 将中心频率移到DC */
    QVector<double> shifted(N);
    for (int i = 0; i < N; ++i) {
        double phase = 2.0 * M_PI * m_centerFreq * i / fs;
        shifted[i] = input[i] * std::cos(phase);
    }

    /* 第二步：低通滤波(简化为滑动平均) */
    int filterLen = qMax(3, N / 8);
    QVector<double> filtered(N, 0.0);
    for (int i = 0; i < N; ++i) {
        double sum = 0.0;
        int cnt = 0;
        int halfF = filterLen / 2;
        for (int j = qMax(0, i - halfF); j <= qMin(N - 1, i + halfF); ++j) {
            sum += shifted[j];
            cnt++;
        }
        filtered[i] = sum / cnt;
    }

    /* 第三步：降采样 */
    int decFactor = qMax(1, static_cast<int>(fs / (2.0 * m_bandwidth)));
    QVector<double> decimated;
    for (int i = 0; i < N; i += decFactor) {
        decimated.append(filtered[i]);
    }

    /* 第四步：对降采样信号执行DFT */
    int M = decimated.size();
    spectrum.resize(M / 2);
    for (int k = 0; k < M / 2; ++k) {
        double realPart = 0.0;
        double imagPart = 0.0;
        for (int n = 0; n < M; ++n) {
            double angle = 2.0 * M_PI * k * n / M;
            realPart += decimated[n] * std::cos(angle);
            imagPart -= decimated[n] * std::sin(angle);
        }
        spectrum[k] = std::sqrt(realPart * realPart + imagPart * imagPart) / M;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalComputations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
    emit computationCompleted(spectrum.size());
    return spectrum;
}

/**
 * @brief 获取当前频谱分辨率(Hz/bin)
 * @return 频率分辨率
 */
double ZoomFFT4::frequencyResolution() const
{
    double fs = 44100.0;
    return m_bandwidth / qMax(1, static_cast<int>(fs / (2.0 * m_bandwidth)));
}

/**
 * @brief 获取分析频段的起始频率(Hz)
 * @return 起始频率
 */
double ZoomFFT4::startFrequency() const
{
    return m_centerFreq - m_bandwidth / 2.0;
}

/**
 * @brief 获取分析频段的结束频率(Hz)
 * @return 结束频率
 */
double ZoomFFT4::endFrequency() const
{
    return m_centerFreq + m_bandwidth / 2.0;
}

/**
 * @brief 重置统计数据
 */
void ZoomFFT4::resetStatistics()
{
    m_stats.totalComputations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
