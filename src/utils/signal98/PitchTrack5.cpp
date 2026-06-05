#include "PitchTrack5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化音高追踪器
 * @param parent 父对象指针
 */
PitchTrack5::PitchTrack5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置最小追踪频率(Hz)
 * @param freq 最小基频(如50Hz)
 */
void PitchTrack5::setMinFreq(double freq)
{
    m_minFreq = qMax(10.0, freq);
}

/**
 * @brief 设置最大追踪频率(Hz)
 * @param freq 最大基频(如2000Hz)
 */
void PitchTrack5::setMaxFreq(double freq)
{
    m_maxFreq = qMax(m_minFreq, freq);
}

/**
 * @brief 对输入信号执行音高追踪(自相关法)
 *
 * 自相关音高检测算法：
 * 1. 计算信号的自相关函数 R(tau) = sum(x[n]*x[n+tau])
 * 2. 在[minLag, maxLag]范围内搜索自相关峰值
 * 3. 使用抛物线插值精确定位峰值位置
 * 4. 基频 = sampleRate / peakLag
 *
 * @param samples 输入音频采样
 */
void PitchTrack5::track(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalTracked++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTracked;
        emit tracked(0.0);
        return;
    }

    const int N = samples.size();
    double sampleRate = 44100.0;

    /* 计算搜索范围 */
    int minLag = static_cast<int>(sampleRate / m_maxFreq);
    int maxLag = static_cast<int>(sampleRate / m_minFreq);
    minLag = qMax(2, minLag);
    maxLag = qMin(N / 2, maxLag);

    /* 计算自相关函数 */
    QVector<double> autocorr(maxLag + 1, 0.0);
    for (int tau = 0; tau <= maxLag; ++tau) {
        double sum = 0.0;
        for (int n = 0; n < N - tau; ++n) {
            sum += samples[n] * samples[n + tau];
        }
        autocorr[tau] = sum;
    }

    /* 归一化自相关(排除tau=0) */
    double r0 = autocorr[0];
    if (r0 < 1e-15) {
        m_timeSum += timer.elapsed();
        m_stats.totalTracked++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTracked;
        emit tracked(0.0);
        return;
    }

    for (int tau = 0; tau <= maxLag; ++tau) {
        autocorr[tau] /= r0;
    }

    /* 在搜索范围内找最大峰值 */
    double bestCorr = -1.0;
    int bestLag = minLag;

    for (int tau = minLag; tau <= maxLag; ++tau) {
        if (autocorr[tau] > bestCorr) {
            bestCorr = autocorr[tau];
            bestLag = tau;
        }
    }

    /* 抛物线插值精确定位 */
    double refinedLag = static_cast<double>(bestLag);
    if (bestLag > 0 && bestLag < maxLag) {
        double y0 = autocorr[bestLag - 1];
        double y1 = autocorr[bestLag];
        double y2 = autocorr[bestLag + 1];
        double denom = 2.0 * (2.0 * y1 - y0 - y2);
        if (std::abs(denom) > 1e-15) {
            refinedLag += (y0 - y2) / denom;
        }
    }

    /* 计算基频 */
    double fundamentalFreq = 0.0;
    if (bestCorr > 0.3 && refinedLag > 0) {
        fundamentalFreq = sampleRate / refinedLag;
        fundamentalFreq = qBound(m_minFreq, fundamentalFreq, m_maxFreq);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalTracked++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTracked;
    emit tracked(fundamentalFreq);
}

/**
 * @brief 重置统计数据
 */
void PitchTrack5::resetStatistics()
{
    m_stats.totalTracked = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
