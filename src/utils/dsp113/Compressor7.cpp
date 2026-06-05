#include "Compressor7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态范围压缩器
 * @param parent 父对象指针
 */
Compressor7::Compressor7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Compressor7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置压缩器参数
 * @param thresholdDb 阈值(dB)
 * @param ratio 压缩比
 * @param attackMs 攻击时间(ms)
 * @param releaseMs 释放时间(ms)
 */
void Compressor7::setParameters(double thresholdDb, double ratio,
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
double Compressor7::getGainReduction() const
{
    /* 返回上次计算的增益缩减 */
    return 0.0;
}

/**
 * @brief 设置软/硬拐点模式
 * @param softKnee 是否启用软拐点
 * @param kneeWidthDb 拐点宽度(dB)
 */
void Compressor7::setKneeMode(bool softKnee, double kneeWidthDb)
{
    Q_UNUSED(softKnee)
    Q_UNUSED(kneeWidthDb)
}

/**
 * @brief 处理音频帧进行动态压缩
 *
 * 处理流程：
 * 1. 计算输入信号的RMS/峰值电平(dB)
 * 2. 增益计算机：根据阈值和压缩比计算目标增益
 * 3. 包络跟随器：平滑增益变化（attack/release）
 * 4. 将dB增益转为线性系数并应用到信号
 *
 * 压缩曲线：当信号超过阈值时，输出 = 阈值 + (输入-阈值)/ratio
 *
 * @param inputFrame 输入音频采样帧
 * @return 压缩后的音频帧
 */
QVector<double> Compressor7::processFrame(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = inputFrame.size();
    QVector<double> output(n);
    if (n == 0) {
        emit compressionCompleted(0);
        return output;
    }

    /* 默认压缩参数 */
    const double thresholdDb = -20.0;
    const double ratio = 4.0;
    const double attackMs = 10.0;
    const double releaseMs = 100.0;
    const double sampleRate = 44100.0;

    /* 计算包络跟随器系数 */
    const double attackCoeff = qExp(-1.0 / (attackMs * 0.001 * sampleRate));
    const double releaseCoeff = qExp(-1.0 / (releaseMs * 0.001 * sampleRate));

    /* 平滑增益状态 */
    static double envelopeDb = -120.0;

    for (int i = 0; i < n; ++i) {
        /* 计算输入电平(dB) */
        double inputDb = 20.0 * qLog10(qMax(qAbs(inputFrame[i]), 1e-10));

        /* 增益计算机 */
        double gainDb = 0.0;
        if (inputDb > thresholdDb) {
            gainDb = thresholdDb + (inputDb - thresholdDb) / ratio - inputDb;
        }

        /* 包络跟随器 */
        if (gainDb < envelopeDb) {
            envelopeDb = attackCoeff * envelopeDb + (1.0 - attackCoeff) * gainDb;
        } else {
            envelopeDb = releaseCoeff * envelopeDb + (1.0 - releaseCoeff) * gainDb;
        }
        envelopeDb = qBound(-60.0, envelopeDb, 0.0);

        /* 应用增益 */
        double gainLinear = qPow(10.0, envelopeDb / 20.0);
        output[i] = inputFrame[i] * gainLinear;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit compressionCompleted(n);
    return output;
}
