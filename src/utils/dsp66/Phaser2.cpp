/**
 * @file Phaser2.cpp
 * @brief 移相器音效处理器实现（第2版）
 *
 * 使用级联全通滤波器实现移相效果。LFO（低频振荡器）调制
 * 全通滤波器的截止频率，产生相位抵消的"嗖嗖"扫频效果。
 * 支持速率、深度、反馈、级数和混合比等参数控制。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp66/Phaser2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化移相器处理器
 * @param parent 父QObject对象指针
 */
Phaser2::Phaser2(QObject* parent)
    : QObject(parent)
{
    /* 初始化全通滤波器状态 */
    m_allpassX.resize(m_stages, 0.0);
    m_allpassY.resize(m_stages, 0.0);
}

/**
 * @brief 设置LFO速率
 * @param hz LFO频率（Hz），范围0.01~20Hz
 */
void Phaser2::setRate(double hz)
{
    m_rate = qBound(0.01, hz, 20.0);
}

/**
 * @brief 设置调制深度
 * @param depth 深度值（0.0~1.0）
 */
void Phaser2::setDepth(double depth)
{
    m_depth = qBound(0.0, depth, 1.0);
}

/**
 * @brief 设置反馈系数
 * @param fb 反馈量（0.0~0.99）
 */
void Phaser2::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
}

/**
 * @brief 设置全通滤波器级数
 * @param stages 级数（2~12），越多效果越明显
 */
void Phaser2::setStages(int stages)
{
    m_stages = qBound(2, stages, 12);
    m_allpassX.resize(m_stages, 0.0);
    m_allpassY.resize(m_stages, 0.0);
}

/**
 * @brief 设置干湿混合比
 * @param mix 湿信号比例（0.0~1.0）
 */
void Phaser2::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

/**
 * @brief 二阶全通滤波器计算
 *
 * 使用一阶全通传递函数 H(z) = (a - z^-1) / (1 - a*z^-1)，
 * a由频率决定。实现相位偏移而幅度不变。
 *
 * @param x 输入样本
 * @param freq 全通滤波器的中心频率
 * @return 经全通滤波后的样本
 */
double Phaser2::allpassFilter(double x, double freq)
{
    /* 计算全通系数 */
    double wt = 2.0 * M_PI * freq / 44100.0;
    double a = (qTan(wt / 2.0) - 1.0) / (qTan(wt / 2.0) + 1.0);

    /* 简化的一阶全通滤波器（使用内部状态） */
    double y = -a * x + m_allpassY[0];
    m_allpassY[0] = x + a * y;

    return y;
}

/**
 * @brief 处理输入音频信号，添加移相效果
 *
 * LFO产生三角波调制信号，驱动级联全通滤波器
 * 的截止频率变化。反馈回路增强效果强度。
 *
 * @param input 输入音频采样序列
 * @return 添加移相效果后的音频序列
 */
QVector<double> Phaser2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) {
        emit processingCompleted(0, m_lfoPos);
        return output;
    }

    int n = input.size();
    output.resize(n);

    double phase = m_lfoPos;
    double phaseInc = m_rate / 44100.0;

    /* LFO扫频范围（全通中心频率范围） */
    double minFreq = 200.0;
    double maxFreq = 8000.0;

    for (int s = 0; s < n; ++s) {
        /* 生成三角波LFO */
        double lfoVal = 2.0 * qAbs(2.0 * (phase - qFloor(phase + 0.5)));
        double modFreq = minFreq + (maxFreq - minFreq) * lfoVal * m_depth;

        /* 级联全通滤波器 */
        double filtered = input[s];
        for (int stage = 0; stage < m_stages; ++stage) {
            /* 每级使用略微偏移的频率 */
            double stageFreq = modFreq * (1.0 + stage * 0.15);
            stageFreq = qBound(20.0, stageFreq, 20000.0);

            double wt = 2.0 * M_PI * stageFreq / 44100.0;
            double tanVal = qTan(wt / 2.0);
            double a = (tanVal - 1.0) / (tanVal + 1.0);

            double x = filtered;
            if (stage < m_allpassX.size() && stage < m_allpassY.size()) {
                double y = -a * x + a * m_allpassX[stage] + m_allpassY[stage];
                m_allpassX[stage] = x;
                m_allpassY[stage] = y;
                filtered = y;
            }
        }

        /* 添加反馈 */
        double wet = filtered + m_feedback * filtered;

        /* 干湿混合 */
        output[s] = input[s] * (1.0 - m_mix) + wet * m_mix;

        /* 更新LFO相位 */
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }

    m_lfoPos = phase;

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(n, m_lfoPos);
    return output;
}

/**
 * @brief 获取当前统计信息
 * @return 处理统计结构
 */
Phaser2::Stats Phaser2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void Phaser2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 重置滤波器状态
 *
 * 清除所有全通滤波器的内部状态和LFO相位。
 * 在处理新的音频段之前调用以避免状态泄漏。
 */
void Phaser2::resetState()
{
    m_allpassX.fill(0.0);
    m_allpassY.fill(0.0);
    m_lfoPos = 0.0;
}

/**
 * @brief 计算当前LFO相位对应的调制频率
 *
 * 将当前LFO位置映射到全通滤波器的中心频率范围。
 *
 * @return 当前的调制频率（Hz）
 */
double Phaser2::currentModFrequency() const
{
    double minFreq = 200.0;
    double maxFreq = 8000.0;
    double lfoVal = 2.0 * qAbs(2.0 * (m_lfoPos - qFloor(m_lfoPos + 0.5)));
    return minFreq + (maxFreq - minFreq) * lfoVal * m_depth;
}

/**
 * @brief 获取当前LFO相位角度
 * @return 相位角度（0~360度）
 */
double Phaser2::lfoPhaseDegrees() const
{
    return m_lfoPos * 360.0;
}
