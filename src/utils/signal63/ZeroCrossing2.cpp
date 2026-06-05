/**
 * @file ZeroCrossing2.cpp
 * @brief 过零检测器实现 (过零率 + 频率估计)
 *
 * 实现基于过零检测的频率分析方法:
 * - 过零率(ZCR): 信号在单位时间内穿越零电平的次数
 *   ZCR = (过零次数) / (信号时长)
 * - 频率估计: 根据相邻正到负过零点的时间间隔估计基频
 *   freq = sampleRate / medianPeriod
 * - 最小距离约束: 过滤噪声引起的虚假过零
 *
 * 检测策略:
 * - 只统计正到负方向的过零 (更稳定的基频估计)
 * - 使用线性插值精确定位过零位置 (亚采样精度)
 * - 使用阈值带 [-thresh, +thresh] 抑制低幅噪声
 * - 使用最小间距过滤密集虚假过零
 * - 使用周期中位数(非均值)提高鲁棒性
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal63/ZeroCrossing2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化过零检测器
 * @param parent 父QObject指针
 *
 * 默认参数: sampleRate=44100, threshold=0.0, minDist=10
 */
ZeroCrossing2::ZeroCrossing2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率 (Hz)，默认 44100.0
 *
 * 采样率用于将过零间隔转换为频率
 */
void ZeroCrossing2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置过零阈值
 * @param thresh 过零检测阈值 (默认 0.0)
 *
 * 信号必须穿越 [-thresh, +thresh] 区域才算有效过零:
 * - thresh = 0: 严格过零检测
 * - thresh > 0: 滞后阈值，抑制小幅振荡噪声
 */
void ZeroCrossing2::setThreshold(double thresh)
{
    m_threshold = qBound(0.0, thresh, 1.0);
}

/**
 * @brief 设置最小过零间距
 * @param samples 两个过零点之间的最小采样间隔 (默认 10)
 *
 * 用于过滤噪声引起的密集虚假过零
 * 对于基频检测，建议设置为 sampleRate / maxExpectedFreq
 */
void ZeroCrossing2::setMinDistance(int samples)
{
    m_minDist = qMax(1, samples);
}

/**
 * @brief 检测信号中的过零点并估计频率
 *
 * 处理流程:
 * 1. 遍历信号，检测正到负方向的符号变化点
 * 2. 使用线性插值精确定位过零位置 (亚采样精度)
 *    crossPos = i + |signal[i]| / |signal[i+1] - signal[i]|
 * 3. 应用阈值带过滤: 信号必须穿越 [-thresh, +thresh]
 * 4. 应用最小距离约束: 过滤密集虚假过零
 * 5. 根据过零间隔的中位数估计基频
 *
 * 频率估计方法:
 * - 多个过零点: freq = sampleRate / medianPeriod (中位数，鲁棒)
 * - 单个过零点: freq = crossingRate * sampleRate / 2
 * - 无过零点: freq = 0
 *
 * @param signal 输入信号
 * @return 过零点的精确位置列表 (以采样为单位的小数位置)
 */
QVector<double> ZeroCrossing2::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> crossings;
    m_count = 0;
    m_freq = 0.0;

    if (signal.size() < 2) {
        m_stats.totalDetections++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalDetections > 0)
            ? m_timeSum / m_stats.totalDetections : 0.0;
        emit frequencyEstimated(0.0);
        return crossings;
    }

    const int N = signal.size();

    /* 步骤1: 检测所有正到负方向的过零点 */
    QVector<double> rawCrossings;

    for (int i = 0; i < N - 1; ++i) {
        double curr = signal[i];
        double next = signal[i + 1];

        /* 检查是否穿过零电平 (或阈值带) */
        bool currAbove = (curr > m_threshold);
        bool nextBelow = (next < -m_threshold);
        bool currBelow = (curr < -m_threshold);
        bool nextAbove = (next > m_threshold);

        if ((currAbove && nextBelow) || (currBelow && nextAbove)) {
            /* 线性插值精确定位过零位置 */
            double crossPos;
            if (qAbs(next - curr) > 1e-15) {
                crossPos = i + qAbs(curr) / qAbs(next - curr);
            } else {
                crossPos = i + 0.5;
            }

            /* 只记录正到负的过零 (用于基频估计) */
            if (currAbove && nextBelow) {
                rawCrossings.append(crossPos);
            }
        }
    }

    /* 步骤2: 应用最小距离约束过滤虚假过零 */
    double lastCrossing = -m_minDist * 2.0;
    for (double pos : rawCrossings) {
        if (pos - lastCrossing >= m_minDist) {
            crossings.append(pos);
            lastCrossing = pos;
        }
    }

    m_count = crossings.size();

    /* 步骤3: 估计频率 */
    if (m_count >= 2) {
        /* 方法: 基于过零间隔的中位数 */
        QVector<double> periods;
        for (int i = 1; i < m_count; ++i) {
            double period = crossings[i] - crossings[i - 1];
            if (period > 0) {
                periods.append(period);
            }
        }

        if (!periods.isEmpty()) {
            /* 取中位数周期，比均值更鲁棒 */
            std::sort(periods.begin(), periods.end());
            double medianPeriod = periods[periods.size() / 2];
            m_freq = m_sampleRate / medianPeriod;
        }
    } else if (m_count == 1 && N > 1) {
        /* 单个过零点: 使用过零率估计 (粗略) */
        m_freq = m_count * m_sampleRate / (2.0 * N);
    }

    /* 限制频率范围在 [0, Nyquist] */
    m_freq = qBound(0.0, m_freq, m_sampleRate / 2.0);

    /* 更新统计 */
    m_stats.totalDetections++;
    m_stats.totalSamples += N;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit frequencyEstimated(m_freq);
    return crossings;
}

/**
 * @brief 重置所有统计数据
 *
 * 重置统计计数器、计时器累积、频率和过零计数
 */
void ZeroCrossing2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_freq = 0.0;
    m_count = 0;
}
