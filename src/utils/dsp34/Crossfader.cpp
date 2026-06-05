/**
 * @file Crossfader.cpp
 * @brief 交叉淡入淡出实现 — 线性/等功率/S曲线/余弦/淡入淡出
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp34/Crossfader.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
Crossfader::Crossfader(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("Crossfader"));
}

/**
 * @brief 设置交叉淡入淡出曲线类型
 *
 * Linear: 线性插值 gainA = 1-pos, gainB = pos
 * EqualPower: 等功率 gainA = cos(pos*pi/2), gainB = sin(pos*pi/2)
 * SCurve: S曲线平滑过渡
 * Cosine: 余弦平滑
 *
 * @param type 曲线类型枚举
 */
void Crossfader::setCurveType(CurveType type)
{
    m_curve = type;
}

/**
 * @brief 设置交叉淡入淡出位置
 *
 * pos=0: 完全A通道; pos=1: 完全B通道; pos=0.5: 中间混合。
 *
 * @param pos 位置 [0.0, 1.0]
 */
void Crossfader::setPosition(double pos)
{
    m_position = qBound(0.0, pos, 1.0);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void Crossfader::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 计算A通道增益
 *
 * 根据当前曲线类型和位置计算A通道的增益系数。
 *
 * @param pos 混合位置 [0, 1]
 * @return A通道增益
 */
double Crossfader::gainA(double pos) const
{
    pos = qBound(0.0, pos, 1.0);
    switch (m_curve) {
    case Linear:
        return 1.0 - pos;
    case EqualPower:
        return qCos(pos * M_PI / 2.0);
    case SCurve: {
        /* S曲线: 使用smoothstep函数 */
        double t = 1.0 - pos;
        return t * t * (3.0 - 2.0 * t);
    }
    case Cosine:
        return 0.5 * (1.0 + qCos(pos * M_PI));
    }
    return 1.0 - pos;
}

/**
 * @brief 计算B通道增益
 *
 * @param pos 混合位置 [0, 1]
 * @return B通道增益
 */
double Crossfader::gainB(double pos) const
{
    pos = qBound(0.0, pos, 1.0);
    switch (m_curve) {
    case Linear:
        return pos;
    case EqualPower:
        return qSin(pos * M_PI / 2.0);
    case SCurve: {
        double t = pos;
        return t * t * (3.0 - 2.0 * t);
    }
    case Cosine:
        return 0.5 * (1.0 - qCos(pos * M_PI));
    }
    return pos;
}

/**
 * @brief 处理单采样点交叉淡入淡出
 *
 * 输出 = sampleA * gainA(position) + sampleB * gainB(position)
 *
 * @param sampleA A通道采样值
 * @param sampleB B通道采样值
 * @return 混合后的采样值
 */
double Crossfader::processOne(double sampleA, double sampleB)
{
    double ga = gainA(m_position);
    double gb = gainB(m_position);
    return sampleA * ga + sampleB * gb;
}

/**
 * @brief 批量处理交叉淡入淡出
 *
 * 对A/B通道的每个对应采样点执行混合。
 * 位置保持不变，所有采样使用相同的混合比例。
 *
 * @param a A通道采样向量
 * @param b B通道采样向量
 * @return 混合后的采样向量
 */
QVector<double> Crossfader::process(const QVector<double>& a, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int len = qMin(a.size(), b.size());
    QVector<double> output(len);

    for (int i = 0; i < len; ++i) {
        output[i] = processOne(a[i], b[i]);
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalCrossfades++;
    m_stats.totalSamplesProcessed += len;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCrossfades;

    emit crossfadeComplete(len);
    return output;
}

/**
 * @brief 淡出处理
 *
 * 将输入信号从满增益线性/曲线衰减到0。
 * 前 fadeSamples 个采样执行淡出，之后输出为0。
 *
 * @param input 输入采样向量
 * @param fadeSamples 淡出采样数
 * @return 淡出后的向量
 */
QVector<double> Crossfader::processFadeOut(const QVector<double>& input, int fadeSamples)
{
    QElapsedTimer timer;
    timer.start();

    int len = input.size();
    QVector<double> output(len, 0.0);
    fadeSamples = qMax(1, fadeSamples);

    for (int i = 0; i < len; ++i) {
        if (i < fadeSamples) {
            /* 淡出: position从0渐变到1，取gainA的衰减 */
            double pos = static_cast<double>(i) / static_cast<double>(fadeSamples);
            output[i] = input[i] * gainA(pos);
        }
        /* 超过fadeSamples的部分保持0 */
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalCrossfades++;
    m_stats.totalSamplesProcessed += len;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCrossfades;

    return output;
}

/**
 * @brief 淡入处理
 *
 * 将输入信号从0渐增到满增益。
 * 前 fadeSamples 个采样执行淡入，之后直接输出。
 *
 * @param input 输入采样向量
 * @param fadeSamples 淡入采样数
 * @return 淡入后的向量
 */
QVector<double> Crossfader::processFadeIn(const QVector<double>& input, int fadeSamples)
{
    QElapsedTimer timer;
    timer.start();

    int len = input.size();
    QVector<double> output(len, 0.0);
    fadeSamples = qMax(1, fadeSamples);

    for (int i = 0; i < len; ++i) {
        if (i < fadeSamples) {
            /* 淡入: position从1渐变到0，取gainA的增长 */
            double pos = 1.0 - static_cast<double>(i) / static_cast<double>(fadeSamples);
            output[i] = input[i] * gainA(pos);
        } else {
            output[i] = input[i];
        }
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalCrossfades++;
    m_stats.totalSamplesProcessed += len;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCrossfades;

    return output;
}

/**
 * @brief 重置所有累积统计信息
 */
void Crossfader::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
