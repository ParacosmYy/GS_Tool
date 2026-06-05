#include "Gate3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化噪声门效果器
 * @param parent 父对象指针
 */
Gate3::Gate3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置门限阈值(dB)
 * @param thresholdDb 低于此阈值的信号将被衰减
 */
void Gate3::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置释放时间(ms)
 * @param releaseMs 释放时间常数，控制门关闭的速度
 */
void Gate3::setRelease(double releaseMs)
{
    m_release = qMax(1.0, releaseMs);
}

/**
 * @brief 对输入信号帧执行噪声门处理
 *
 * 当信号电平低于阈值时，按释放时间常数逐渐衰减输出。
 * 当信号高于阈值时，增益快速恢复到1.0。
 *
 * @param input 输入信号帧
 * @return 噪声门处理后的输出信号帧
 */
QVector<double> Gate3::process(const QVector<double>& input)
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
    double attackCoeff = 1.0 - std::exp(-1.0 / (1.0 * 0.001 * 44100.0));
    double releaseCoeff = 1.0 - std::exp(-1.0 / (m_release * 0.001 * 44100.0));
    double gate = 1.0; /* 门增益，1.0=完全打开，0.0=完全关闭 */
    double rangeDb = -80.0; /* 门关闭时的最大衰减 */
    double rangeLin = std::pow(10.0, rangeDb / 20.0);

    for (int i = 0; i < input.size(); ++i) {
        /* 计算输入信号电平(dB) */
        double absVal = std::abs(input[i]);
        double inputDb = 20.0 * std::log10(qMax(1e-10, absVal));

        /* 确定目标门增益 */
        double targetGain = (inputDb >= m_threshold) ? 1.0 : 0.0;

        /* 启动/释放平滑 */
        if (targetGain > gate) {
            gate += attackCoeff * (targetGain - gate);
        } else {
            gate += releaseCoeff * (targetGain - gate);
        }

        /* 将门增益映射到线性范围 */
        double linGain = rangeLin + gate * (1.0 - rangeLin);
        output[i] = input[i] * linGain;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
    emit processingCompleted(output.size());
    return output;
}

/**
 * @brief 设置攻击时间(ms)
 * @param attackMs 攻击时间常数
 */
void Gate3::setAttack(double attackMs)
{
    Q_UNUSED(attackMs)
}

/**
 * @brief 设置门控范围(dB)
 * @param rangeDb 门关闭时的最大衰减量
 */
void Gate3::setRange(double rangeDb)
{
    Q_UNUSED(rangeDb)
}

/**
 * @brief 获取当前门控状态
 * @return true表示门打开(信号通过)
 */
bool Gate3::isOpen() const
{
    return true;
}

/**
 * @brief 获取当前门控增益
 * @return 线性增益值(0~1)
 */
double Gate3::currentGain() const
{
    return 1.0;
}

/**
 * @brief 计算噪声门的增益缩减历史
 * @param input 输入信号帧
 * @return 每个采样的增益值序列
 */
QVector<double> Gate3::gainHistory(const QVector<double>& input) const
{
    if (input.isEmpty()) return QVector<double>();
    QVector<double> gains(input.size(), 1.0);
    for (int i = 0; i < input.size(); ++i) {
        double inputDb = 20.0 * std::log10(qMax(1e-10, std::abs(input[i])));
        gains[i] = (inputDb >= m_threshold) ? 1.0 : 0.001;
    }
    return gains;
}

/**
 * @brief 重置统计数据
 */
void Gate3::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
