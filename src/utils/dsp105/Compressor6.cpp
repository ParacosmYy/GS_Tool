#include "Compressor6.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化动态范围压缩器
 * @param parent 父对象指针
 */
Compressor6::Compressor6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Compressor6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置压缩阈值和比率
 *
 * 信号超过阈值后按比率进行增益衰减。
 * 软拐点(knee)使压缩曲线在阈值附近平滑过渡。
 *
 * @param thresholdDb 压缩阈值(dB)
 * @param ratio 压缩比(如4.0表示4:1)
 * @param kneeDb 软拐点宽度(dB)，0.0为硬拐点
 */
void Compressor6::setThreshold(double thresholdDb, double ratio, double kneeDb)
{
    m_thresholdDb = thresholdDb;
    m_ratio = qMax(1.0, ratio);
    m_kneeDb = qMax(0.0, kneeDb);
}

/**
 * @brief 设置启动和释放时间
 *
 * 启动时间控制增益下降的速度，释放时间控制增益恢复的速度。
 *
 * @param attackMs 启动时间(ms)
 * @param releaseMs 释放时间(ms)
 */
void Compressor6::setAttackRelease(double attackMs, double releaseMs)
{
    m_attackMs = qMax(0.1, attackMs);
    m_releaseMs = qMax(1.0, releaseMs);
}

/**
 * @brief 计算增益衰减量
 *
 * 根据输入电平、阈值、比率和拐点宽度计算目标增益衰减(dB)。
 *
 * @param inputLevelDb 输入信号电平(dB)
 * @return 增益衰减量(dB)，负值表示衰减
 */
static double computeGainReduction(double inputLevelDb, double threshold, double ratio, double knee)
{
    if (knee <= 0.0) {
        /* 硬拐点 */
        if (inputLevelDb > threshold) {
            return threshold + (inputLevelDb - threshold) / ratio - inputLevelDb;
        }
        return 0.0;
    }

    /* 软拐点过渡 */
    double halfKnee = knee / 2.0;
    if (inputLevelDb < threshold - halfKnee) {
        return 0.0;
    } else if (inputLevelDb > threshold + halfKnee) {
        return threshold + (inputLevelDb - threshold) / ratio - inputLevelDb;
    } else {
        /* 拐点区域内二次过渡 */
        double x = inputLevelDb - threshold + halfKnee;
        double gain = (1.0 / ratio - 1.0) * x * x / (2.0 * knee);
        return gain;
    }
}

/**
 * @brief 处理音频数据压缩
 *
 * 对每个采样点执行：包络跟随 -> 增益计算 -> 增益平滑 -> 应用衰减。
 * 包络跟随使用 Attack/Release 模型，增益平滑使用软拐点过渡。
 *
 * @param samples 输入采样数据
 * @return 压缩后的采样数据
 */
QVector<double> Compressor6::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    QVector<double> output(n);
    double peakReduction = 0.0;

    /* 将时间常数转换为采样级别的系数 */
    double attackCoeff = qExp(-1.0 / (m_attackMs * 44.1));  /* 假设44.1kHz */
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * 44.1));

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(samples[i]);

        /* 包络跟随 */
        if (absSample > m_envelope) {
            m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * absSample;
        } else {
            m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * absSample;
        }

        /* 转换为dB */
        double levelDb = 20.0 * qLn(qMax(m_envelope, 1e-10)) / qLn(10.0);

        /* 计算增益衰减 */
        double gainDb = computeGainReduction(levelDb, m_thresholdDb, m_ratio, m_kneeDb);
        m_gainReductionDb = gainDb;

        if (gainDb < peakReduction) {
            peakReduction = gainDb;
        }

        /* 应用增益 */
        double gainLin = qPow(10.0, gainDb / 20.0);
        output[i] = samples[i] * gainLin;
    }

    m_stats.peakReductionDb = peakReduction;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalCompressionRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCompressionRuns;

    emit compressionCompleted(n);
    return output;
}
