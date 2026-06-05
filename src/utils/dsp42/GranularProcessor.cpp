/**
 * @file GranularProcessor.cpp
 * @brief 颗粒处理器实现 — 颗粒合成+散射+时间拉伸
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp42/GranularProcessor.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
GranularProcessor::GranularProcessor(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("GranularProcessor"));
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率（Hz）
 */
void GranularProcessor::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(8000.0, sampleRate);
}

/**
 * @brief 设置颗粒密度
 *
 * 密度控制每毫秒生成的颗粒数量。较高的密度
 * 产生更丰富的纹理，但增加计算量。
 *
 * @param grainsPerMs 每毫秒颗粒数
 */
void GranularProcessor::setGrainDensity(double grainsPerMs)
{
    m_density = qMax(0.1, grainsPerMs);
}

/**
 * @brief 设置颗粒持续时间范围
 *
 * 所有颗粒的持续时间在该范围内均匀随机分布。
 * 较短的颗粒产生更晶莹的效果，较长的颗粒更平滑。
 *
 * @param minMs 最小持续时间（毫秒）
 * @param maxMs 最大持续时间（毫秒）
 */
void GranularProcessor::setGrainDuration(double minMs, double maxMs)
{
    m_minDuration = qMax(1.0, minMs);
    m_maxDuration = qMax(m_minDuration, maxMs);
}

/**
 * @brief 处理输入信号，生成颗粒合成输出
 *
 * 从输入缓冲区中按密度生成颗粒，每个颗粒具有
 * 随机的起始位置、持续时间、音高偏移和声像位置。
 * 使用汉宁窗形状的包络避免颗粒间的咔嗒噪声。
 *
 * @param input 输入信号缓冲区
 * @param outputLength 输出信号长度（采样点数）
 * @return 颗粒合成输出信号
 */
QVector<double> GranularProcessor::process(const QVector<double>& input, int outputLength)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || outputLength <= 0) return {};

    int inputLen = input.size();
    QVector<double> output(outputLength, 0.0);
    QVector<double> grainCount(outputLength, 0.0);

    m_activeGrains.clear();
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> durDist(m_minDuration, m_maxDuration);
    std::uniform_real_distribution<double> posDist(0.0, 1.0);
    std::uniform_real_distribution<double> panDist(-1.0, 1.0);
    std::uniform_real_distribution<double> pitchDist(0.5, 2.0);
    std::uniform_real_distribution<double> shapeDist(0.2, 0.8);

    int samplesPerMs = qMax(1, (int)(m_sampleRate / 1000.0));
    int grainInterval = qMax(1, (int)(samplesPerMs / m_density));

    m_position = 0;
    int totalGrains = 0;

    for (int outPos = 0; outPos < outputLength; ) {
        /* 生成新颗粒 */
        if (outPos >= m_nextGrain) {
            GrainConfig grain;
            grain.position = posDist(rng) * inputLen;
            grain.duration = durDist(rng);
            grain.pitchShift = pitchDist(rng);
            grain.pan = panDist(rng);
            grain.gain = 0.5 + 0.5 * (rng() % 100) / 100.0;
            grain.shape = shapeDist(rng);

            m_activeGrains.append(grain);
            totalGrains++;

            int grainSamples = (int)(grain.duration * m_sampleRate / 1000.0);
            grainSamples = qMax(1, grainSamples);

            /* 渲染颗粒到输出 */
            for (int s = 0; s < grainSamples && (outPos + s) < outputLength; ++s) {
                /* 包络 */
                double env = grainEnvelope(s, grainSamples, grain.shape);

                /* 从输入读取样本（带音高偏移） */
                double readPos = grain.position + s * grain.pitchShift;
                int idx0 = (int)readPos % inputLen;
                int idx1 = (idx0 + 1) % inputLen;
                double frac = readPos - (int)readPos;

                /* 线性插值 */
                double sample = input[idx0] * (1.0 - frac) + input[idx1] * frac;

                output[outPos + s] += sample * env * grain.gain;
                grainCount[outPos + s] += env;
            }

            m_nextGrain = outPos + grainInterval;
        }

        outPos = m_nextGrain;
    }

    /* 归一化重叠区域 */
    for (int i = 0; i < outputLength; ++i) {
        if (grainCount[i] > 1.0) {
            output[i] /= grainCount[i];
        }
    }

    /* 平滑限幅 */
    for (int i = 0; i < outputLength; ++i) {
        output[i] = qBound(-1.0, output[i], 1.0);
    }

    m_position = outputLength;
    m_stats.totalProcessCalls++;
    m_stats.totalGrainsGenerated += totalGrains;
    m_stats.activeGrains = m_activeGrains.size();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    emit processingCompleted(totalGrains, outputLength);
    return output;
}

/**
 * @brief 获取当前活跃颗粒列表
 * @return 活跃颗粒的配置信息
 */
QVector<GranularProcessor::GrainConfig> GranularProcessor::activeGrainList() const
{
    return m_activeGrains;
}

/**
 * @brief 计算颗粒包络
 *
 * 使用可变形的余弦窗函数。shape参数控制包络形状：
 * - 接近0：高斯形（柔和攻击和释放）
 * - 接近0.5：汉宁窗（正弦包络）
 * - 接近1：梯形（快速攻击/释放，长持续段）
 *
 * @param sample 当前采样点索引
 * @param duration 颗粒总长度（采样点）
 * @param shape 包络形状参数（0-1）
 * @return 包络值（0-1）
 */
double GranularProcessor::grainEnvelope(int sample, int duration, double shape) const
{
    if (duration <= 0) return 0.0;
    double t = (double)sample / duration;

    /* 可变形余弦窗 */
    double attack = qMax(0.01, shape * 0.3);
    double release = qMax(0.01, (1.0 - shape) * 0.3);

    double env = 1.0;
    if (t < attack) {
        env = 0.5 * (1.0 - qCos(M_PI * t / attack));
    } else if (t > 1.0 - release) {
        env = 0.5 * (1.0 - qCos(M_PI * (1.0 - t) / release));
    }

    return qBound(0.0, env, 1.0);
}

/**
 * @brief 重置所有统计数据
 */
void GranularProcessor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
