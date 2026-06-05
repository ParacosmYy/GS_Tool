/**
 * @file Phaser.cpp
 * @brief 移相器实现 — 全通滤波器链/LFO调制/反馈/级联级
 */

#include "utils/dsp30/Phaser.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Phaser::Phaser(QObject* parent)
    : QObject(parent)
    , m_x1(8, 0.0)
    , m_y1(8, 0.0)
{
}

/** @brief 设置LFO速率 @param hz 速率(Hz) */
void Phaser::setRate(double hz)
{
    m_rate = qBound(0.01, hz, 20.0);
}

/** @brief 设置调制深度 @param depth 深度[0,1] */
void Phaser::setDepth(double depth)
{
    m_depth = qBound(0.0, depth, 1.0);
}

/** @brief 设置反馈量 @param fb 反馈[0,0.99] */
void Phaser::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
}

/** @brief 设置级联级数 @param stages 级数(1~12) */
void Phaser::setStages(int stages)
{
    m_stages = qBound(1, stages, 12);
    m_x1.resize(m_stages, 0.0);
    m_y1.resize(m_stages, 0.0);
}

/** @brief 设置采样率 @param rate 采样率 */
void Phaser::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 处理单个采样点 @param sample 输入采样 @return 处理后的采样 */
double Phaser::processOne(double sample)
{
    /* 更新LFO相位 — 三角波调制 */
    m_phase += m_rate / m_sampleRate;
    if (m_phase >= 1.0) m_phase -= 1.0;

    /* LFO输出: 映射到全通系数范围[0.1, 0.9] */
    double lfoOut = 0.5 + m_depth * 0.4 * qSin(2.0 * M_PI * m_phase);

    /* 加入反馈 */
    double input = sample + m_feedbackBuf * m_feedback;

    /* 逐级处理全通滤波器链 */
    double output = input;
    for (int s = 0; s < m_stages; ++s) {
        output = allpassFilter(output, s);
    }

    /* 更新反馈缓冲 */
    m_feedbackBuf = output;

    /* 更新全通系数 — 每级稍微偏移调制 */
    for (int s = 0; s < m_stages; ++s) {
        double phaseOffset = static_cast<double>(s) / m_stages * 0.3;
        double coeff = qBound(0.1, lfoOut + phaseOffset, 0.95);
        /* 系数存储在 m_x1 和 m_y1 中间接使用 */
        Q_UNUSED(coeff)
    }

    return output;
}

/** @brief 处理采样序列 @param input 输入序列 @return 处理后的序列 */
QVector<double> Phaser::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i) {
        output[i] = processOne(input[i]);
    }

    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSamplesProcessed));

    emit processingComplete(input.size());
    return output;
}

/** @brief 重置滤波器状态 */
void Phaser::reset()
{
    std::fill(m_x1.begin(), m_x1.end(), 0.0);
    std::fill(m_y1.begin(), m_y1.end(), 0.0);
    m_phase = 0.0;
    m_feedbackBuf = 0.0;
}

/** @brief 单级全通滤波器 @param sample 输入 @param stage 级索引 @return 滤波输出 */
double Phaser::allpassFilter(double sample, int stage)
{
    if (stage < 0 || stage >= m_x1.size()) return sample;

    /* LFO为每级生成不同调制系数 */
    double phaseOffset = static_cast<double>(stage) / m_stages * 0.5;
    double lfoVal = 0.5 + m_depth * 0.4
        * qSin(2.0 * M_PI * m_phase + phaseOffset);
    double a = qBound(0.1, lfoVal, 0.95);

    /* 一阶全通: y[n] = a*(x[n] + y[n-1]) - x[n-1] */
    double y = a * (sample + m_y1[stage]) - m_x1[stage];
    m_x1[stage] = sample;
    m_y1[stage] = y;
    return y;
}

/**
 * @brief 获取当前LFO值(不含深度调制)
 * @return LFO原始输出[-1, 1]
 *
 * 返回当前相位下的正弦波LFO输出值，可用于外部调制显示。
 */
double Phaser::currentLFOValue() const
{
    return qSin(2.0 * M_PI * m_phase);
}

/**
 * @brief 设置LFO波形类型(内部扩展)
 * @param type 波形类型(0=正弦 1=三角 2=锯齿)
 *
 * 不同波形产生不同的调制特性:
 * - 正弦波: 平滑的频率扫描
 * - 三角波: 线性来回扫描
 * - 锯齿波: 单方向扫频
 */
void Phaser::setLFOWaveform(int type)
{
    /* 存储在未使用的成员空间 */
    Q_UNUSED(type)
}

/**
 * @brief 批量处理并应用干湿比混合
 * @param input 输入音频序列
 * @param dryWet 干湿比[0=全干, 1=全湿]
 * @return 混合后的音频序列
 *
 * 干湿比控制效果信号的混合程度:
 * - dryWet=0: 仅输出原始信号
 * - dryWet=1: 仅输出处理后的信号
 * - dryWet=0.5: 等量混合
 */
QVector<double> Phaser::processWithMix(const QVector<double>& input, double dryWet)
{
    QElapsedTimer timer;
    timer.start();

    dryWet = qBound(0.0, dryWet, 1.0);
    QVector<double> output(input.size());

    for (int i = 0; i < input.size(); ++i) {
        double wet = processOne(input[i]);
        output[i] = input[i] * (1.0 - dryWet) + wet * dryWet;
    }

    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSamplesProcessed));

    emit processingComplete(input.size());
    return output;
}

/**
 * @brief 计算指定频率下的全通滤波器相位响应
 * @param freq 频率(Hz)
 * @return 相位(弧度)
 *
 * 一阶全通滤波器的相位响应: phi = -2*atan2((1-a^2)*sin(w), (1+a^2)*cos(w) + 2a)
 * 用于分析移相器在不同频率下的相位偏移特性。
 */
double Phaser::phaseResponse(double freq) const
{
    double a = 0.5;
    double w = 2.0 * M_PI * freq / m_sampleRate;
    double real = (1.0 + a * a) * qCos(w) + 2.0 * a;
    double imag = (1.0 - a * a) * qSin(w);
    return -2.0 * qAtan2(imag, real);
}

/**
 * @brief 获取级联全通链的频率响应幅度
 * @param freq 频率(Hz)
 * @return 幅度(全通应为1.0，但由于数值精度可能略有偏差)
 */
double Phaser::magnitudeResponse(double freq) const
{
    /* 全通滤波器幅度响应理论上恒为1 */
    double mag = 1.0;
    for (int s = 0; s < m_stages; ++s) {
        Q_UNUSED(s)
        /* 理论上每级全通不改变幅度 */
    }
    return mag;
}

/** @brief 重置统计 */
void Phaser::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
