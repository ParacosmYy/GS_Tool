/**
 * @file DynamicCompressor2.cpp
 * @brief 动态范围压缩器实现
 *
 * 实现完整的动态范围压缩器：包络检测器（平滑跟踪信号电平）、
 * 增益计算器（基于阈值/比率/拐点的压缩曲线）和增益平滑器
 * （attack/release时间常数控制）。使用QElapsedTimer计时。
 */

#include "utils/dsp49/DynamicCompressor2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @class DynamicCompressor2
 * @brief 动态范围压缩器，支持阈值、比率、attack/release和软拐点参数
 *
 * 信号处理流程：输入 -> 电平检测(峰值) -> 增益计算(软/硬拐点)
 * -> 增益平滑(attack/release) -> 输出。包络跟踪使用一阶IIR平滑滤波器。
 */

/**
 * @brief 构造函数，初始化默认压缩参数
 * @param parent 父QObject指针
 */
DynamicCompressor2::DynamicCompressor2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void DynamicCompressor2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置压缩阈值（dB）
 * @param db 阈值电平，低于此电平的信号不受压缩
 */
void DynamicCompressor2::setThreshold(double db)
{
    m_threshold = db;
}

/**
 * @brief 设置压缩比率
 * @param r 压缩比率（如4.0表示4:1），1.0表示无压缩
 */
void DynamicCompressor2::setRatio(double r)
{
    m_ratio = qMax(1.0, r);
}

/**
 * @brief 设置attack时间（ms）
 * @param ms attack时间常数，控制增益下降的速度
 */
void DynamicCompressor2::setAttack(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置release时间（ms）
 * @param ms release时间常数，控制增益恢复的速度
 */
void DynamicCompressor2::setRelease(double ms)
{
    m_release = qMax(0.01, ms);
}

/**
 * @brief 设置软拐点宽度（dB）
 * @param db 拐点宽度，0.0为硬拐点，大于0为软拐点过渡区域
 */
void DynamicCompressor2::setKnee(double db)
{
    m_knee = qMax(0.0, db);
}

/**
 * @brief 处理音频数据
 *
 * 对输入信号逐样本执行动态范围压缩：
 * 1. 计算输入电平（dB）
 * 2. 使用包络检测器平滑电平跟踪
 * 3. 根据压缩曲线计算增益
 * 4. 应用attack/release平滑增益变化
 *
 * @param input 输入音频样本（-1.0 ~ 1.0浮点）
 * @return 压缩后的音频样本
 */
QVector<double> DynamicCompressor2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    QVector<double> output(n, 0.0);

    if (n == 0) {
        m_stats.totalProcessCalls++;
        return output;
    }

    /* 计算attack/release系数（一阶IIR平滑） */
    double attackCoeff = qExp(-1.0 / (m_attack * 0.001 * m_sampleRate));
    double releaseCoeff = qExp(-1.0 / (m_release * 0.001 * m_sampleRate));

    double totalReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(input[i]);

        /* 转换为dB，避免log(0) */
        double inputDb = -120.0;
        if (absVal > 1e-10) {
            inputDb = 20.0 * qLn(absVal) / qLn(10.0);
        }

        /* 包络检测器：平滑跟踪信号电平 */
        if (inputDb > m_envelope) {
            m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * inputDb;
        } else {
            m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * inputDb;
        }

        /* 根据压缩曲线计算增益 */
        double gainDb = computeGain(m_envelope);
        double gainLin = qPow(10.0, gainDb / 20.0);

        output[i] = input[i] * gainLin;
        totalReduction += qAbs(gainDb);
    }

    /* 记录最终增益衰减量 */
    m_gainReduction = (n > 0) ? totalReduction / n : 0.0;

    m_stats.totalProcessCalls++;
    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessCalls > 0)
        ? m_timeSum / m_stats.totalProcessCalls : 0.0;

    emit processingCompleted(n, m_gainReduction);
    return output;
}

/**
 * @brief 重置统计数据
 */
void DynamicCompressor2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 0.0;
    m_gainReduction = 0.0;
}

/**
 * @brief 根据压缩曲线计算增益（dB）
 *
 * 软拐点模式：在阈值两侧各knee/2的过渡区域内
 * 使用二次插值实现平滑过渡。
 * 硬拐点模式（knee=0）：超过阈值的部分按ratio压缩。
 *
 * @param inputDb 输入信号电平（dB）
 * @return 增益调整量（dB），负值表示衰减
 */
double DynamicCompressor2::computeGain(double inputDb) const
{
    double halfKnee = m_knee / 2.0;
    double thresholdMinusKnee = m_threshold - halfKnee;
    double thresholdPlusKnee = m_threshold + halfKnee;

    if (m_knee <= 0.0) {
        /* 硬拐点：低于阈值无压缩，高于阈值按ratio压缩 */
        if (inputDb <= m_threshold) {
            return 0.0;
        }
        return m_threshold + (inputDb - m_threshold) / m_ratio - inputDb;
    }

    /* 软拐点：三段处理 */
    if (inputDb < thresholdMinusKnee) {
        /* 低于拐点起始：无压缩 */
        return 0.0;
    } else if (inputDb > thresholdPlusKnee) {
        /* 高于拐点结束：完全压缩 */
        return m_threshold + (inputDb - m_threshold) / m_ratio - inputDb;
    } else {
        /* 拐点过渡区：二次插值平滑过渡 */
        double x = inputDb - thresholdMinusKnee;
        double slope = 1.0 / m_ratio - 1.0;
        double gain = x * x * slope / (2.0 * m_knee);
        return gain;
    }
}
