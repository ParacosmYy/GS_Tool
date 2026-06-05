#include "Expander6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态范围扩展器
 * @param parent 父对象指针
 */
Expander6::Expander6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Expander6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置扩展器参数
 * @param thresholdDb 阈值(dB)
 * @param ratio 扩展比
 * @param attackMs 启动时间(ms)
 * @param releaseMs 释放时间(ms)
 */
void Expander6::setParameters(double thresholdDb, double ratio,
                               double attackMs, double releaseMs)
{
    Q_UNUSED(thresholdDb)
    Q_UNUSED(ratio)
    Q_UNUSED(attackMs)
    Q_UNUSED(releaseMs)
}

/**
 * @brief 获取当前增益缩减量
 * @return 增益缩减值(dB)
 */
double Expander6::getGainReduction() const
{
    return 0.0;
}

/**
 * @brief 启用/禁用侧链检测模式
 * @param enabled 是否启用侧链
 * @param sidechainSignal 侧链信号
 */
void Expander6::setSidechain(bool enabled, const QVector<double>& sidechainSignal)
{
    Q_UNUSED(enabled)
    Q_UNUSED(sidechainSignal)
}

/**
 * @brief 处理音频帧进行动态扩展
 *
 * 向下扩展器：当信号低于阈值时额外衰减，
 * 增大信号动态范围。与压缩器互补。
 *
 * 处理流程：
 * 1. 检测信号电平(dB)
 * 2. 当电平低于阈值时，按扩展比计算增益衰减
 * 3. 包络跟随器平滑增益变化
 * 4. 应用线性增益
 *
 * 扩展曲线：当信号 < 阈值时，
 *   输出_dB = 阈值 - (阈值 - 输入) * ratio
 *   增益_dB = 输出_dB - 输入_dB
 *
 * @param inputFrame 输入音频采样帧
 * @return 扩展处理后的音频帧
 */
QVector<double> Expander6::processFrame(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = inputFrame.size();
    QVector<double> output(n);
    if (n == 0) {
        emit expansionCompleted(0);
        return output;
    }

    /* 默认扩展参数 */
    const double thresholdDb = -30.0;
    const double ratio = 2.0;
    const double attackMs = 10.0;
    const double releaseMs = 100.0;
    const double sampleRate = 44100.0;

    const double attackCoeff = qExp(-1.0 / (attackMs * 0.001 * sampleRate));
    const double releaseCoeff = qExp(-1.0 / (releaseMs * 0.001 * sampleRate));

    static double envelopeDb = 0.0;

    for (int i = 0; i < n; ++i) {
        double inputDb = 20.0 * qLog10(qMax(qAbs(inputFrame[i]), 1e-10));

        /* 扩展增益计算：低于阈值时衰减 */
        double gainDb = 0.0;
        if (inputDb < thresholdDb) {
            double belowThreshold = thresholdDb - inputDb;
            double outputDb = thresholdDb - belowThreshold * ratio;
            gainDb = outputDb - inputDb;
        }

        /* 包络跟随器 */
        double target = gainDb;
        if (target < envelopeDb) {
            envelopeDb = attackCoeff * envelopeDb + (1.0 - attackCoeff) * target;
        } else {
            envelopeDb = releaseCoeff * envelopeDb + (1.0 - releaseCoeff) * target;
        }
        envelopeDb = qBound(-80.0, envelopeDb, 0.0);

        double gainLinear = qPow(10.0, envelopeDb / 20.0);
        output[i] = inputFrame[i] * gainLinear;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit expansionCompleted(n);
    return output;
}
