/**
 * @file SlidingDFT4.h
 * @brief 滑动DFT(递推更新+实时频谱计算) — Sliding DFT with Recursive Update for Real-Time Spectrum Computation
 *
 * 功能: 实现滑动DFT，通过递推公式逐样本更新频谱，无需完整FFT重算，
 *       支持多通道并行、窗函数补偿和频率Bin提取。
 *
 * 协作: DiscreteHartleyTransform(DHT) / FftEngine(FFT) / GoertzelFilter(单频检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QtContainerFwd>

/**
 * @brief 滑动DFT处理器
 */
class SlidingDFT4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamples = 0;         ///< 累计处理样本数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int transformSize = 0;            ///< DFT点数
        quint64 totalUpdates = 0;         ///< 累计更新次数
    };

    explicit SlidingDFT4(int N = 256, QObject *parent = nullptr);
    ~SlidingDFT4() override;

    /** @brief 设置DFT点数(须为2的幂) */
    void setTransformSize(int N);

    /**
     * @brief 推入一个新样本，递推更新所有频谱Bin
     * @param sample 输入采样值
     */
    void pushSample(double sample);

    /**
     * @brief 批量处理样本
     * @param samples 输入采样序列
     */
    void processBatch(const QVector<double>& samples);

    /** @brief 获取当前频谱幅度 */
    QVector<double> magnitudes() const;

    /** @brief 获取当前频谱相位 */
    QVector<double> phases() const;

    /** @brief 获取指定Bin的幅度 */
    double binMagnitude(int k) const;

    /** @brief 获取指定Bin的相位 */
    double binPhase(int k) const;

    /** @brief 重置所有状态 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 样本处理完成 @param idx 样本序号 */
    void sampleProcessed(quint64 idx);
    /** @brief 批量处理完成 @param count 样本数 */
    void batchCompleted(int count);

private:
    /** @brief 单个Bin的复数状态 */
    struct BinState {
        double real = 0.0;
        double imag = 0.0;
        double coeffReal = 0.0;   ///< e^{-j*2pi*k/N} 实部
        double coeffImag = 0.0;   ///< e^{-j*2pi*k/N} 虚部
    };

    /** @brief 初始化Bin系数 */
    void initBins();

    int m_N;
    QVector<BinState> m_bins;
    QVector<double> m_buffer;       ///< 环形缓冲区
    int m_bufIdx = 0;               ///< 缓冲区写入位置
    quint64 m_sampleCount = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
