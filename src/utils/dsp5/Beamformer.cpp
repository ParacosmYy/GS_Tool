/**
 * @file Beamformer.cpp
 * @brief 延迟-求和波束成形器实现 — 传感器阵列信号增强
 */

#include "utils/dsp5/Beamformer.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param numSensors 传感器数 @param spacing 间距 @param sampleRate 采样率 @param parent 父对象 */
Beamformer::Beamformer(int numSensors, double spacing,
                       double sampleRate, QObject* parent)
    : QObject(parent)
    , m_numSensors(qMax(2, numSensors))
    , m_spacing(qMax(0.001, spacing))
    , m_sampleRate(qMax(1.0, sampleRate))
    , m_waveSpeed(343.0)
    , m_steeringAngle(0.0)
    , m_timeSum(0.0)
{
    /* 默认均匀权重 */
    m_weights.resize(m_numSensors);
    m_weights.fill(1.0 / static_cast<double>(m_numSensors));
}

/** @brief 设置波束指向角度 @param angle 角度(度) */
void Beamformer::setSteeringAngle(double angle)
{
    /* 限制角度范围 [-90, 90] 度 */
    angle = qBound(-90.0, angle, 90.0);
    m_steeringAngle = qDegreesToRadians(angle);
}

/** @brief 处理多通道信号帧 @param inputSignals inputSignals[channel][sample] @return 波束成形输出 */
QVector<double> Beamformer::process(const QVector<QVector<double>>& inputSignals)
{
    m_timer.start();

    QVector<double> output;

    if (inputSignals.size() != m_numSensors || inputSignals.isEmpty()) {
        return output;
    }

    /* 确定输出长度(取最短通道) */
    int minLen = inputSignals[0].size();
    for (int ch = 1; ch < m_numSensors; ++ch) {
        minLen = qMin(minLen, inputSignals[ch].size());
    }

    if (minLen <= 0) return output;

    output.resize(minLen);

    /* 计算各通道时延(秒) */
    QVector<double> delays = computeDelays();

    /* 转换为采样点数延迟 */
    QVector<double> delaySamples(m_numSensors);
    double maxDelay = 0.0;
    for (int ch = 0; ch < m_numSensors; ++ch) {
        delaySamples[ch] = delays[ch] * m_sampleRate;
        if (qAbs(delaySamples[ch]) > maxDelay) {
            maxDelay = qAbs(delaySamples[ch]);
        }
    }

    /* 延迟-求和 */
    int halfWindow = static_cast<int>(qCeil(maxDelay)) + 1;
    int startIdx = halfWindow;

    for (int n = startIdx; n < minLen; ++n) {
        double sum = 0.0;
        for (int ch = 0; ch < m_numSensors; ++ch) {
            /* 对齐时间: 输出采样n对应通道ch的采样 n + delaySamples[ch] */
            double targetIdx = static_cast<double>(n) - delaySamples[ch];
            if (targetIdx >= 0.0 && targetIdx < static_cast<double>(minLen)) {
                double interpolated = sincInterpolate(inputSignals[ch], targetIdx);
                sum += m_weights[ch] * interpolated;
            }
        }
        output[n] = sum;
    }

    /* 前部补零 */
    for (int n = 0; n < startIdx && n < minLen; ++n) {
        output[n] = 0.0;
    }

    ++m_stats.totalProcessed;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalProcessed);

    emit processingComplete(minLen);
    return output;
}

/** @brief 设置各通道权重 @param weights 权重向量 */
void Beamformer::setWeights(const QVector<double>& weights)
{
    if (weights.size() != m_numSensors) return;

    /* 归一化权重使总和为1 */
    double sum = 0.0;
    for (double w : weights) {
        sum += qAbs(w);
    }
    if (qFuzzyIsNull(sum)) return;

    m_weights.resize(m_numSensors);
    for (int i = 0; i < m_numSensors; ++i) {
        m_weights[i] = weights[i] / sum;
    }
}

/** @brief 设置波传播速度 @param speed 波速(m/s) */
void Beamformer::setWaveSpeed(double speed)
{
    m_waveSpeed = qMax(1.0, speed);
}

/** @brief 重置统计 */
void Beamformer::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算各传感器时延 @return 时延数组(秒) */
QVector<double> Beamformer::computeDelays() const
{
    QVector<double> delays(m_numSensors);

    /* ULA模型: 传感器m的位置 d_m = m * spacing */
    /* 时延 tau_m = -d_m * sin(theta) / c */
    /* 参考传感器(索引0)延迟为0 */
    for (int m = 0; m < m_numSensors; ++m) {
        double position = static_cast<double>(m) * m_spacing;
        delays[m] = -position * qSin(m_steeringAngle) / m_waveSpeed;
    }

    /* 减去最大延迟使所有延迟为正(因果化) */
    double maxDelay = 0.0;
    for (double d : delays) {
        if (d < maxDelay) maxDelay = d;
    }
    for (int m = 0; m < m_numSensors; ++m) {
        delays[m] -= maxDelay;
    }

    return delays;
}

/** @brief sinc插值 @param signal 信号 @param delaySamples 延迟采样数 @return 插值结果 */
double Beamformer::sincInterpolate(const QVector<double>& signal,
                                   double delaySamples) const
{
    /* 窗口化sinc插值(Lanczos, a=4) */
    int a = 4;
    int idx = static_cast<int>(qFloor(delaySamples));
    double frac = delaySamples - static_cast<double>(idx);

    double result = 0.0;
    for (int k = -a + 1; k <= a; ++k) {
        int si = idx + k;
        if (si < 0 || si >= signal.size()) continue;

        double x = frac - static_cast<double>(k);
        double s;
        if (qFuzzyIsNull(x)) {
            s = 1.0;
        } else {
            double pix = M_PI * x;
            s = qSin(pix) / pix; /* sinc(x) */
            /* Lanczos窗: sinc(x/a) */
            double pixA = M_PI * x / static_cast<double>(a);
            s *= qSin(pixA) / pixA;
        }
        result += signal[si] * s;
    }

    return result;
}
