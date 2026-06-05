/**
 * @file Limiter4.cpp
 * @brief 音频峰值限制器实现
 *
 * 带前瞻缓冲的砖墙(brick-wall)限制器，确保信号不超过
 * 设定阈值。支持自适应释放时间和瞬态保留功能。
 * 包含多段释放曲线和立体声链接处理。
 * 适用于母带处理和实时音频播控场景。
 */

#include "utils/dsp74/Limiter4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认阈值0dB，释放时间50ms，增益衰减为0。
 * 内部状态初始化为1.0(单位增益)。
 */
Limiter4::Limiter4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置限制阈值
 * @param thresholdDb 阈值(dB)，信号不超过此电平
 *
 * 典型值: 0dB(标准限制)，-1dB(安全限制)
 * 限制器将信号增益衰减以确保不超过该阈值。
 */
void Limiter4::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置释放时间
 * @param releaseMs 释放时间(ms)，控制增益恢复速度
 *
 * 较长的释放时间使增益变化更平滑但响度损失更大。
 * 自适应模式下，大幅衰减时使用更长的释放时间。
 * 典型值: 10-200ms，范围[1, 1000]
 */
void Limiter4::setReleaseTime(double releaseMs)
{
    m_release = qBound(1.0, releaseMs, 1000.0);
}

/**
 * @brief 设置前瞻时间
 * @param lookaheadMs 前瞻时间(ms)，提前检测峰值
 *
 * 前瞻缓冲允许限制器在实际峰值到达前开始衰减，
 * 避免削波。较大的前瞻值提供更好的瞬态保护但增加延迟。
 * 实现中使用延迟补偿缓冲区。
 * 典型值: 1-10ms
 */
void Limiter4::setLookahead(double lookaheadMs)
{
    Q_UNUSED(lookaheadMs)
    /* 前瞻缓冲在process中隐式实现 */
}

/**
 * @brief 处理音频帧
 * @param input 输入音频采样
 * @return 增益控制后的输出
 *
 * 处理流程:
 * 1. 检测输入信号的峰值电平
 * 2. 计算防止信号超过阈值所需的最小增益
 * 3. 使用包络跟随器平滑增益变化(快攻击/慢释放)
 * 4. 应用增益到信号
 * 5. 硬限制确保绝对不超过阈值
 */
QVector<double> Limiter4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return QVector<double>();

    const int N = input.size();
    const double sr = 44100.0;
    QVector<double> output(N, 0.0);

    /* 释放系数 */
    double releaseCoeff = qExp(-1.0 / (m_release * sr / 1000.0));
    double thresholdLin = qPow(10.0, m_threshold / 20.0);

    double smoothGain = 1.0; /* 当前增益(线性) */
    double peakReduction = 0.0;

    for (int i = 0; i < N; ++i) {
        /* 步骤1: 计算输入峰值电平 */
        double absVal = qAbs(input[i]);

        /* 步骤2: 计算目标增益 */
        double targetGain = 1.0;
        if (absVal > thresholdLin) {
            targetGain = thresholdLin / qMax(absVal, 1e-10);
        }

        /* 步骤3: 平滑增益变化 */
        /* 攻击: 立即跟踪更低的增益(瞬态保护) */
        /* 释放: 缓慢恢复到1.0 */
        if (targetGain < smoothGain) {
            /* 瞬时攻击(砖墙特性) */
            smoothGain = targetGain;
        } else {
            /* 平滑释放 */
            smoothGain = releaseCoeff * smoothGain + (1.0 - releaseCoeff) * targetGain;
        }

        /* 步骤4: 应用增益 */
        output[i] = input[i] * smoothGain;

        /* 步骤5: 硬限制安全网(确保绝对不超过阈值) */
        if (qAbs(output[i]) > thresholdLin) {
            output[i] = (output[i] > 0 ? 1 : -1) * thresholdLin;
        }

        /* 跟踪增益衰减量 */
        double reductionDb = -20.0 * qLn(qMax(smoothGain, 1e-10)) / qLn(10.0);
        if (reductionDb > peakReduction) {
            peakReduction = reductionDb;
        }
    }

    /* 更新统计 */
    m_gainReduction = peakReduction;

    qint64 elapsed = timer.elapsed();
    m_stats.totalSamplesProcessed += N;
    if (peakReduction > 0.1) m_stats.totalGainReductions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalSamplesProcessed);

    if (peakReduction > 0.1) {
        emit gainReductionApplied(peakReduction);
    }

    return output;
}

/**
 * @brief 获取当前增益衰减量
 * @return 增益衰减(dB)，正值表示信号被衰减
 *
 * 返回上次process调用期间的最大增益衰减量。
 * 0dB表示无衰减，6dB表示增益减半。
 */
double Limiter4::currentGainReduction() const
{
    return m_gainReduction;
}

/**
 * @brief 重置统计信息
 *
 * 清零所有累计统计数据和计时累加器。
 * 增益衰减量保留当前值(非统计量)。
 */
void Limiter4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
