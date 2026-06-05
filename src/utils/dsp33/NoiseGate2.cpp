/**
 * @file NoiseGate2.cpp
 * @brief 噪声门增强实现 — 迟滞比较/Attack-Release-Hold包络/状态追踪
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp33/NoiseGate2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/**
 * @brief 构造函数，初始化默认DSP参数
 * @param parent 父对象
 */
NoiseGate2::NoiseGate2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("NoiseGate2"));
}

/**
 * @brief 设置开启阈值(dB)
 *
 * 信号电平高于此阈值时门开启。实际阈值考虑迟滞窗口。
 *
 * @param db 阈值(dB)，典型值 -60 ~ 0
 */
void NoiseGate2::setThreshold(double db)
{
    m_threshold = db;
}

/**
 * @brief 设置迟滞宽度(dB)
 *
 * 迟滞防止阈值附近的信号造成门快速开关抖动。
 * 开启阈值 = threshold，关闭阈值 = threshold - hysteresis。
 *
 * @param db 迟滞宽度(dB)，典型值 3 ~ 12
 */
void NoiseGate2::setHysteresis(double db)
{
    m_hysteresis = qMax(0.0, db);
}

/**
 * @brief 设置Attack时间(ms)
 *
 * 门从关闭到完全开启的过渡时间。
 *
 * @param ms Attack时间(毫秒)，典型值 0.1 ~ 10
 */
void NoiseGate2::setAttack(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置Release时间(ms)
 *
 * 门从开启到完全关闭的过渡时间。
 *
 * @param ms Release时间(毫秒)，典型值 10 ~ 500
 */
void NoiseGate2::setRelease(double ms)
{
    m_release = qMax(0.01, ms);
}

/**
 * @brief 设置Hold时间(ms)
 *
 * 门开启后至少保持开启的时间，防止短间隔的快速开关。
 *
 * @param ms Hold时间(毫秒)，典型值 5 ~ 100
 */
void NoiseGate2::setHold(double ms)
{
    m_hold = qMax(0.0, ms);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void NoiseGate2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 处理单个采样点
 *
 * 流程: 计算包络 → 迟滞比较判定 → Hold计时 → Attack/Release增益 → 输出
 *
 * @param sample 输入采样值（线性幅度）
 * @return 门控后的输出采样值
 */
double NoiseGate2::processOne(double sample)
{
    QElapsedTimer timer;
    timer.start();

    /* 计算输入信号的包络(dB)
     * 使用绝对值平方包络检测，平滑系数约为10ms */
    double absSample = qFabs(sample);
    double sampleDb = 20.0 * qLn(qMax(absSample, 1e-10)) / qLn(10.0);

    /* 平滑包络: 简单一阶IIR，时间常数约10ms */
    double smoothCoeff = qExp(-1.0 / (0.01 * m_sampleRate));
    m_envelope = smoothCoeff * m_envelope + (1.0 - smoothCoeff) * sampleDb;

    /* 迟滞比较器判定 */
    double openThreshold = m_threshold;
    double closeThreshold = m_threshold - m_hysteresis;

    if (!m_gateOpen) {
        /* 门当前关闭: 只有包络超过开启阈值才打开 */
        if (m_envelope >= openThreshold) {
            m_gateOpen = true;
            m_holdTimer = m_hold * m_sampleRate / 1000.0;
            emit gateChanged(true);
        }
    } else {
        /* 门当前开启: Hold计时递减 */
        if (m_holdTimer > 0) {
            m_holdTimer -= 1.0;
        } else {
            /* Hold结束后检查是否应关闭 */
            if (m_envelope < closeThreshold) {
                m_gateOpen = false;
                emit gateChanged(false);
            }
        }
    }

    /* 计算Attack/Release增益
     * 使用指数衰减模型: gain趋向target */
    double targetGain = m_gateOpen ? 1.0 : 0.0;

    double coeff;
    if (targetGain > 0.5) {
        /* Attack: 快速开启 */
        coeff = qExp(-1.0 / (m_attack * m_sampleRate / 1000.0));
    } else {
        /* Release: 缓慢关闭 */
        coeff = qExp(-1.0 / (m_release * m_sampleRate / 1000.0));
    }

    /* m_envelope在此处复用为增益平滑变量 */
    static double gain = 0.0;
    gain = coeff * gain + (1.0 - coeff) * targetGain;
    gain = qBound(0.0, gain, 1.0);

    /* 更新统计信息 */
    m_stats.totalSamplesProcessed++;

    return sample * gain;
}

/**
 * @brief 批量处理采样向量
 *
 * 对每个采样点依次调用processOne。
 *
 * @param input 输入采样向量
 * @return 门控后的输出向量
 */
QVector<double> NoiseGate2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i) {
        output[i] = processOne(input[i]);
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalGateEvents++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGateEvents;

    return output;
}

/**
 * @brief 查询当前门状态
 * @return true=门开启，false=门关闭
 */
bool NoiseGate2::isGateOpen() const
{
    return m_gateOpen;
}

/**
 * @brief 重置门状态（包络、计时器、门状态）
 */
void NoiseGate2::reset()
{
    m_envelope = -120.0;
    m_gateOpen = false;
    m_holdTimer = 0.0;
}

/**
 * @brief 重置所有累积统计信息
 */
void NoiseGate2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
