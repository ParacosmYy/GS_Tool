/**
 * @file Compressor4.cpp
 * @brief 动态范围压缩器实现 — 软拐点音频压缩
 *
 * 实现具有可配置阈值、比率、软拐点、攻击/释放时间和补偿增益
 * 的动态范围压缩器。使用包络跟随器和平滑增益控制实现自然压缩效果。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/dsp63/Compressor4.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
Compressor4::Compressor4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置压缩阈值
 * @param thresh 阈值(dB)，信号超过此值开始压缩
 */
void Compressor4::setThreshold(double thresh)
{
    m_threshold = thresh;
}

/**
 * @brief 设置压缩比
 * @param ratio 压缩比(如4.0表示4:1)，必须 >= 1.0
 */
void Compressor4::setRatio(double ratio)
{
    m_ratio = qMax(1.0, ratio);
}

/**
 * @brief 设置软拐点宽度
 * @param db 拐点宽度(dB)，0.0为硬拐点
 */
void Compressor4::setKnee(double db)
{
    m_knee = qMax(0.0, db);
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间(毫秒)，控制增益减小的速度
 */
void Compressor4::setAttack(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间(毫秒)，控制增益恢复的速度
 */
void Compressor4::setRelease(double ms)
{
    m_release = qMax(1.0, ms);
}

/**
 * @brief 设置补偿增益
 * @param db 补偿增益(dB)，用于补偿压缩造成的音量损失
 */
void Compressor4::setMakeupGain(double db)
{
    m_makeup = db;
}

/**
 * @brief 处理音频信号
 *
 * 处理流程:
 * 1. 检测信号电平(dB)
 * 2. 根据阈值/比率/拐点计算增益衰减
 * 3. 使用攻击/释放时间平滑增益变化
 * 4. 应用补偿增益
 *
 * @param input 输入音频采样序列
 * @return 压缩处理后的音频采样序列
 */
QVector<double> Compressor4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) {
        return {};
    }

    /* 采样率假设为44100Hz */
    const double sampleRate = 44100.0;

    /* 计算攻击/释放系数 */
    const double attackCoeff = qExp(-1.0 / (m_attack * 0.001 * sampleRate));
    const double releaseCoeff = qExp(-1.0 / (m_release * 0.001 * sampleRate));

    /* 补偿增益(线性) */
    const double makeupLin = qPow(10.0, m_makeup * 0.05);

    QVector<double> output(n);
    double envelopeDb = -120.0;  /* 初始包络为极低值 */
    double gainLin = 1.0;        /* 当前增益(线性) */
    m_gainReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double sample = input[i];

        /* 步骤1: 将采样值转换为dB电平 */
        double absSample = qAbs(sample);
        double inputDb = 20.0 * qLog10(qMax(1e-10, absSample));

        /* 步骤2: 包络跟随(峰值检测) */
        if (inputDb > envelopeDb) {
            envelopeDb = attackCoeff * envelopeDb + (1.0 - attackCoeff) * inputDb;
        } else {
            envelopeDb = releaseCoeff * envelopeDb + (1.0 - releaseCoeff) * inputDb;
        }

        /* 步骤3: 计算增益衰减(使用软拐点) */
        double gainReductionDb = softKnee(envelopeDb);
        m_gainReduction = gainReductionDb;

        /* 步骤4: 将dB增益转换为线性增益并平滑 */
        double targetGain = qPow(10.0, gainReductionDb * 0.05);
        double coeff = (gainReductionDb < 0.0) ? attackCoeff : releaseCoeff;
        gainLin = coeff * gainLin + (1.0 - coeff) * targetGain;

        /* 步骤5: 应用增益和补偿 */
        output[i] = sample * gainLin * makeupLin;
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
void Compressor4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算软拐点增益衰减
 *
 * 软拐点公式: 在阈值附近smooth过渡，
 * kneeWidth为零时退化为硬拐点。
 *
 * @param inputDb 输入信号电平(dB)
 * @return 增益衰减量(dB)，负值表示衰减
 */
double Compressor4::softKnee(double inputDb) const
{
    if (m_knee <= 0.0) {
        /* 硬拐点 */
        if (inputDb > m_threshold) {
            return -(inputDb - m_threshold) * (1.0 - 1.0 / m_ratio);
        }
        return 0.0;
    }

    /* 软拐点: 在 [threshold - knee/2, threshold + knee/2] 区间平滑过渡 */
    double halfKnee = m_knee * 0.5;
    double kneeLo = m_threshold - halfKnee;
    double kneeHi = m_threshold + halfKnee;

    if (inputDb <= kneeLo) {
        /* 低于拐点区域: 无压缩 */
        return 0.0;
    }

    if (inputDb >= kneeHi) {
        /* 高于拐点区域: 完全压缩 */
        return -(inputDb - m_threshold) * (1.0 - 1.0 / m_ratio);
    }

    /* 拐点区域内: 二次插值平滑过渡 */
    double x = inputDb - kneeLo;
    double compressionFactor = (1.0 - 1.0 / m_ratio) * x * x / (2.0 * m_knee);
    return -(compressionFactor);
}
