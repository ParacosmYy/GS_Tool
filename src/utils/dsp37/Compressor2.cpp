/**
 * @file Compressor2.cpp
 * @brief 动态压缩器实现 - 软拐点、包络跟随器与增益计算
 *
 * 对输入信号进行动态范围压缩:
 * 1. 通过包络跟随器检测信号电平
 * 2. 使用软拐点曲线计算增益衰减量
 * 3. 将增益应用到信号上，保持平滑的时间响应
 */

#include "utils/dsp37/Compressor2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 *
 * 默认参数: 阈值-20dB, 比率4:1, 拐点宽度6dB,
 * 启动时间10ms, 释放时间100ms
 */
Compressor2::Compressor2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置压缩阈值
 * @param db 阈值(dB)，超过此电平开始压缩
 */
void Compressor2::setThreshold(double db)
{
    m_threshold = db;
}

/**
 * @brief 设置压缩比率
 * @param r 压缩比率(1=无压缩, inf=限幅器)
 */
void Compressor2::setRatio(double r)
{
    m_ratio = qMax(1.0, r);
}

/**
 * @brief 设置软拐点宽度
 * @param db 拐点宽度(dB)，0=硬拐点
 */
void Compressor2::setKnee(double db)
{
    m_knee = qMax(0.0, db);
}

/**
 * @brief 设置启动时间
 * @param ms 启动时间(毫秒)，控制增益减小的速度
 */
void Compressor2::setAttack(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间(毫秒)，控制增益恢复的速度
 */
void Compressor2::setRelease(double ms)
{
    m_release = qMax(0.01, ms);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void Compressor2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 计算压缩增益(dB)
 *
 * 软拐点增益计算公式:
 * - 输入电平在阈值以下: 增益 = 0dB
 * - 输入在拐点范围内: 使用二次插值平滑过渡
 * - 输入超过拐点范围: 增益 = (threshold - input) * (1 - 1/ratio)
 *
 * @param inputDb 输入信号电平(dB)
 * @return 增益衰减量(dB)，负值表示衰减
 */
double Compressor2::computeGain(double inputDb) const
{
    double halfKnee = m_knee / 2.0;
    double kneeLo = m_threshold - halfKnee;
    double kneeHi = m_threshold + halfKnee;

    if (inputDb <= kneeLo) {
        /* 拐点以下: 无压缩 */
        return 0.0;
    } else if (inputDb >= kneeHi) {
        /* 拐点以上: 完全压缩 */
        return m_threshold + (inputDb - m_threshold) / m_ratio - inputDb;
    } else {
        /* 拐点范围内: 二次插值 */
        double x = inputDb - kneeLo;
        double w = m_knee;
        double tQ = (2.0 * x - w) / (2.0 * w);
        return tQ * tQ * (m_knee * (1.0 / m_ratio - 1.0)) / 2.0;
    }
}

/**
 * @brief 处理单个采样点
 *
 * 处理流程:
 * 1. 计算输入电平(dB)
 * 2. 包络跟随器平滑检测电平
 * 3. 计算压缩增益
 * 4. 应用增益到信号
 *
 * @param sample 输入采样值(线性幅度)
 * @return 压缩后的采样值
 */
double Compressor2::processOne(double sample)
{
    /* 计算输入电平(dB) */
    double absSample = qAbs(sample);
    double inputDb = (absSample > 1e-10) ? 20.0 * qLn(absSample) / qLn(10.0) : -120.0;

    /* 包络跟随器(平滑检测电平) */
    double coeff = 0.0;
    if (inputDb > m_envelope) {
        /* 启动阶段 */
        coeff = 1.0 - qExp(-1.0 / (m_attack * m_sampleRate * 0.001));
    } else {
        /* 释放阶段 */
        coeff = 1.0 - qExp(-1.0 / (m_release * m_sampleRate * 0.001));
    }
    m_envelope += coeff * (inputDb - m_envelope);

    /* 计算压缩增益 */
    double gainDb = computeGain(m_envelope);
    m_gainReduction = gainDb;

    /* 转换为线性增益 */
    double gainLin = qPow(10.0, gainDb / 20.0);

    return sample * gainLin;
}

/**
 * @brief 批量处理采样序列
 * @param input 输入采样序列
 * @return 压缩后的采样序列
 */
QVector<double> Compressor2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(input.size());

    double peakReduction = 0.0;

    for (double sample : input) {
        double out = processOne(sample);
        output.append(out);
        peakReduction = qMin(peakReduction, m_gainReduction);
    }

    m_stats.totalSamplesProcessed += input.size();
    m_stats.peakReductionDb = qMin(m_stats.peakReductionDb, peakReduction);
    m_timeSum += timer.elapsed();
    if (m_stats.totalSamplesProcessed > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamplesProcessed;

    emit processingComplete(input.size());
    return output;
}

/**
 * @brief 获取当前增益衰减量
 * @return 增益衰减(dB)，负值表示衰减
 */
double Compressor2::gainReduction() const
{
    return m_gainReduction;
}

/**
 * @brief 重置内部状态(包络等)
 */
void Compressor2::reset()
{
    m_envelope = 0.0;
    m_gainReduction = 0.0;
}

/**
 * @brief 重置所有统计数据
 */
void Compressor2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
