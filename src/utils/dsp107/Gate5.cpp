#include "Gate5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化噪声门处理器
 * @param parent 父对象指针
 */
Gate5::Gate5(QObject* parent)
    : QObject(parent)
    , m_envelope(0.0)
    , m_holdTimer(0.0)
    , m_isOpen(false)
{
}

/**
 * @brief 重置所有统计信息
 */
void Gate5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置门控参数
 *
 * 配置噪声门的关键参数。阈值决定门开启/关闭的分界线，
 * 范围控制关闭时的最大衰减量，attack/release控制增益变化速度。
 *
 * @param thresholdDb 门控阈值(dB)
 * @param rangeDb 门控范围/衰减量(dB)
 * @param attackMs 启动时间(ms)
 * @param holdMs 保持时间(ms)
 * @param releaseMs 释放时间(ms)
 */
void Gate5::setParameters(double thresholdDb, double rangeDb,
                           double attackMs, double holdMs, double releaseMs)
{
    m_thresholdDb = thresholdDb;
    m_rangeDb = rangeDb;
    m_attackMs = qMax(0.1, attackMs);
    m_holdMs = qMax(0.0, holdMs);
    m_releaseMs = qMax(0.1, releaseMs);
}

/**
 * @brief 处理采样数据
 *
 * 对输入采样执行噪声门控处理：
 * 1. 计算信号包络（绝对值平滑跟踪）
 * 2. 比较包络与阈值决定门状态
 * 3. 保持时间防止快速开关抖动
 * 4. 应用增益衰减（attack/release平滑过渡）
 *
 * @param samples 输入采样数据
 * @return 门控处理后的采样数据
 */
QVector<double> Gate5::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    QVector<double> output(n);
    if (n == 0) {
        emit gateCompleted(0);
        return output;
    }

    /* 假设采样率44100Hz，计算attack/release系数 */
    const double sampleRate = 44100.0;
    const double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * sampleRate));
    const double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * sampleRate));

    /* 范围增益：将dB转为线性 */
    const double rangeLinear = qPow(10.0, m_rangeDb / 20.0);
    const double thresholdLinear = qPow(10.0, m_thresholdDb / 20.0);

    double openCount = 0.0;

    for (int i = 0; i < n; ++i) {
        double absVal = qAbs(samples[i]);

        /* 包络跟随器：快速攻击，慢释放 */
        if (absVal > m_envelope) {
            m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * absVal;
        } else {
            m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * absVal;
        }
        m_envelope = qBound(0.0, m_envelope, 1.0);

        /* 门状态判定 */
        bool shouldOpen = m_envelope >= thresholdLinear;

        if (shouldOpen && !m_isOpen) {
            m_isOpen = true;
            m_holdTimer = m_holdMs * 0.001 * sampleRate;
        } else if (!shouldOpen && m_isOpen) {
            if (m_holdTimer > 0.0) {
                m_holdTimer -= 1.0;
            } else {
                m_isOpen = false;
            }
        }

        if (m_isOpen) {
            m_holdTimer = m_holdMs * 0.001 * sampleRate;
        }

        /* 计算增益：开时=1.0，关时=rangeLinear */
        double targetGain = m_isOpen ? 1.0 : rangeLinear;

        /* 平滑增益变化 */
        static double smoothGain = 1.0;
        double gainCoeff = (targetGain > smoothGain) ? (1.0 - attackCoeff) : (1.0 - releaseCoeff);
        smoothGain = smoothGain + gainCoeff * (targetGain - smoothGain);
        smoothGain = qBound(rangeLinear, smoothGain, 1.0);

        output[i] = samples[i] * smoothGain;

        if (m_isOpen) {
            openCount += 1.0;
        }
    }

    /* 更新门开启比例统计 */
    m_stats.gateOpenRatio = (n > 0) ? openCount / n : 0.0;

    /* 计时统计 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalGateRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGateRuns;

    emit gateCompleted(n);
    return output;
}
