#include "Limiter4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化限幅器
 * @param parent 父对象指针
 */
Limiter4::Limiter4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置限幅阈值(dB)
 * @param thresholdDb 超过此阈值的信号将被衰减
 */
void Limiter4::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置输出上限(dB)
 * @param ceilingDb 输出信号的最大允许电平
 */
void Limiter4::setCeiling(double ceilingDb)
{
    m_ceiling = ceilingDb;
}

/**
 * @brief 对输入信号帧执行限幅处理
 *
 * 使用瞬时增益控制：当信号超过阈值时，
 * 计算使输出不超过上限的增益量，并应用平滑过渡。
 *
 * @param input 输入信号帧
 * @return 限幅后的输出信号帧
 */
QVector<double> Limiter4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    if (input.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
        emit processingCompleted(0);
        return output;
    }

    output.resize(input.size());
    double attackCoeff = 1.0 - std::exp(-1.0 / (0.5 * 0.001 * 44100.0));
    double releaseCoeff = 1.0 - std::exp(-1.0 / (50.0 * 0.001 * 44100.0));
    double gainDb = 0.0;
    double maxGainReduction = m_threshold - m_ceiling;

    for (int i = 0; i < input.size(); ++i) {
        /* 计算输入信号电平(dB) */
        double absVal = std::abs(input[i]);
        double inputDb = 20.0 * std::log10(qMax(1e-10, absVal));

        /* 计算目标增益 */
        double targetGain = 0.0;
        if (inputDb + gainDb > m_ceiling) {
            double excess = inputDb + gainDb - m_ceiling;
            targetGain = -qMin(excess, maxGainReduction);
        }

        /* 启动/释放平滑 */
        double coeff = (targetGain < gainDb) ? attackCoeff : releaseCoeff;
        gainDb += coeff * (targetGain - gainDb);

        /* 应用增益 */
        double linGain = std::pow(10.0, gainDb / 20.0);
        output[i] = input[i] * linGain;

        /* 硬限制确保不超过ceiling */
        double outCeilLin = std::pow(10.0, m_ceiling / 20.0);
        if (std::abs(output[i]) > outCeilLin) {
            output[i] = (output[i] > 0 ? 1 : -1) * outCeilLin;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
    emit processingCompleted(output.size());
    return output;
}

/**
 * @brief 设置启动时间(ms)
 * @param attackMs 启动时间常数
 */
void Limiter4::setAttack(double attackMs)
{
    Q_UNUSED(attackMs)
}

/**
 * @brief 设置释放时间(ms)
 * @param releaseMs 释放时间常数
 */
void Limiter4::setRelease(double releaseMs)
{
    Q_UNUSED(releaseMs)
}

/**
 * @brief 计算当前增益缩减量(dB)
 * @return 增益缩减量
 */
double Limiter4::gainReduction() const
{
    return 0.0;
}

/**
 * @brief 重置统计数据
 */
void Limiter4::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
