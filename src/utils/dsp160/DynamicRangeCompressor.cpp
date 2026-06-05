/**
 * @file DynamicRangeCompressor.cpp
 * @brief 动态范围压缩器实现
 *
 * 实现增益计算(软拐点)、attack/release包络跟随、
 * RMS/峰值检测和增益平滑。
 */

#include "utils/dsp160/DynamicRangeCompressor.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
DynamicRangeCompressor::DynamicRangeCompressor(QObject* parent)
    : QObject(parent)
{
}

void DynamicRangeCompressor::setAttack(double ms)
{
    m_attackMs = qBound(0.1, ms, 100.0);
}

void DynamicRangeCompressor::setRelease(double ms)
{
    m_releaseMs = qBound(10.0, ms, 3000.0);
}

void DynamicRangeCompressor::setThreshold(double db)
{
    m_thresholdDb = qBound(-60.0, db, 0.0);
}

void DynamicRangeCompressor::setRatio(double ratio)
{
    m_ratio = qBound(1.0, ratio, 20.0);
}

void DynamicRangeCompressor::setKnee(double db)
{
    m_kneeDb = qBound(0.0, db, 24.0);
}

void DynamicRangeCompressor::setDetectionMode(DetectionMode mode)
{
    m_detectionMode = mode;
}

/**
 * @brief 计算压缩增益(dB)
 *
 * 使用软拐点公式：当输入电平在阈值±knee/2范围内时，
 * 增益曲线平滑过渡，避免硬拐点的突变失真。
 */
double DynamicRangeCompressor::computeGainReduction(double inputLevelDb) const
{
    if (m_kneeDb > 0.0) {
        /* 软拐点 */
        double halfKnee = m_kneeDb / 2.0;
        double lowerBound = m_thresholdDb - halfKnee;
        double upperBound = m_thresholdDb + halfKnee;

        if (inputLevelDb <= lowerBound) {
            return 0.0;  /* 低于拐点区域，无压缩 */
        } else if (inputLevelDb >= upperBound) {
            /* 高于拐点区域，完全压缩 */
            double overDb = inputLevelDb - m_thresholdDb;
            return -overDb * (1.0 - 1.0 / m_ratio);
        } else {
            /* 拐点过渡区域 */
            double x = inputLevelDb - lowerBound;
            double t = m_thresholdDb - lowerBound + x * x / (2.0 * m_kneeDb);
            return -t * (1.0 - 1.0 / m_ratio);
        }
    } else {
        /* 硬拐点 */
        if (inputLevelDb <= m_thresholdDb) {
            return 0.0;
        }
        double overDb = inputLevelDb - m_thresholdDb;
        return -overDb * (1.0 - 1.0 / m_ratio);
    }
}

/**
 * @brief 增益平滑(attack/release包络跟随)
 *
 * 使用一阶IIR低通滤波器：attack时间控制增益下降速度，
 * release时间控制增益恢复速度。attack远小于release以
 * 实现快速响应、慢速恢复的经典压缩特性。
 */
double DynamicRangeCompressor::smoothGain(double targetGainDb, double currentGainDb) const
{
    double coeff;
    if (targetGainDb < currentGainDb) {
        /* Attack: 增益降低(信号超出阈值) */
        coeff = qExp(-1.0 / (m_attackMs * m_sampleRate / 1000.0));
    } else {
        /* Release: 增益恢复 */
        coeff = qExp(-1.0 / (m_releaseMs * m_sampleRate / 1000.0));
    }
    return currentGainDb + (1.0 - coeff) * (targetGainDb - currentGainDb);
}

/**
 * @brief 处理单帧音频数据
 *
 * 逐采样点执行：检测电平 → 计算压缩增益 → 平滑 → 应用增益。
 */
QVector<double> DynamicRangeCompressor::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return QVector<double>();

    QVector<double> output(n);
    double frameReductionSum = 0.0;
    int reductionCount = 0;

    for (int i = 0; i < n; ++i) {
        double sample = input[i];

        /* 检测电平 */
        double levelDb;
        if (m_detectionMode == DetectionMode::RMS) {
            /* RMS平滑检测 */
            double sampleSq = sample * sample;
            double rmsAlpha = qExp(-1.0 / (0.050 * m_sampleRate));  /* 50ms窗口 */
            m_rmsWindow = rmsAlpha * m_rmsWindow + (1.0 - rmsAlpha) * sampleSq;
            double rms = qSqrt(qMax(m_rmsWindow, 1e-20));
            levelDb = 20.0 * qLn(rms) / qLn(10.0);
        } else {
            /* 峰值检测 */
            double peak = qAbs(sample);
            levelDb = 20.0 * qLn(qMax(peak, 1e-20)) / qLn(10.0);
        }

        /* 计算目标增益 */
        double targetGainDb = computeGainReduction(levelDb);

        /* 增益平滑 */
        m_currentGainDb = smoothGain(targetGainDb, m_currentGainDb);

        /* 应用增益 */
        double gainLinear = qPow(10.0, m_currentGainDb / 20.0);
        output[i] = sample * gainLinear;

        /* 统计 */
        if (m_currentGainDb < -0.1) {
            frameReductionSum += m_currentGainDb;
            reductionCount++;
        }
    }

    /* 更新统计 */
    m_stats.totalSamples += n;
    if (reductionCount > 0) {
        m_stats.totalGainReductions += reductionCount;
        m_reductionSum += frameReductionSum;
        m_stats.avgReductionDb = m_reductionSum / m_stats.totalGainReductions;
    }

    m_timeSum += timer.elapsed();
    double totalFrames = m_stats.totalSamples / qMax(n, 1);
    m_stats.avgProcessingTimeMs = (totalFrames > 0) ? m_timeSum / totalFrames : 0.0;

    double avgReduction = (reductionCount > 0) ? frameReductionSum / reductionCount : 0.0;
    emit frameProcessed(n, avgReduction);

    return output;
}

void DynamicRangeCompressor::reset()
{
    m_currentGainDb = 0.0;
    m_rmsWindow = 0.0;
}

void DynamicRangeCompressor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reductionSum = 0.0;
}
