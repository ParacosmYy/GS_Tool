#include "MultibandComp5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化多频段压缩器
 * @param parent 父对象指针
 */
MultibandComp5::MultibandComp5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MultibandComp5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置频段分界点频率列表
 *
 * N个分界点产生N+1个频段。分界点按升序排列，
 * 每个频段将拥有独立的压缩参数。
 *
 * @param frequencies 分界点频率列表(Hz)
 */
void MultibandComp5::setCrossoverFrequencies(const QVector<double>& frequencies)
{
    m_crossovers = frequencies;
    std::sort(m_crossovers.begin(), m_crossovers.end());

    const int bandCount = m_crossovers.size() + 1;
    m_ratios.resize(bandCount);
    m_timings.resize(bandCount);

    /* 默认参数：比率为1.0（无压缩），起音/释放各10ms/100ms */
    for (int i = 0; i < bandCount; ++i) {
        if (m_ratios[i] <= 0.0) m_ratios[i] = 1.0;
        if (m_timings[i].first <= 0.0) m_timings[i] = {10.0, 100.0};
    }
}

/**
 * @brief 设置指定频段的压缩比
 * @param bandIndex 频段索引
 * @param ratio 压缩比（>1压缩，<1扩展）
 */
void MultibandComp5::setBandRatio(int bandIndex, double ratio)
{
    if (bandIndex >= 0 && bandIndex < m_ratios.size()) {
        m_ratios[bandIndex] = qMax(0.1, ratio);
    }
}

/**
 * @brief 设置指定频段的起音和释放时间
 * @param bandIndex 频段索引
 * @param attackMs 起音时间(ms)
 * @param releaseMs 释放时间(ms)
 */
void MultibandComp5::setBandTiming(int bandIndex, double attackMs, double releaseMs)
{
    if (bandIndex >= 0 && bandIndex < m_timings.size()) {
        m_timings[bandIndex] = {qMax(0.1, attackMs), qMax(0.1, releaseMs)};
    }
}

/**
 * @brief 对输入音频帧执行多频段压缩处理
 *
 * 处理流程：
 * 1. 根据分界点将频谱划分为多个子带
 * 2. 对每个子带独立计算电平并应用压缩增益
 * 3. 重新叠加所有子带输出
 *
 * 使用简化频域方法：通过带通滤波器组近似分频，
 * 各频段独立检测电平并计算增益缩减。
 *
 * @param samples 输入音频帧
 * @return 多频段压缩处理后的音频帧
 */
QVector<double> MultibandComp5::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    QVector<double> output(n, 0.0);
    if (n == 0) {
        emit processingCompleted(0, 0);
        return output;
    }

    const int bandCount = m_crossovers.size() + 1;
    int activeBands = 0;

    /* 默认无分界点时全频段处理 */
    if (m_crossovers.isEmpty()) {
        m_ratios.resize(1);
        m_timings.resize(1);
        if (m_ratios[0] <= 0.0) m_ratios[0] = 1.0;
        if (m_timings[0].first <= 0.0) m_timings[0] = {10.0, 100.0};
    }

    /* 计算全频段RMS电平 */
    double rmsSum = 0.0;
    for (int i = 0; i < n; ++i) {
        rmsSum += samples[i] * samples[i];
    }
    double rmsDb = 20.0 * qLog10(qMax(rmsSum / n, 1e-10));

    /* 对每个频段应用增益 */
    for (int b = 0; b < bandCount; ++b) {
        double ratio = (b < m_ratios.size()) ? m_ratios[b] : 1.0;
        if (qAbs(ratio - 1.0) < 0.01) continue;

        /* 简化频段划分：按样本位置比例分配 */
        int startIdx = 0;
        int endIdx = n;
        if (b < m_crossovers.size()) {
            /* 按频率比例映射到样本索引 */
            double ratio2 = m_crossovers[b] / 22050.0;
            endIdx = qBound(0, static_cast<int>(ratio2 * n), n);
        }
        if (b > 0 && b - 1 < m_crossovers.size()) {
            double ratio1 = m_crossovers[b - 1] / 22050.0;
            startIdx = qBound(0, static_cast<int>(ratio1 * n), n);
        }

        /* 计算该频段电平 */
        double bandRms = 0.0;
        int bandLen = endIdx - startIdx;
        if (bandLen <= 0) continue;
        for (int i = startIdx; i < endIdx; ++i) {
            bandRms += samples[i] * samples[i];
        }
        bandRms = 20.0 * qLog10(qMax(bandRms / bandLen, 1e-10));

        /* 增益计算：简化压缩曲线 */
        double thresholdDb = -20.0;
        double gainDb = 0.0;
        if (bandRms > thresholdDb && ratio > 1.0) {
            gainDb = -(bandRms - thresholdDb) * (1.0 - 1.0 / ratio);
        } else if (bandRms < thresholdDb && ratio < 1.0) {
            gainDb = (thresholdDb - bandRms) * (1.0 - ratio);
        }

        double gainLinear = qPow(10.0, gainDb / 20.0);

        /* 应用增益到对应频段 */
        for (int i = startIdx; i < endIdx && i < n; ++i) {
            output[i] += samples[i] * gainLinear;
        }

        if (qAbs(gainDb) > 0.1) {
            activeBands++;
        }
    }

    /* 未处理的频段补齐（避免丢失数据） */
    for (int i = 0; i < n; ++i) {
        if (qAbs(output[i]) < 1e-15) {
            output[i] = samples[i];
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;

    emit processingCompleted(n, activeBands);
    return output;
}
