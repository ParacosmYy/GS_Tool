#include "NoiseEstimate4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file NoiseEstimate4.cpp
 * @brief 噪声水平估计器实现
 *
 * 支持三种噪声估计策略:
 * - minima: 滑动窗口最小值跟踪
 * - percentile: 排序后的百分位数
 * - mmse: 最小均方误差估计
 */

/**
 * @brief 构造函数，初始化默认估计参数
 * @param parent 父QObject对象指针
 */
NoiseEstimate4::NoiseEstimate4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置分析窗口大小
 * @param size 滑动窗口的采样点数
 */
void NoiseEstimate4::setWindowSize(int size)
{
    m_windowSize = qMax(2, size);
}

/**
 * @brief 设置估计方法
 * @param method 估计方法名称: minima/percentile/mmse
 */
void NoiseEstimate4::setMethod(const QString& method)
{
    if (method == "minima" || method == "percentile" || method == "mmse") {
        m_method = method;
    }
}

/**
 * @brief 估计噪声水平
 *
 * 根据选择的方法估计信号中的噪声水平:
 * - minima: 在滑动窗口内取最小值的加权平均
 * - percentile: 对信号能量排序取第10百分位
 * - mmse: 基于统计模型的MMSE估计
 *
 * @param samples 输入信号采样数据
 * @return 估计的噪声水平(dB)
 */
double NoiseEstimate4::estimate(const QVector<double>& samples)
{
    if (samples.isEmpty()) return -100.0;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    double noiseLevelDb = -100.0;

    if (m_method == "minima") {
        // 滑动窗口最小值跟踪法
        double minEnergySum = 0.0;
        int windowCount = 0;

        for (int i = 0; i <= N - m_windowSize; i += m_windowSize / 2) {
            double minVal = 1e18;
            for (int j = i; j < i + m_windowSize && j < N; ++j) {
                const double energy = samples[j] * samples[j];
                minVal = qMin(minVal, energy);
            }
            minEnergySum += minVal;
            windowCount++;
        }

        if (windowCount > 0) {
            const double avgMinEnergy = minEnergySum / windowCount;
            noiseLevelDb = 10.0 * std::log10(qMax(avgMinEnergy, 1e-20));
        }
    } else if (m_method == "percentile") {
        // 百分位数法: 取能量的第10百分位
        QVector<double> energies;
        energies.reserve(N);
        for (int i = 0; i < N; ++i) {
            energies.append(samples[i] * samples[i]);
        }
        std::sort(energies.begin(), energies.end());

        const int idx = qBound(0, static_cast<int>(0.1 * N), N - 1);
        noiseLevelDb = 10.0 * std::log10(qMax(energies[idx], 1e-20));
    } else {
        // MMSE估计: 基于信号统计特性
        double meanEnergy = 0.0;
        double varEnergy = 0.0;

        for (int i = 0; i < N; ++i) {
            meanEnergy += samples[i] * samples[i];
        }
        meanEnergy /= N;

        for (int i = 0; i < N; ++i) {
            const double diff = samples[i] * samples[i] - meanEnergy;
            varEnergy += diff * diff;
        }
        varEnergy /= N;

        // MMSE估计: 噪声能量 = mean - sqrt(var) (简化模型)
        const double noiseEst = qMax(meanEnergy - std::sqrt(varEnergy), 1e-20);
        noiseLevelDb = 10.0 * std::log10(noiseEst);
    }

    // 更新统计信息
    m_stats.totalFrames++;
    const double prevAvg = m_stats.avgNoiseLevel;
    m_stats.avgNoiseLevel = prevAvg + (noiseLevelDb - prevAvg) / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit estimated(noiseLevelDb);
    return noiseLevelDb;
}

/**
 * @brief 重置所有统计信息
 */
void NoiseEstimate4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
