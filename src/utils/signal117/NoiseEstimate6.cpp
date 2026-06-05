#include "NoiseEstimate6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化噪声估计器
 * @param parent 父对象指针
 */
NoiseEstimate6::NoiseEstimate6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void NoiseEstimate6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 重置噪声估计内部状态
 */
void NoiseEstimate6::resetState()
{
    /* 内部状态在updateOnline中按需重新初始化 */
}

/**
 * @brief 使用最小统计量法估计噪声功率谱
 *
 * 对多帧功率谱序列，取每个频率bin的时间方向最小值作为噪声估计。
 * 使用滑动窗口提高跟踪能力，并施加偏差补偿因子。
 *
 * @param powerSpectrum 多帧功率谱序列 (帧数 × 频率bin数)
 * @return 估计的噪声功率谱
 */
QVector<double> NoiseEstimate6::minStatistics(
    const QVector<QVector<double>>& powerSpectrum)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> noiseEstimate;
    const int numFrames = powerSpectrum.size();
    if (numFrames == 0) {
        emit estimationCompleted(0);
        return noiseEstimate;
    }

    const int numBins = powerSpectrum[0].size();
    noiseEstimate.resize(numBins);

    /* 对每个频率bin取时间方向最小值 */
    const int windowLen = qMax(1, numFrames / 2);
    for (int k = 0; k < numBins; ++k) {
        double minVal = 1e18;
        for (int f = 0; f < numFrames; ++f) {
            if (k < powerSpectrum[f].size()) {
                double val = qMax(powerSpectrum[f][k], 1e-20);
                if (val < minVal) minVal = val;
            }
        }
        /* Martin偏差补偿因子 (近似1.5) */
        noiseEstimate[k] = minVal * 1.5;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimationOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimationOps;

    emit estimationCompleted(numFrames);
    return noiseEstimate;
}

/**
 * @brief 使用分位数法估计噪声
 *
 * 对功率谱排序后取指定分位数位置的值作为噪声功率估计。
 * 默认取中位数(0.5)，低分位数(0.1~0.3)通常更接近噪声底。
 *
 * @param powerSpectrum 功率谱序列
 * @param quantile 分位数阈值 [0, 1]
 * @return 噪声功率估计值
 */
double NoiseEstimate6::quantileEstimate(const QVector<double>& powerSpectrum,
                                         double quantile)
{
    QElapsedTimer timer;
    timer.start();

    const int n = powerSpectrum.size();
    if (n == 0) {
        emit estimationCompleted(0);
        return 0.0;
    }

    quantile = qBound(0.0, quantile, 1.0);
    QVector<double> sorted = powerSpectrum;
    std::sort(sorted.begin(), sorted.end());

    int idx = qBound(0, static_cast<int>(quantile * (n - 1)), n - 1);
    double estimate = qMax(sorted[idx], 1e-20);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimationOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimationOps;

    emit estimationCompleted(1);
    return estimate;
}

/**
 * @brief 在线逐帧噪声估计更新
 *
 * 使用一阶递归平滑跟踪噪声功率谱：
 * - 当帧能量低于当前估计时，使用较快的更新速率（噪声段）
 * - 当帧能量高于当前估计时，使用极慢的更新速率（信号段）
 * - VAD辅助判断：高能量帧基本不更新噪声估计
 *
 * @param currentFrame 当前帧功率谱
 * @return 更新后的噪声估计
 */
QVector<double> NoiseEstimate6::updateOnline(const QVector<double>& currentFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = currentFrame.size();
    if (n == 0) {
        emit estimationCompleted(0);
        return m_noiseEstimate;
    }

    const double alphaDecay = 0.95;    /* 信号段慢衰减 */
    const double alphaTrack = 0.85;    /* 噪声段快跟踪 */

    if (m_noiseEstimate.size() != n) {
        m_noiseEstimate = currentFrame;
    }

    /* 计算帧能量 */
    double frameEnergy = 0.0;
    double noiseEnergy = 0.0;
    for (int i = 0; i < n; ++i) {
        frameEnergy += currentFrame[i];
        noiseEnergy += m_noiseEstimate[i];
    }

    /* 简单VAD判断 */
    bool isNoise = (frameEnergy < noiseEnergy * 2.0);

    for (int i = 0; i < n; ++i) {
        double val = qMax(currentFrame[i], 1e-20);
        if (val < m_noiseEstimate[i] || isNoise) {
            m_noiseEstimate[i] = alphaTrack * m_noiseEstimate[i]
                                 + (1.0 - alphaTrack) * val;
        } else {
            m_noiseEstimate[i] = alphaDecay * m_noiseEstimate[i]
                                 + (1.0 - alphaDecay) * val;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEstimationOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEstimationOps;

    emit estimationCompleted(1);
    return m_noiseEstimate;
}
