#include "SpectralFlux3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱通量计算器
 * @param parent 父QObject对象指针
 */
SpectralFlux3::SpectralFlux3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算连续频谱帧之间的频谱通量序列
 *
 * 频谱通量定义为相邻帧频谱幅度差之和（仅保留正值）：
 * SF(t) = sum(max(0, |X(t,f)| - |X(t-1,f)|))
 *
 * 正值约束确保只捕获频谱能量的增加（onset特征），
 * 忽略能量衰减（offset特征）。
 *
 * @param spectra 频谱帧序列，每帧为幅度谱向量
 * @return 频谱通量值序列，长度为spectra.size()-1
 */
QVector<double> SpectralFlux3::compute(const QVector<QVector<double>>& spectra)
{
    QElapsedTimer timer;
    timer.start();

    const int numFrames = spectra.size();
    if (numFrames < 2) return {};

    const int numBins = spectra[0].size();
    QVector<double> flux(numFrames - 1);

    /// 逐帧计算频谱通量
    for (int t = 1; t < numFrames; ++t) {
        double frameFlux = 0.0;
        const int bins = qMin(spectra[t].size(), spectra[t - 1].size());

        for (int f = 0; f < bins; ++f) {
            double diff = spectra[t][f] - spectra[t - 1][f];
            frameFlux += qMax(0.0, diff);  ///< 仅保留正值（能量增长）
        }

        flux[t - 1] = frameFlux;
    }

    /// 保存最后一帧用于后续增量计算
    if (numFrames > 0) {
        m_prevSpectrum = spectra.last();
    }

    /// 更新统计信息
    m_stats.totalFramesProcessed += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesProcessed);

    return flux;
}

/**
 * @brief 基于频谱通量检测onset点
 *
 * 使用自适应阈值方法检测onset：
 * 1. 对通量序列计算局部均值作为基线
 * 2. 阈值 = 基线 + threshold * 局部标准差
 * 3. 检测超过阈值的峰值点
 * 4. 施加最小onset间隔约束避免重复检测
 *
 * @param flux 频谱通量序列
 * @param threshold 检测阈值倍数(典型值1.0~2.0)
 * @return onset点的帧索引列表
 */
QVector<int> SpectralFlux3::detectOnsets(const QVector<double>& flux, double threshold)
{
    QElapsedTimer timer;
    timer.start();

    const int n = flux.size();
    if (n == 0) return {};

    /// 计算全局统计量
    double mean = 0.0;
    for (double v : flux) mean += v;
    mean /= n;

    double variance = 0.0;
    for (double v : flux) {
        double diff = v - mean;
        variance += diff * diff;
    }
    double stddev = std::sqrt(variance / n);

    /// 局部自适应阈值（滑动窗口）
    const int windowSize = qMax(4, n / 8);
    QVector<double> localThreshold(n);
    for (int i = 0; i < n; ++i) {
        int start = qMax(0, i - windowSize / 2);
        int end = qMin(n, i + windowSize / 2);
        double localMean = 0.0;
        for (int j = start; j < end; ++j) localMean += flux[j];
        localMean /= (end - start);
        localThreshold[i] = localMean + threshold * stddev;
    }

    /// 峰值检测
    QVector<int> onsets;
    int minInterval = qMax(1, n / 32);  ///< 最小onset间隔
    int lastOnset = -minInterval;

    for (int i = 1; i < n - 1; ++i) {
        if (flux[i] > localThreshold[i] &&
            flux[i] > flux[i - 1] && flux[i] >= flux[i + 1] &&
            (i - lastOnset) >= minInterval) {
            onsets.append(i);
            emit onsetDetected(i, flux[i]);
            lastOnset = i;
        }
    }

    /// 更新统计信息
    m_stats.totalOnsetsDetected += onsets.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesProcessed);

    return onsets;
}

/**
 * @brief 获取当前统计数据
 * @return 包含处理帧数、onset检测数和平均耗时的Stats结构
 */
SpectralFlux3::Stats SpectralFlux3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值，清除历史频谱
 */
void SpectralFlux3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_prevSpectrum.clear();
}
