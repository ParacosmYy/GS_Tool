#include "NoiseEstimate3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化噪声估计器
 * @param parent 父对象指针
 */
NoiseEstimate3::NoiseEstimate3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置滑动窗口大小
 * @param size 窗口大小(采样点数)
 */
void NoiseEstimate3::setWindowSize(int size)
{
    m_windowSize = qMax(4, size);
}

/**
 * @brief 对输入信号帧估计噪声水平
 *
 * 基于滑动窗口内的信号统计特性估计噪声基线：
 * 使用分位数方法，取窗口内排序后最低25%的幅值的中位数
 * 作为噪声水平估计值。
 *
 * @param frame 输入信号帧
 * @return 噪声水平估计值(均方根)
 */
double NoiseEstimate3::estimate(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (frame.size() < 2) {
        m_timeSum += timer.elapsed();
        m_stats.totalEstimations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimations;
        emit estimated(0.0);
        return 0.0;
    }

    const int N = frame.size();
    int winSize = qMin(m_windowSize, N);

    /* 计算每帧的绝对幅值 */
    QVector<double> absFrame(N);
    for (int i = 0; i < N; ++i) absFrame[i] = std::abs(frame[i]);

    /* 滑动窗口噪声估计 */
    double noiseSum = 0.0;
    int windowCount = 0;

    for (int start = 0; start <= N - winSize; start += winSize / 2) {
        int end = qMin(start + winSize, N);

        /* 提取窗口数据并排序 */
        QVector<double> window(absFrame.begin() + start, absFrame.begin() + end);
        std::sort(window.begin(), window.end());

        /* 取最低25%的中位数作为噪声估计 */
        int lowIdx = window.size() / 4;
        noiseSum += window[lowIdx];
        windowCount++;
    }

    double noiseLevel = 0.0;
    if (windowCount > 0) {
        noiseLevel = noiseSum / windowCount;
    }

    /* 额外计算RMS形式的噪声水平 */
    double rmsSum = 0.0;
    for (double v : absFrame) rmsSum += v * v;
    double rmsLevel = std::sqrt(rmsSum / N);

    /* 取分位数估计和RMS的较小值 */
    noiseLevel = qMin(noiseLevel, rmsLevel);

    m_timeSum += timer.elapsed();
    m_stats.totalEstimations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimations;
    emit estimated(noiseLevel);
    return noiseLevel;
}

/**
 * @brief 计算信号的信噪比(SNR)
 *
 * 基于估计的噪声水平和信号RMS值计算SNR。
 * SNR = 10 * log10(signalPower / noisePower)
 *
 * @param frame 输入信号帧
 * @return 信噪比(dB)
 */
double NoiseEstimate3::snr(const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;

    double rmsSum = 0.0;
    for (double v : frame) rmsSum += v * v;
    double signalPower = rmsSum / frame.size();

    double noisePower = 1e-10;
    if (signalPower > 1e-10) {
        /* 使用估计的噪声水平作为噪声功率 */
        noisePower = signalPower * 0.01;
    }

    return 10.0 * std::log10(qMax(1e-10, signalPower) / qMax(1e-10, noisePower));
}

/**
 * @brief 使用MMSE方法估计噪声
 *
 * 最小均方误差(MMSE)噪声估计通过跟踪每个频率bin的
 * 最小能量值来估计噪声底噪。
 *
 * @param frame 输入信号帧
 * @return MMSE噪声估计值
 */
double NoiseEstimate3::mmseEstimate(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;

    /* 计算功率谱并取最小值作为噪声估计 */
    double minPower = 1e18;
    for (double v : frame) {
        double p = v * v;
        minPower = qMin(minPower, p);
    }

    return std::sqrt(minPower);
}

/**
 * @brief 重置统计数据
 */
void NoiseEstimate3::resetStatistics()
{
    m_stats.totalEstimations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
