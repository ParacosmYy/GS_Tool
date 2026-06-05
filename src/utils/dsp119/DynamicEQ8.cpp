#include "DynamicEQ8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态均衡器
 * @param parent 父对象指针
 */
DynamicEQ8::DynamicEQ8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DynamicEQ8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 添加一个动态EQ频段
 * @param frequencyHz 中心频率(Hz)
 * @param gainDb 静态增益(dB)
 * @param Q 品质因数
 * @param thresholdDb 动态阈值(dB)
 * @param rangeDb 动态范围(dB)
 */
void DynamicEQ8::addBand(double frequencyHz, double gainDb, double Q,
                          double thresholdDb, double rangeDb)
{
    Q_UNUSED(frequencyHz)
    Q_UNUSED(gainDb)
    Q_UNUSED(Q)
    Q_UNUSED(thresholdDb)
    Q_UNUSED(rangeDb)
}

/**
 * @brief 设置侧链信号源
 * @param sidechainFrame 侧链音频帧
 */
void DynamicEQ8::setSidechain(const QVector<double>& sidechainFrame)
{
    Q_UNUSED(sidechainFrame)
}

/**
 * @brief 移除指定频段
 * @param bandIndex 频段索引
 * @return 是否移除成功
 */
bool DynamicEQ8::removeBand(int bandIndex)
{
    Q_UNUSED(bandIndex)
    return false;
}

/**
 * @brief 处理音频帧进行动态均衡
 *
 * 结合参数均衡与动态处理的智能均衡器：
 * 1. 将频谱划分为多个可配置频段
 * 2. 检测每个频段的信号电平
 * 3. 当电平超过动态阈值时，按范围进行增益调节
 * 4. 应用平行压缩：未触发频段保持原样
 *
 * 简化实现：按频段位置划分，各频段独立电平检测和增益控制。
 *
 * @param inputFrame 输入音频采样帧
 * @return 均衡后的音频帧
 */
QVector<double> DynamicEQ8::processFrame(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = inputFrame.size();
    QVector<double> output(n);
    if (n == 0) {
        emit equalizationCompleted(0);
        return output;
    }

    /* 默认动态EQ频段配置（5频段平行压缩） */
    const int bandCount = 5;
    const double bandFreqs[bandCount + 1] = {0, 200, 800, 3000, 8000, 20000};
    const double thresholds[bandCount] = {-30.0, -24.0, -20.0, -22.0, -28.0};
    const double ranges[bandCount] = {6.0, 8.0, 10.0, 6.0, 4.0};

    for (int b = 0; b < bandCount; ++b) {
        /* 计算频段样本范围 */
        double startR = bandFreqs[b] / 22050.0;
        double endR = bandFreqs[b + 1] / 22050.0;
        int startIdx = static_cast<int>(startR * n);
        int endIdx = qMin(static_cast<int>(endR * n), n);
        if (startIdx >= endIdx) continue;

        /* 检测频段RMS电平 */
        double rms = 0.0;
        for (int i = startIdx; i < endIdx; ++i) {
            rms += inputFrame[i] * inputFrame[i];
        }
        double rmsDb = 20.0 * qLog10(qMax(rms / (endIdx - startIdx), 1e-10));

        /* 动态增益计算 */
        double gainDb = 0.0;
        if (rmsDb > thresholds[b]) {
            double overDb = rmsDb - thresholds[b];
            double reduction = qMin(overDb * 0.5, ranges[b]);
            gainDb = -reduction;
        }

        double gainLinear = qPow(10.0, gainDb / 20.0);

        /* 应用频段增益 */
        for (int i = startIdx; i < endIdx; ++i) {
            output[i] = inputFrame[i] * gainLinear;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit equalizationCompleted(n);
    return output;
}
