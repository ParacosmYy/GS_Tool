#include "EnvelopeDetect7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化包络检测器
 * @param parent 父对象指针
 */
EnvelopeDetect7::EnvelopeDetect7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置攻击时间常数(秒)
 * @param seconds 攻击时间，控制包络上升速度
 */
void EnvelopeDetect7::setAttackTime(double seconds)
{
    m_attackTime = qMax(0.0001, seconds);
}

/**
 * @brief 设置释放时间常数(秒)
 * @param seconds 释放时间，控制包络下降速度
 */
void EnvelopeDetect7::setReleaseTime(double seconds)
{
    m_releaseTime = qMax(0.0001, seconds);
}

/**
 * @brief 对输入信号执行包络检测
 *
 * 使用峰值检测器方法提取信号包络：
 * - 当输入绝对值大于当前包络值时，按攻击时间常数上升
 * - 当输入绝对值小于当前包络值时，按释放时间常数下降
 *
 * attack_coeff = 1 - exp(-1 / (attackTime * sampleRate))
 * release_coeff = 1 - exp(-1 / (releaseTime * sampleRate))
 *
 * @param samples 输入音频采样
 */
void EnvelopeDetect7::detect(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalDetected++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetected;
        emit detected(QVector<double>());
        return;
    }

    const int N = samples.size();
    double sampleRate = 44100.0;

    /* 计算攻击和释放系数 */
    double attackCoeff = 1.0 - std::exp(-1.0 / (m_attackTime * sampleRate));
    double releaseCoeff = 1.0 - std::exp(-1.0 / (m_releaseTime * sampleRate));

    /* 峰值包络检测 */
    QVector<double> envelope(N, 0.0);
    double env = 0.0;

    for (int i = 0; i < N; ++i) {
        double input = std::abs(samples[i]);

        if (input > env) {
            /* 攻击：快速上升 */
            env += attackCoeff * (input - env);
        } else {
            /* 释放：缓慢下降 */
            env += releaseCoeff * (input - env);
        }

        envelope[i] = env;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalDetected++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetected;
    emit detected(envelope);
}

/**
 * @brief 使用RMS方法计算信号包络
 *
 * 与峰值检测不同，RMS包络通过平方-平滑-开根号
 * 计算均方根包络，对瞬态响应更平滑。
 *
 * @param samples 输入音频采样
 * @return RMS包络曲线
 */
QVector<double> EnvelopeDetect7::rmsEnvelope(const QVector<double>& samples) const
{
    if (samples.isEmpty()) return QVector<double>();

    const int N = samples.size();
    double sampleRate = 44100.0;
    double coeff = 1.0 - std::exp(-1.0 / (m_releaseTime * sampleRate));

    QVector<double> envelope(N, 0.0);
    double env = 0.0;

    for (int i = 0; i < N; ++i) {
        double input = samples[i] * samples[i]; /* 平方 */
        env += coeff * (input - env);
        envelope[i] = std::sqrt(qMax(0.0, env)); /* 均方根 */
    }

    return envelope;
}

/**
 * @brief 计算包络的峰值因子
 *
 * 峰值因子 = 包络峰值 / 包络RMS
 * 高峰值因子表示瞬态丰富，低峰值因子表示持续音。
 *
 * @param envelope 包络曲线
 * @return 峰值因子
 */
double EnvelopeDetect7::crestFactor(const QVector<double>& envelope) const
{
    if (envelope.isEmpty()) return 0.0;

    double peak = 0.0;
    double rmsSum = 0.0;
    for (double v : envelope) {
        peak = qMax(peak, std::abs(v));
        rmsSum += v * v;
    }
    double rms = std::sqrt(rmsSum / envelope.size());
    return (rms > 1e-10) ? peak / rms : 0.0;
}

/**
 * @brief 重置统计数据
 */
void EnvelopeDetect7::resetStatistics()
{
    m_stats.totalDetected = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
