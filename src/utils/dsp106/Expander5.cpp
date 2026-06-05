#include "Expander5.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化动态范围扩展器
 * @param parent 父对象指针
 */
Expander5::Expander5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Expander5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置扩展参数
 *
 * 当信号低于阈值时，按扩展比向下衰减。
 * 扩展比>1时为向下扩展，ratio=inf时为门限效果。
 *
 * @param thresholdDb 扩展阈值(dB)
 * @param ratio 扩展比
 * @param rangeDb 最大扩展范围(dB)
 */
void Expander5::setExpansion(double thresholdDb, double ratio, double rangeDb)
{
    m_thresholdDb = thresholdDb;
    m_ratio = qMax(1.0, ratio);
    m_rangeDb = qMax(0.0, rangeDb);
}

/**
 * @brief 设置启动和释放时间
 *
 * @param attackMs 启动时间(ms)，控制增益变化到衰减状态的速度
 * @param releaseMs 释放时间(ms)，控制增益恢复的速度
 * @param holdMs 保持时间(ms)，在释放前的等待时间
 */
void Expander5::setTiming(double attackMs, double releaseMs, double holdMs)
{
    m_attackMs = qMax(0.1, attackMs);
    m_releaseMs = qMax(1.0, releaseMs);
    m_holdMs = qMax(0.0, holdMs);
}

/**
 * @brief 处理音频数据扩展
 *
 * 对每个采样点执行：包络跟随 -> 增益计算 -> 增益平滑 -> 应用增益。
 * 当信号电平低于阈值时按比例衰减，保持时间防止快速开关。
 *
 * @param samples 输入采样数据
 * @return 扩展后的采样数据
 */
QVector<double> Expander5::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    QVector<double> output(n);
    double maxExpansion = 0.0;
    double sampleRate = 44100.0; /* 假设采样率 */

    /* 将时间常数转换为采样级别系数 */
    double attackCoeff = qExp(-1.0 / (m_attackMs * sampleRate / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * sampleRate / 1000.0));

    double holdSamples = m_holdMs * sampleRate / 1000.0;
    double holdCounter = 0.0;

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(samples[i]);

        /* 包络跟随 */
        if (absSample > m_envelope) {
            m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * absSample;
            holdCounter = holdSamples; /* 重置保持计数器 */
        } else {
            if (holdCounter > 0) {
                holdCounter -= 1.0;
            } else {
                m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * absSample;
            }
        }

        /* 转换为dB */
        double levelDb = 20.0 * qLn(qMax(m_envelope, 1e-10)) / qLn(10.0);

        /* 计算扩展增益 */
        double gainDb = 0.0;
        if (levelDb < m_thresholdDb) {
            /* 低于阈值时按扩展比衰减 */
            double overDb = m_thresholdDb - levelDb;
            gainDb = -overDb * (m_ratio - 1.0) / m_ratio;

            /* 限制最大衰减范围 */
            if (gainDb < -m_rangeDb) {
                gainDb = -m_rangeDb;
            }
        }

        m_currentGainDb = gainDb;
        if (gainDb < maxExpansion) maxExpansion = gainDb;

        /* 应用增益 */
        double gainLin = qPow(10.0, gainDb / 20.0);
        output[i] = samples[i] * gainLin;
    }

    m_stats.maxExpansionDb = maxExpansion;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalExpansionRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalExpansionRuns;

    emit expansionCompleted(n);
    return output;
}
