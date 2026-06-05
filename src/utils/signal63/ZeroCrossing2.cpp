/**
 * @file ZeroCrossing2.cpp
 * @brief 过零检测器实现 (过零率 + 频率估计)
 *
 * 实现基于过零检测的频率分析方法:
 * - 过零率: 信号在单位时间内穿越零电平的次数
 * - 频率估计: 根据相邻正-负过零点的时间间隔估计基频
 * - 最小距离约束: 过滤噪声引起的虚假过零
 * 常用于音高检测、语音端点检测和简单频率测量。
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
 */
ZeroCrossing2::ZeroCrossing2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率 (Hz)，默认 44100.0
 */
void ZeroCrossing2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置过零阈值
 * @param thresh 过零检测阈值 (默认 0.0)
 *
 * 信号必须穿越 -thresh ~ +thresh 区域才算有效过零
 * 设置非零阈值可以抑制噪声
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
 */
void ZeroCrossing2::setMinDistance(int samples)
{
    m_minDist = qMax(1, samples);
}

/**
 * @brief 检测信号中的过零点并估计频率
 *
 * 检测流程:
 * 1. 遍历信号，找到符号变化点 (正变负或负变正)
 * 2. 使用线性插值精确定位过零位置
 * 3. 应用阈值过滤和最小距离约束
 * 4. 根据过零率估计频率: freq = crossingRate * sampleRate / 2
 * 5. 根据相邻过零间隔估计基频
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

    /* 步骤1: 检测所有符号变化点 */
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

    /* 步骤2: 应用最小距离约束 */
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
        /* 方法1: 基于过零间隔的中位数 */
        QVector<double> periods;
        for (int i = 1; i < m_count; ++i) {
            double period = crossings[i] - crossings[i - 1];
            if (period > 0) {
                periods.append(period);
            }
        }

        if (!periods.isEmpty()) {
            /* 取中位数周期，更鲁棒 */
            std::sort(periods.begin(), periods.end());
            double medianPeriod = periods[periods.size() / 2];
            m_freq = m_sampleRate / medianPeriod;
        }
    } else if (m_count == 1 && N > 1) {
        /* 单个过零点: 使用过零率估计 */
        m_freq = m_count * m_sampleRate / (2.0 * N);
    }

    /* 限制频率范围 */
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
 */
void ZeroCrossing2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_freq = 0.0;
    m_count = 0;
}
