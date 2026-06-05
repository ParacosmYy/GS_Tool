#include "TransientDetect3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化瞬态信号检测器
 * @param parent 父对象指针
 */
TransientDetect3::TransientDetect3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置检测灵敏度
 * @param sensitivity 灵敏度系数(>1更灵敏，<1更保守)
 */
void TransientDetect3::setSensitivity(double sensitivity)
{
    m_sensitivity = qMax(0.1, sensitivity);
}

/**
 * @brief 对输入信号执行瞬态检测
 *
 * 基于短时能量差分和频谱通量两个指标检测瞬态事件：
 * 1. 计算短时能量包络
 * 2. 检测能量上升沿(一阶差分超过动态阈值)
 * 3. 发出检测到的瞬态位置信号
 *
 * @param samples 输入音频采样
 */
void TransientDetect3::detect(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalDetected++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetected;
        return;
    }

    const int N = samples.size();
    int frameSize = qMax(4, N / 32);
    int hopSize = frameSize / 2;

    /* 计算短时能量包络 */
    QVector<double> energy;
    for (int start = 0; start + frameSize <= N; start += hopSize) {
        double e = 0.0;
        for (int i = start; i < start + frameSize; ++i) {
            e += samples[i] * samples[i];
        }
        energy.append(e / frameSize);
    }

    if (energy.size() < 2) {
        m_timeSum += timer.elapsed();
        m_stats.totalDetected++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetected;
        return;
    }

    /* 计算能量差分(一阶导数) */
    QVector<double> diff(energy.size() - 1);
    for (int i = 0; i < diff.size(); ++i) {
        diff[i] = energy[i + 1] - energy[i];
    }

    /* 计算动态阈值：基于差分的中位数和MAD */
    auto sortedDiff = diff;
    std::sort(sortedDiff.begin(), sortedDiff.end());
    double median = sortedDiff[sortedDiff.size() / 2];

    double madSum = 0.0;
    for (double d : diff) madSum += std::abs(d - median);
    double mad = madSum / diff.size();

    double threshold = median + m_sensitivity * qMax(mad, 1e-10);

    /* 检测超过阈值的正差分(上升沿) */
    for (int i = 0; i < diff.size(); ++i) {
        if (diff[i] > threshold) {
            int position = i * hopSize + frameSize / 2;
            emit detected(position);
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalDetected++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetected;
}

/**
 * @brief 重置统计数据
 */
void TransientDetect3::resetStatistics()
{
    m_stats.totalDetected = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
