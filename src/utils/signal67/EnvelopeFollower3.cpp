/**
 * @file EnvelopeFollower3.cpp
 * @brief 包络跟踪器实现（第3版）
 *
 * 实现音频信号的包络（幅度包络）提取。支持峰值检测和
 * RMS两种模式。使用一阶低通滤波器实现平滑的攻击/释放
 * 时间控制，常用于动态处理器（压缩器/扩展器）。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal67/EnvelopeFollower3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化包络跟踪器
 * @param parent 父QObject对象指针
 */
EnvelopeFollower3::EnvelopeFollower3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void EnvelopeFollower3::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间（毫秒），控制包络上升速度
 */
void EnvelopeFollower3::setAttackTime(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间（毫秒），控制包络下降速度
 */
void EnvelopeFollower3::setReleaseTime(double ms)
{
    m_release = qMax(0.01, ms);
}

/**
 * @brief 设置检测模式
 * @param mode 检测模式："peak"峰值检测, "rms"均方根检测
 */
void EnvelopeFollower3::setMode(const QString& mode)
{
    if (mode == "peak" || mode == "rms") {
        m_mode = mode;
    }
}

/**
 * @brief 处理输入音频信号，提取包络
 *
 * 对每个样本计算瞬时幅度，然后根据攻击/释放系数
 * 平滑输出。攻击时间控制包络跟随信号上升的速度，
 * 释放时间控制包络衰减的速度。
 *
 * @param input 输入音频采样序列
 * @return 包络幅度序列
 */
QVector<double> EnvelopeFollower3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) {
        emit processingCompleted(0, 0.0);
        return output;
    }

    int n = input.size();
    output.resize(n);

    /* 计算攻击和释放系数 */
    double attackCoeff = qExp(-1.0 / (m_attack * 0.001 * m_sampleRate));
    double releaseCoeff = qExp(-1.0 / (m_release * 0.001 * m_sampleRate));

    if (m_mode == "peak") {
        /* 峰值检测模式 */
        for (int i = 0; i < n; ++i) {
            double absVal = qAbs(input[i]);

            if (absVal > m_level) {
                /* 信号上升，使用攻击系数 */
                m_level = attackCoeff * m_level + (1.0 - attackCoeff) * absVal;
            } else {
                /* 信号下降，使用释放系数 */
                m_level = releaseCoeff * m_level + (1.0 - releaseCoeff) * absVal;
            }

            output[i] = m_level;
            if (m_level > m_peak) m_peak = m_level;
        }
    } else {
        /* RMS检测模式：使用滑动窗口的均方根 */
        int winSize = static_cast<int>(m_attack * 0.001 * m_sampleRate);
        winSize = qMax(1, qMin(winSize, n));
        double sumSq = 0.0;

        for (int i = 0; i < n; ++i) {
            sumSq += input[i] * input[i];

            if (i >= winSize) {
                sumSq -= input[i - winSize] * input[i - winSize];
            }

            int count = qMin(i + 1, winSize);
            double rms = qSqrt(sumSq / count);

            /* 平滑RMS值 */
            if (rms > m_level) {
                m_level = attackCoeff * m_level + (1.0 - attackCoeff) * rms;
            } else {
                m_level = releaseCoeff * m_level + (1.0 - releaseCoeff) * rms;
            }

            output[i] = m_level;
            if (m_level > m_peak) m_peak = m_level;
        }
    }

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, m_peak);
    return output;
}

/**
 * @brief 获取当前统计信息
 * @return 处理统计结构
 */
EnvelopeFollower3::Stats EnvelopeFollower3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void EnvelopeFollower3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 重置包络状态
 *
 * 将当前电平和峰值归零，但不影响统计数据。
 * 适用于处理新的音频段时清除前一帧的状态。
 */
void EnvelopeFollower3::resetState()
{
    m_level = 0.0;
    m_peak = 0.0;
}

/**
 * @brief 计算分贝值
 *
 * 将当前电平转换为分贝标度。
 * 0dB对应幅度1.0，负值表示衰减。
 *
 * @return 当前电平的分贝值
 */
double EnvelopeFollower3::currentLevelDb() const
{
    if (m_level <= 0.0) return -120.0;
    return 20.0 * qLn(m_level) / qLn(10.0);
}

/**
 * @brief 获取峰值分贝值
 * @return 峰值电平的分贝值
 */
double EnvelopeFollower3::peakLevelDb() const
{
    if (m_peak <= 0.0) return -120.0;
    return 20.0 * qLn(m_peak) / qLn(10.0);
}

/**
 * @brief 计算峰值因数（ Crest Factor）
 *
 * 峰值因数 = 峰值 / RMS电平。高峰值因数表示信号有尖锐的瞬态。
 * 正弦波约1.41，方波为1.0，音乐通常2~6。
 *
 * @return 峰值因数
 */
double EnvelopeFollower3::crestFactor() const
{
    if (m_level < 1e-10) return 0.0;
    return m_peak / m_level;
}
