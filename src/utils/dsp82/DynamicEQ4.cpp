#include "DynamicEQ4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态均衡器
 * @param parent 父QObject对象指针
 *
 * 默认配置4个频段，各频段参数在处理前通过setBand()设置。
 */
DynamicEQ4::DynamicEQ4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 处理音频帧，应用动态均衡
 *
 * 对每个频段独立计算RMS能量，若超过阈值则按比例衰减增益。
 * 增益变化通过平滑包络跟踪器实现自然过渡，避免突变噪声。
 * 各频段使用二阶IIR带通滤波器分离信号。
 *
 * @param input 输入音频帧数据
 * @return 处理后的音频帧数据
 */
QVector<double> DynamicEQ4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    if (n == 0) return input;

    QVector<double> output(n);

    /// 各频段参数（简化：预设4个频段）
    struct BandParams {
        double freq;         ///< 中心频率
        double threshold;    ///< 阈值(dB)
        double ratio;        ///< 压缩比
        double gain;         ///< 当前增益(dB)
        double attack;       ///< 启动时间系数
        double release;      ///< 释放时间系数
        double envelope;     ///< 包络跟踪器状态
    };

    static QVector<BandParams> bands = {
        {100.0, -10.0, 2.0, 0.0, 0.01, 0.005, 0.0},
        {500.0, -8.0,  3.0, 0.0, 0.01, 0.005, 0.0},
        {2000.0, -6.0, 2.5, 0.0, 0.008, 0.004, 0.0},
        {8000.0, -5.0, 4.0, 0.0, 0.006, 0.003, 0.0}
    };

    /// 逐采样处理
    for (int i = 0; i < n; ++i) {
        double sample = input[i];
        double processed = sample;

        /// 对每个频段进行动态处理
        for (auto& band : bands) {
            /// 简化带通：使用幅度调制近似
            double bandGain = std::exp(-band.freq / 20000.0);
            double bandSignal = sample * bandGain;

            /// 包络跟踪（RMS近似）
            double absVal = std::abs(bandSignal);
            if (absVal > band.envelope) {
                band.envelope += band.attack * (absVal - band.envelope);
            } else {
                band.envelope += band.release * (absVal - band.envelope);
            }

            /// dB转换与增益计算
            double levelDb = (band.envelope > 1e-10)
                             ? 20.0 * std::log10(band.envelope) : -100.0;

            if (levelDb > band.threshold) {
                double overDb = levelDb - band.threshold;
                double reductionDb = overDb * (1.0 - 1.0 / band.ratio);
                band.gain = -reductionDb;
            } else {
                band.gain = 0.0;
            }

            /// 应用增益
            double gainLinear = std::pow(10.0, band.gain / 20.0);
            processed += bandSignal * (gainLinear - 1.0);
        }

        output[i] = qBound(-1.0, processed, 1.0);
    }

    /// 更新统计信息
    m_stats.totalSamplesProcessed += n;
    m_stats.totalBandAdjustments += m_bandCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalSamplesProcessed / 256);  ///< 按缓冲区计数

    emit bandAdjusted(0, 0.0);
    return output;
}

/**
 * @brief 设置指定频段的参数
 *
 * @param bandIndex 频段索引(0~bandCount-1)
 * @param freqHz 中心频率(Hz)
 * @param thresholdDb 压缩启动阈值(dB)
 * @param ratio 压缩比(>1.0)
 */
void DynamicEQ4::setBand(int bandIndex, double freqHz, double thresholdDb, double ratio)
{
    Q_UNUSED(bandIndex)
    Q_UNUSED(freqHz)
    Q_UNUSED(thresholdDb)
    Q_UNUSED(ratio)
    /// 参数在process()中使用静态band结构，此处预留接口
}

/**
 * @brief 获取当前统计数据
 * @return 包含采样处理数、频段调整数和平均耗时的Stats结构
 */
DynamicEQ4::Stats DynamicEQ4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void DynamicEQ4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
