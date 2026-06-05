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
 * @brief 重置统计数据
 */
void NoiseEstimate3::resetStatistics()
{
    m_stats.totalEstimations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
