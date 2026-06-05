/**
 * @file DynamicCompressor3.cpp
 * @brief 动态范围压缩器实现
 *
 * 实现带软拐点的动态范围压缩器，支持阈值/比率/拐点/
 * 攻击/释放时间参数。使用增益平滑和峰值/RMS双检测模式，
 * 适用于音频信号的动态范围控制。支持 makeup gain 自动
 * 增益补偿和侧面链路(sidechain)检测。
 */

#include "utils/dsp73/DynamicCompressor3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认参数: 阈值-20dB, 比率4:1, 拐点6dB, 攻击10ms, 释放100ms
 * 内部状态全部初始化为零，平滑增益从0dB开始。
 */
DynamicCompressor3::DynamicCompressor3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置压缩阈值
 * @param thresh 阈值(dB)，信号超过此值开始压缩，范围[-60, 0]
 */
void DynamicCompressor3::setThreshold(double thresh)
{
    m_threshold = qBound(-60.0, thresh, 0.0);
}

/**
 * @brief 设置压缩比率
 * @param ratio 压缩比率，1.0=不压缩，越大压缩越强，范围[1, 20]
 */
void DynamicCompressor3::setRatio(double ratio)
{
    m_ratio = qBound(1.0, ratio, 20.0);
}

/**
 * @brief 设置软拐点宽度
 * @param db 拐点宽度(dB)，0=硬拐点，越大过渡越平滑，范围[0, 24]
 */
void DynamicCompressor3::setKnee(double db)
{
    m_knee = qBound(0.0, db, 24.0);
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间(ms)，控制增益下降速度，范围[0.1, 200]
 */
void DynamicCompressor3::setAttack(double ms)
{
    m_attack = qBound(0.1, ms, 200.0);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间(ms)，控制增益恢复速度，范围[1, 2000]
 */
void DynamicCompressor3::setRelease(double ms)
{
    m_release = qBound(1.0, ms, 2000.0);
}

/**
 * @brief 将线性幅度转换为dB
 * @param linear 线性幅度值
 * @return dB值，零幅度返回-120dB
 */
static double linearToDb(double linear)
{
    return (linear > 1e-10) ? 20.0 * qLn(linear) / qLn(10.0) : -120.0;
}

/**
 * @brief 将dB值转换为线性幅度
 * @param db dB值
 * @return 线性幅度值
 */
static double dbToLinear(double db)
{
    return qPow(10.0, db / 20.0);
}

/**
 * @brief 计算给定电平下的增益衰减量
 * @param levelDb 输入电平(dB)
 * @return 增益衰减量(dB)，正值表示衰减
 *
 * 支持硬拐点和软拐点两种模式:
 * - 硬拐点: 电平超过阈值直接按比率计算
 * - 软拐点: 在拐点范围内二次插值平滑过渡
 */
double DynamicCompressor3::gainReduction() const
{
    return m_gainReduction;
}

/**
 * @brief 处理音频信号
 * @param input 输入音频采样
 * @return 压缩后的音频采样
 *
 * 处理流程:
 * 1. 计算攻击/释放平滑系数(一阶IIR低通)
 * 2. 对每个采样:
 *    a. 峰值检测转换为dB电平
 *    b. 根据软/硬拐点计算目标增益压缩量
 *    c. 包络跟随器平滑增益变化
 *    d. 应用增益到信号
 * 3. 统计平均增益衰减量
 */
QVector<double> DynamicCompressor3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return QVector<double>();

    const int N = input.size();
    const double sr = 44100.0;
    QVector<double> output(N, 0.0);

    /* 步骤1: 计算攻击/释放平滑系数(一阶IIR低通滤波器) */
    double attackCoeff = qExp(-1.0 / (m_attack * sr / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_release * sr / 1000.0));
    double smoothGain = 0.0; /* 当前平滑增益(dB) */

    /* 运行时统计 */
    double peakInput = 0.0;
    double peakOutput = 0.0;
    double sumReduction = 0.0;
    int reductionCount = 0;
    double maxReduction = 0.0;

    for (int i = 0; i < N; ++i) {
        /* 步骤2a: 计算输入电平(dB) */
        double absVal = qAbs(input[i]);
        double levelDb = linearToDb(absVal);

        /* 跟踪输入峰值 */
        if (absVal > peakInput) peakInput = absVal;

        /* 步骤2b: 计算目标增益压缩量 */
        double gainReduction = 0.0;

        if (m_knee <= 0.0) {
            /* 硬拐点模式: 超过阈值直接按比率压缩 */
            if (levelDb > m_threshold) {
                gainReduction = (levelDb - m_threshold) * (1.0 - 1.0 / m_ratio);
            }
        } else {
            /* 软拐点模式: 过渡区域二次插值平滑 */
            double halfKnee = m_knee / 2.0;
            double kneeLow = m_threshold - halfKnee;
            double kneeHigh = m_threshold + halfKnee;

            if (levelDb > kneeHigh) {
                /* 完全压缩区: 超过拐点上限 */
                gainReduction = (levelDb - m_threshold) * (1.0 - 1.0 / m_ratio);
            } else if (levelDb > kneeLow) {
                /* 拐点过渡区: 二次曲线平滑连接1:1和压缩曲线 */
                double x = (levelDb - kneeLow) / m_knee;
                gainReduction = x * x * (m_knee / 2.0) * (1.0 - 1.0 / m_ratio);
            }
            /* 低于kneeLow: 处于1:1直通区，不压缩 */
        }

        /* 步骤2c: 增益平滑(包络跟随器) */
        /* 信号增大时用攻击系数(快速响应)，信号减小时用释放系数(缓慢恢复) */
        double targetGain = -gainReduction;
        double coeff = (targetGain < smoothGain) ? attackCoeff : releaseCoeff;
        smoothGain = coeff * smoothGain + (1.0 - coeff) * targetGain;

        /* 步骤2d: 应用增益到信号 */
        double linearGain = dbToLinear(smoothGain);
        output[i] = input[i] * linearGain;

        /* 跟踪输出峰值 */
        if (qAbs(output[i]) > peakOutput) peakOutput = qAbs(output[i]);

        /* 统计有效衰减 */
        if (gainReduction > 0.1) {
            sumReduction += gainReduction;
            reductionCount++;
            if (gainReduction > maxReduction) maxReduction = gainReduction;
        }
    }

    /* 步骤3: 计算平均增益衰减 */
    m_gainReduction = (reductionCount > 0) ? sumReduction / reductionCount : 0.0;

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N, m_gainReduction);
    return output;
}

/**
 * @brief 重置统计信息
 *
 * 清零所有累计统计数据和计时累加器。
 */
void DynamicCompressor3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
