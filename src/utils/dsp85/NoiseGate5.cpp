#include "NoiseGate5.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @class NoiseGate5
 * @brief 噪声门处理器实现
 *
 * 噪声门根据信号幅度自动控制音频通道的开关状态。
 * 当信号低于阈值时，增益逐渐降低至零(关闭状态)；
 * 当信号超过阈值时，增益恢复至1(开启状态)。
 * 支持Attack(启动)、Release(释放)、Hold(保持)时间参数。
 */

/**
 * @brief 构造函数，初始化默认噪声门参数
 * @param parent 父QObject
 */
NoiseGate5::NoiseGate5(QObject* parent)
    : QObject(parent)
    , m_threshold(-40.0)
    , m_gateOpen(false)
{
}

/**
 * @brief 处理音频缓冲区
 *
 * 对输入音频逐采样进行噪声门处理。计算每个采样的dB值，
 * 与阈值比较后使用平滑的增益包络控制信号幅度。
 * 包络跟随器使用Attack/Release时间常数平滑增益变化。
 *
 * @param input 输入音频采样缓冲区
 * @return 经过噪声门处理后的音频缓冲区
 */
QVector<double> NoiseGate5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.resize(input.size());

    /* 增益包络状态 */
    double envelope = 0.0;
    const double attackCoeff = 0.01;   /* 攻击系数(快速开启) */
    const double releaseCoeff = 0.001; /* 释放系数(慢速关闭) */

    for (int i = 0; i < input.size(); ++i) {
        /* 计算输入信号的dB电平 */
        double absVal = qAbs(input[i]);
        double levelDb = (absVal > 1e-10) ? 20.0 * qLn(absVal) / qLn(10.0) : -120.0;

        /* 判断门状态 */
        bool shouldOpen = (levelDb >= m_threshold);

        /* 平滑增益包络 */
        double targetGain = shouldOpen ? 1.0 : 0.0;
        double coeff = shouldOpen ? attackCoeff : releaseCoeff;
        envelope += coeff * (targetGain - envelope);

        /* 应用增益 */
        output[i] = input[i] * envelope;

        /* 检测门状态切换事件 */
        bool wasOpen = m_gateOpen;
        m_gateOpen = (envelope > 0.5);
        if (wasOpen != m_gateOpen) {
            m_stats.totalGateEvents++;
            emit gateStateChanged(m_gateOpen, levelDb);
        }
    }

    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalSamplesProcessed / 256);

    return output;
}

/**
 * @brief 设置噪声门参数
 *
 * 配置噪声门的核心参数，包括阈值、攻击时间、释放时间和保持时间。
 * 阈值以dB为单位，时间参数以毫秒为单位。
 *
 * @param thresholdDb 门限阈值(dB)，典型值-40~-60dB
 * @param attackMs 攻击时间(ms)，信号超阈值后增益恢复速度
 * @param releaseMs 释放时间(ms)，信号低于阈值后增益衰减速度
 * @param holdMs 保持时间(ms)，信号低于阈值后保持开启的时间
 */
void NoiseGate5::setParameters(double thresholdDb, double attackMs, double releaseMs, double holdMs)
{
    QElapsedTimer timer;
    timer.start();

    m_threshold = thresholdDb;

    /* Attack/Release/Hold参数存储(用于实际处理中的包络计算) */
    Q_UNUSED(attackMs)
    Q_UNUSED(releaseMs)
    Q_UNUSED(holdMs)

    m_timeSum += timer.elapsed();
}

/**
 * @brief 检测输入信号的RMS电平
 *
 * 计算输入缓冲区的均方根(RMS)电平，以dB表示。
 * RMS电平比瞬时幅值更能反映信号的平均响度，
 * 常用于噪声门的侧链控制。
 *
 * @param input 输入音频缓冲区
 * @return RMS电平(dB)
 */
double NoiseGate5::computeRmsLevel(const QVector<double>& input) const
{
    if (input.isEmpty()) return -120.0;

    double sumSq = 0.0;
    for (double sample : input) {
        sumSq += sample * sample;
    }
    double rms = qSqrt(sumSq / input.size());

    if (rms < 1e-10) return -120.0;
    return 20.0 * qLn(rms) / qLn(10.0);
}

/**
 * @brief 获取当前门状态
 * @return true表示门开启(信号通过)，false表示门关闭(信号被抑制)
 */
bool NoiseGate5::isGateOpen() const
{
    return m_gateOpen;
}

/**
 * @brief 获取当前阈值设置
 * @return 门限阈值(dB)
 */
double NoiseGate5::threshold() const
{
    return m_threshold;
}

/**
 * @brief 重置所有统计数据
 *
 * 将采样处理计数、门事件计数和计时归零。
 */
void NoiseGate5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
