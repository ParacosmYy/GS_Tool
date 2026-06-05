#include "Limiter6.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化信号限幅器
 * @param parent 父对象指针
 */
Limiter6::Limiter6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Limiter6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置限幅参数
 * @param ceilingDb 限幅上限(dB)
 * @param releaseMs 释放时间(ms)
 */
void Limiter6::setParameters(double ceilingDb, double releaseMs)
{
    m_ceilingDb = ceilingDb;
    m_releaseMs = qMax(1.0, releaseMs);
}

/**
 * @brief 设置lookahead时间
 * @param lookaheadMs lookahead延迟(ms)
 */
void Limiter6::setLookahead(double lookaheadMs)
{
    m_lookaheadMs = qMax(0.0, lookaheadMs);
}

/**
 * @brief 处理采样数据限幅
 *
 * 砖墙限制器确保信号不超过指定上限。
 * 工作流程：
 * 1. 检测输入信号的峰值电平
 * 2. 计算需要的增益衰减使峰值不超过ceiling
 * 3. 使用平滑的释放曲线避免失真
 * 4. 应用增益到输出信号
 *
 * @param samples 输入采样数据
 * @return 限幅后的采样数据
 */
QVector<double> Limiter6::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    QVector<double> output(n);
    double peakDb = -120.0;

    double ceilingLin = qPow(10.0, m_ceilingDb / 20.0);
    double sampleRate = 44100.0;
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * sampleRate / 1000.0));
    double gainReduction = 1.0; /* 线性增益 */

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(samples[i]);

        /* 检测是否超过上限 */
        if (absSample * gainReduction > ceilingLin) {
            /* 需要增加衰减 */
            double targetGain = ceilingLin / qMax(absSample, 1e-10);
            gainReduction = qMin(gainReduction, targetGain);
        }

        /* 释放：增益逐渐恢复到1.0 */
        if (gainReduction < 1.0) {
            gainReduction = releaseCoeff * gainReduction + (1.0 - releaseCoeff);
        }

        /* 应用增益 */
        output[i] = samples[i] * gainReduction;

        /* 硬限幅保护（砖墙） */
        if (qAbs(output[i]) > ceilingLin) {
            output[i] = (output[i] > 0 ? 1 : -1) * ceilingLin;
        }

        /* 跟踪峰值 */
        double outDb = 20.0 * qLn(qMax(qAbs(output[i]), 1e-10)) / qLn(10.0);
        if (outDb > peakDb) peakDb = outDb;
    }

    m_gainReductionDb = 20.0 * qLn(qMax(gainReduction, 1e-10)) / qLn(10.0);
    m_stats.peakOutputDb = peakDb;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalLimitingRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLimitingRuns;

    emit limitingCompleted(n);
    return output;
}
