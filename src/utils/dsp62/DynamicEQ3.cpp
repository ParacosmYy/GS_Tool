/**
 * @file DynamicEQ3.cpp
 * @brief 动态均衡器实现 — 频段自适应增益控制
 *
 * 根据指定频段的信号电平动态调整增益，集成压缩/扩展功能，
 * 支持可配置的攻击/释放时间、阈值和比率参数。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/dsp62/DynamicEQ3.h"

#include <QElapsedTimer>
#include <QStringList>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
DynamicEQ3::DynamicEQ3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置均衡器中心频率
 * @param freq 中心频率(Hz)，必须 > 0
 */
void DynamicEQ3::setFrequency(double freq)
{
    m_freq = qMax(20.0, freq);
}

/**
 * @brief 设置频带带宽
 * @param bw 带宽(倍频程)，必须 > 0
 */
void DynamicEQ3::setBandwidth(double bw)
{
    m_bw = qMax(0.01, bw);
}

/**
 * @brief 设置压缩阈值
 * @param thresh 阈值(dB)，典型值 -60 ~ 0
 */
void DynamicEQ3::setThreshold(double thresh)
{
    m_threshold = thresh;
}

/**
 * @brief 设置压缩比
 * @param ratio 压缩比，必须 > 1.0
 */
void DynamicEQ3::setRatio(double ratio)
{
    m_ratio = qMax(1.0, ratio);
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间(毫秒)，必须 > 0
 */
void DynamicEQ3::setAttack(double ms)
{
    m_attack = qMax(0.1, ms);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间(毫秒)，必须 > 0
 */
void DynamicEQ3::setRelease(double ms)
{
    m_release = qMax(1.0, ms);
}

/**
 * @brief 处理音频信号，应用动态均衡
 *
 * 对输入信号进行频带分析，检测频带内能量，
 * 根据阈值和比率动态调整增益。
 *
 * @param input 输入音频采样序列
 * @return 处理后的音频采样序列
 */
QVector<double> DynamicEQ3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) {
        return {};
    }

    /* 假设采样率44100Hz用于频率计算 */
    const double sampleRate = 44100.0;

    /* 攻击/释放系数 (平滑包络跟随器) */
    const double attackCoeff = qExp(-1.0 / (m_attack * sampleRate * 0.001));
    const double releaseCoeff = qExp(-1.0 / (m_release * sampleRate * 0.001));

    /* 频带参数: 计算频带边界对应的近似系数 */
    const double loFreq = m_freq / qPow(2.0, m_bw * 0.5);
    const double hiFreq = m_freq * qPow(2.0, m_bw * 0.5);
    /* 简化二阶带通滤波器系数 */
    const double omega = 2.0 * M_PI * m_freq / sampleRate;
    const double Q = m_freq / (hiFreq - loFreq);
    const double alpha = qSin(omega) / (2.0 * Q);
    const double b0 = alpha;
    const double b1 = 0.0;
    const double b2 = -alpha;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * qCos(omega);
    const double a2 = 1.0 - alpha;

    QVector<double> output(n);
    double env = 0.0;
    double x1 = 0.0, x2 = 0.0;   /* 带通滤波器状态 */
    double y1 = 0.0, y2 = 0.0;
    double gainState = 1.0;        /* 平滑增益状态 */
    m_gainReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double x = input[i];

        /* 步骤1: 带通滤波，提取目标频段 */
        double bp = (b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        x2 = x1; x1 = x;
        y2 = y1; y1 = bp;

        /* 步骤2: 包络检测 (峰值检测器) */
        double absBp = qAbs(bp);
        if (absBp > env) {
            env = attackCoeff * env + (1.0 - attackCoeff) * absBp;
        } else {
            env = releaseCoeff * env + (1.0 - releaseCoeff) * absBp;
        }

        /* 步骤3: 转换为dB并计算增益 */
        double envDb = 20.0 * qLog10(qMax(1e-10, env));
        double gainDb = 0.0;
        if (envDb > m_threshold) {
            gainDb = -(envDb - m_threshold) * (1.0 - 1.0 / m_ratio);
        }

        m_gainReduction = gainDb;

        /* 步骤4: 平滑增益变化 */
        double targetGain = qPow(10.0, gainDb * 0.05); /* 0.05 = dB/20 for amplitude */
        double gainCoeff = (gainDb < 0.0) ? attackCoeff : releaseCoeff;
        gainState = gainCoeff * gainState + (1.0 - gainCoeff) * targetGain;

        /* 步骤5: 应用增益到原始信号 */
        output[i] = x * gainState;
    }

    /* 更新统计信息 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, m_gainReduction);
    return output;
}

/**
 * @brief 重置所有统计数据
 */
void DynamicEQ3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 获取当前参数配置摘要
 *
 * 返回所有关键参数的当前值，用于调试和状态监控。
 *
 * @return 参数描述字符串列表
 */
QStringList DynamicEQ3::parameterSummary() const
{
    QStringList summary;
    summary << QString("中心频率: %1 Hz").arg(m_freq, 0, 'f', 1);
    summary << QString("带宽: %1 倍频程").arg(m_bw, 0, 'f', 2);
    summary << QString("阈值: %1 dB").arg(m_threshold, 0, 'f', 1);
    summary << QString("比率: %1:1").arg(m_ratio, 0, 'f', 1);
    summary << QString("攻击: %1 ms").arg(m_attack, 0, 'f', 1);
    summary << QString("释放: %1 ms").arg(m_release, 0, 'f', 1);
    return summary;
}

/**
 * @brief 验证参数是否在有效范围内
 *
 * 检查所有参数是否满足约束条件:
 * - 频率 >= 20 Hz
 * - 带宽 > 0
 * - 比率 >= 1.0
 * - 攻击 > 0 ms
 * - 释放 > 0 ms
 *
 * @return true如果所有参数有效
 */
bool DynamicEQ3::validateParameters() const
{
    if (m_freq < 20.0) return false;
    if (m_bw <= 0.0) return false;
    if (m_ratio < 1.0) return false;
    if (m_attack <= 0.0) return false;
    if (m_release <= 0.0) return false;
    return true;
}

/**
 * @brief 预计算频带内的平均能量估计
 *
 * 对输入信号进行简单的频带能量估计，用于预测压缩量。
 * 使用RMS(均方根)计算频带内的近似能量。
 *
 * @param input 输入信号
 * @return 估计的频带能量(dB)
 */
double DynamicEQ3::estimateBandEnergy(const QVector<double>& input) const
{
    if (input.isEmpty()) return -120.0;

    /* 简单RMS估计 */
    double sumSq = 0.0;
    for (double sample : input) {
        sumSq += sample * sample;
    }
    double rms = qSqrt(sumSq / static_cast<double>(input.size()));
    return 20.0 * qLog10(qMax(1e-10, rms));
}
