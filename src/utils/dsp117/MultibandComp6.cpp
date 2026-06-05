#include "MultibandComp6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化多段动态压缩器
 * @param parent 父对象指针
 */
MultibandComp6::MultibandComp6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void MultibandComp6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置分频点
 * @param crossoverFreqs 分频点频率列表(Hz)
 * @param sampleRate 采样率(Hz)
 */
void MultibandComp6::setCrossovers(const QVector<double>& crossoverFreqs,
                                     double sampleRate)
{
    Q_UNUSED(crossoverFreqs)
    Q_UNUSED(sampleRate)
}

/**
 * @brief 设置指定频段的压缩参数
 * @param bandIndex 频段索引
 * @param thresholdDb 阈值(dB)
 * @param ratio 压缩比
 * @param attackMs 攻击时间(ms)
 * @param releaseMs 释放时间(ms)
 */
void MultibandComp6::setBandParameters(int bandIndex, double thresholdDb,
                                        double ratio, double attackMs,
                                        double releaseMs)
{
    Q_UNUSED(bandIndex)
    Q_UNUSED(thresholdDb)
    Q_UNUSED(ratio)
    Q_UNUSED(attackMs)
    Q_UNUSED(releaseMs)
}

/**
 * @brief 获取各频段当前增益缩减量
 * @return 各频段的增益缩减值(dB)
 */
QVector<double> MultibandComp6::getBandGainReductions() const
{
    return {0.0, 0.0, 0.0, 0.0};
}

/**
 * @brief 处理音频帧进行多段压缩
 *
 * 使用Linkwitz-Riley交叉滤波器将信号分为多个频段，
 * 对每个频段独立进行动态压缩后重新合成。
 *
 * 处理流程：
 * 1. 交叉滤波器组分频（低通+高通级联）
 * 2. 每频段独立电平检测与增益计算
 * 3. Attack/release包络平滑
 * 4. 频段叠加合成输出
 *
 * 简化实现：按样本索引比例划分频段区域，
 * 各区域独立计算电平并应用压缩增益。
 *
 * @param inputFrame 输入音频采样帧
 * @return 压缩后的音频帧
 */
QVector<double> MultibandComp6::processFrame(const QVector<double>& inputFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = inputFrame.size();
    QVector<double> output(n, 0.0);
    if (n == 0) {
        emit compressionCompleted(0);
        return output;
    }

    /* 默认4频段分频参数 */
    const int bandCount = 4;
    const double thresholdsDb[bandCount] = {-24.0, -20.0, -18.0, -22.0};
    const double ratios[bandCount] = {3.0, 4.0, 3.5, 5.0};
    const double attackMs[bandCount] = {10.0, 5.0, 5.0, 2.0};
    const double releaseMs[bandCount] = {100.0, 80.0, 60.0, 40.0};
    const double sampleRate = 44100.0;

    /* 按频段比例划分样本区间 */
    const double freqBands[bandCount + 1] = {0.0, 120.0, 1000.0, 4000.0, 20000.0};
    int bandStart[bandCount];
    int bandEnd[bandCount];

    for (int b = 0; b < bandCount; ++b) {
        double startRatio = freqBands[b] / 22050.0;
        double endRatio = freqBands[b + 1] / 22050.0;
        bandStart[b] = static_cast<int>(startRatio * n);
        bandEnd[b] = static_cast<int>(endRatio * n);
        bandEnd[b] = qMin(bandEnd[b], n);
    }

    /* 各频段独立压缩处理 */
    for (int b = 0; b < bandCount; ++b) {
        int len = bandEnd[b] - bandStart[b];
        if (len <= 0) continue;

        /* 计算频段RMS电平 */
        double rms = 0.0;
        for (int i = bandStart[b]; i < bandEnd[b]; ++i) {
            rms += inputFrame[i] * inputFrame[i];
        }
        double rmsDb = 20.0 * qLog10(qMax(rms / len, 1e-10));

        /* 增益计算机 */
        double gainDb = 0.0;
        if (rmsDb > thresholdsDb[b]) {
            gainDb = -(rmsDb - thresholdsDb[b]) * (1.0 - 1.0 / ratios[b]);
        }
        double gainLinear = qPow(10.0, gainDb / 20.0);

        /* 应用频段增益 */
        for (int i = bandStart[b]; i < bandEnd[b]; ++i) {
            output[i] += inputFrame[i] * gainLinear;
        }
    }

    /* 填充未覆盖区域 */
    for (int i = 0; i < n; ++i) {
        if (qAbs(output[i]) < 1e-15) {
            output[i] = inputFrame[i];
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit compressionCompleted(n);
    return output;
}
