/**
 * @file WavetableOsc.cpp
 * @brief 波表合成器实现
 *
 * 基于相位累加器和波表查表的合成引擎。
 * 支持多帧波表间的线性交叉渐变, 线性插值读取,
 * 以及基于波表大小/频率比的抗混叠保护。
 */

#include "utils/dsp25/WavetableOsc.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param tableSize 单帧波表大小(默认2048), 越大抗混叠越好
 * @param parent 父对象
 */
WavetableOsc::WavetableOsc(int tableSize, QObject* parent)
    : QObject(parent)
    , m_tableSize(tableSize)
{
}

/**
 * @brief 加载单帧波表
 *
 * 将单帧数据加载为唯一的一帧波表。
 * 波表大小应与构造函数指定的一致, 若不匹配则自动截断或补零。
 * 加载后重新计算相位增量。
 *
 * @param wavetable 波表采样数据, 应包含恰好tableSize个采样
 * @param sampleRate 目标采样率(Hz)
 */
void WavetableOsc::loadWavetable(const QVector<double>& wavetable, double sampleRate)
{
    m_sampleRate = sampleRate;
    m_frames.clear();
    QVector<double> frame(m_tableSize, 0.0);
    int copyLen = qMin(wavetable.size(), m_tableSize);
    for (int i = 0; i < copyLen; ++i) {
        frame[i] = wavetable[i];
    }
    m_frames.push_back(frame);
    m_phaseIncrement = m_frequency / m_sampleRate;
}

/**
 * @brief 加载多维波表
 *
 * 加载多帧波表数据(从柔和到明亮的音色变化)。
 * 合成时根据m_position在相邻帧间线性插值(交叉渐变)。
 * 每帧大小应与tableSize一致。
 *
 * @param frames 波表帧列表, 通常2~16帧
 * @param sampleRate 目标采样率(Hz)
 */
void WavetableOsc::loadMultidimWavetable(const QVector<QVector<double>>& frames,
                                         double sampleRate)
{
    m_sampleRate = sampleRate;
    m_frames.clear();
    for (const auto& src : frames) {
        QVector<double> frame(m_tableSize, 0.0);
        int copyLen = qMin(src.size(), m_tableSize);
        for (int i = 0; i < copyLen; ++i) {
            frame[i] = src[i];
        }
        m_frames.push_back(frame);
    }
    if (m_frames.isEmpty()) {
        m_frames.push_back(QVector<double>(m_tableSize, 0.0));
    }
    m_phaseIncrement = m_frequency / m_sampleRate;
}

/**
 * @brief 生成单个采样
 *
 * 1. 根据当前相位在波表中线性插值读取采样值
 * 2. 若有多帧, 根据m_position在相邻帧间交叉渐变
 * 3. 推进相位累加器(自动环绕)
 * 4. 抗混叠检查: 当频率过高(基频超过Nyquist/2)时衰减输出
 *
 * @return 输出采样值, 范围[-1, 1]
 */
double WavetableOsc::tick()
{
    if (m_frames.isEmpty()) {
        return 0.0;
    }

    /* 抗混叠: 当频率超过Nyquist/2时逐渐衰减 */
    double nyquistRatio = m_frequency / (m_sampleRate * 0.5);
    double antiAliasGain = 1.0;
    if (nyquistRatio > 0.8) {
        antiAliasGain = qMax(0.0, 1.0 - (nyquistRatio - 0.8) / 0.2);
    }

    double sample = 0.0;

    if (m_frames.size() == 1) {
        /* 单帧: 直接插值 */
        sample = interpolate(m_phase, 0);
    } else {
        /* 多帧: 交叉渐变 */
        double pos = m_position * (m_frames.size() - 1);
        pos = qBound(0.0, pos, static_cast<double>(m_frames.size() - 1));
        int frameLow = static_cast<int>(pos);
        int frameHigh = qMin(frameLow + 1, m_frames.size() - 1);
        double frac = pos - frameLow;
        double sLow = interpolate(m_phase, frameLow);
        double sHigh = interpolate(m_phase, frameHigh);
        sample = sLow * (1.0 - frac) + sHigh * frac;
    }

    /* 推进相位 */
    m_phase += m_phaseIncrement;
    while (m_phase >= 1.0) {
        m_phase -= 1.0;
    }
    while (m_phase < 0.0) {
        m_phase += 1.0;
    }

    return sample * antiAliasGain;
}

/**
 * @brief 批量生成采样
 *
 * 循环调用tick()生成指定数量的采样值。
 * 更新统计信息中的采样生成计数。
 *
 * @param count 需要生成的采样数量
 * @return 输出采样缓冲区
 */
QVector<double> WavetableOsc::generate(int count)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(count);
    for (int i = 0; i < count; ++i) {
        output[i] = tick();
    }

    /* 更新统计 */
    m_stats.totalSamplesGenerated += count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalNotesPlayed + 1);

    return output;
}

/**
 * @brief 设置振荡频率
 *
 * 更新相位增量 = frequency / sampleRate。
 * 当频率变化时发射frequencyChanged信号。
 *
 * @param freq 频率(Hz), 应在(0, sampleRate/2)范围内
 */
void WavetableOsc::setFrequency(double freq)
{
    m_frequency = qMax(0.0, freq);
    m_phaseIncrement = m_frequency / m_sampleRate;
    ++m_stats.totalNotesPlayed;
    emit frequencyChanged(m_frequency);
}

/**
 * @brief 设置多维波表位置
 *
 * 控制在多帧波表中的插值位置, 0.0=第一帧(柔和),
 * 1.0=最后一帧(明亮), 中间值线性交叉渐变。
 *
 * @param position 波表位置, 范围[0, 1]
 */
void WavetableOsc::setPosition(double position)
{
    m_position = qBound(0.0, position, 1.0);
}

/**
 * @brief 重置相位累加器
 * 将相位归零, 重新从波表起始位置开始播放
 */
void WavetableOsc::reset()
{
    m_phase = 0.0;
}

/**
 * @brief 线性插值查表
 *
 * 将[0,1)范围的相位映射到波表索引, 在相邻采样间线性插值。
 * 使用环绕索引确保相位0和1无缝衔接。
 *
 * @param phase 相位[0, 1)
 * @param frame 波表帧索引
 * @return 插值后的采样值
 */
double WavetableOsc::interpolate(double phase, int frame) const
{
    double pos = phase * m_tableSize;
    int idx0 = static_cast<int>(pos) % m_tableSize;
    int idx1 = (idx0 + 1) % m_tableSize;
    double frac = pos - static_cast<int>(pos);

    double s0 = m_frames[frame][idx0];
    double s1 = m_frames[frame][idx1];
    return s0 + (s1 - s0) * frac;
}

/**
 * @brief 重置统计计数器
 * 将采样生成数、音符播放数和平均处理时间归零
 */
void WavetableOsc::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
