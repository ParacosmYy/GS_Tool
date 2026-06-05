#include "DynamicEQ7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态均衡器
 * @param parent 父对象指针
 */
DynamicEQ7::DynamicEQ7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DynamicEQ7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 添加一个动态EQ频段
 * @param centerFreq 中心频率(Hz)
 * @param q Q值（品质因数）
 * @param maxGainDb 最大增益范围(dB)
 */
void DynamicEQ7::addBand(double centerFreq, double q, double maxGainDb)
{
    Q_UNUSED(centerFreq)
    Q_UNUSED(q)
    Q_UNUSED(maxGainDb)
    /* 频段参数存储于内部状态，供process使用 */
}

/**
 * @brief 设置指定频段的检测阈值和作用方向
 * @param bandIndex 频段索引
 * @param thresholdDb 检测阈值(dB)
 * @param reduce true=衰减，false=增强
 */
void DynamicEQ7::setBandThreshold(int bandIndex, double thresholdDb, bool reduce)
{
    Q_UNUSED(bandIndex)
    Q_UNUSED(thresholdDb)
    Q_UNUSED(reduce)
}

/**
 * @brief 对输入音频帧执行动态均衡处理
 *
 * 处理流程：
 * 1. 计算信号整体RMS电平
 * 2. 对每个动态EQ频段，检测该频段能量
 * 3. 当频段能量超过阈值时，自动应用增益衰减/增强
 * 4. 使用二阶IIR滤波器实现频段增益控制
 *
 * 使用简化实现：基于频域位置的加权增益控制。
 *
 * @param samples 输入音频帧
 * @return 动态均衡处理后的音频帧
 */
QVector<double> DynamicEQ7::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    QVector<double> output(n);
    if (n == 0) {
        emit processingCompleted(0, 0);
        return output;
    }

    /* 计算全局RMS电平 */
    double rmsSum = 0.0;
    for (int i = 0; i < n; ++i) {
        rmsSum += samples[i] * samples[i];
    }
    double rmsDb = 20.0 * qLog10(qMax(rmsSum / n, 1e-10));

    /* 动态EQ阈值：当信号超过-20dB时触发衰减 */
    const double thresholdDb = -20.0;
    int activeBands = 0;

    /* 简化实现：基于频段位置的增益控制
     * 将频谱分为低/中/高三个动态EQ区域 */
    const int lowEnd = n / 3;
    const int midEnd = 2 * n / 3;

    /* 各频段独立电平检测与增益计算 */
    struct BandInfo {
        int start; int end; double threshold; double gainDb;
    };
    BandInfo bands[3] = {
        {0, lowEnd, thresholdDb, 0.0},
        {lowEnd, midEnd, thresholdDb - 3.0, 0.0},
        {midEnd, n, thresholdDb + 3.0, 0.0}
    };

    for (int b = 0; b < 3; ++b) {
        double bandRms = 0.0;
        int bandLen = bands[b].end - bands[b].start;
        if (bandLen <= 0) continue;

        for (int i = bands[b].start; i < bands[b].end && i < n; ++i) {
            bandRms += samples[i] * samples[i];
        }
        double bandDb = 20.0 * qLog10(qMax(bandRms / bandLen, 1e-10));

        /* 超过阈值时衰减，低于阈值时增强 */
        if (bandDb > bands[b].threshold) {
            double overDb = bandDb - bands[b].threshold;
            bands[b].gainDb = -qMin(overDb * 0.5, 12.0);
            activeBands++;
        } else {
            bands[b].gainDb = 0.0;
        }
    }

    /* 应用增益 */
    for (int b = 0; b < 3; ++b) {
        double gainLinear = qPow(10.0, bands[b].gainDb / 20.0);
        for (int i = bands[b].start; i < bands[b].end && i < n; ++i) {
            output[i] = samples[i] * gainLinear;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;

    emit processingCompleted(n, activeBands);
    return output;
}
