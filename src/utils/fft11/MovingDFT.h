/**
 * @file MovingDFT.h
 * @brief 滑动DFT引擎 — O(1)逐样本更新的实时频谱分析
 *
 * 功能: 利用滑动DFT算法实现每输入一个样本O(1)时间更新频谱，
 *       适用于实时串口数据的连续频谱监测。
 *       支持多频率bin并行追踪、幅度/相位提取、泄漏抑制。
 *
 * 协作: SpectrumAnalyzer(频谱显示) / DataTrigger(频率触发)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QList>
#include <complex>

/**
 * @brief 滑动DFT引擎 — 逐样本O(1)实时频谱更新
 */
class MovingDFT : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSamplesProcessed = 0;      ///< 累计处理采样数
        int totalResets = 0;                ///< 累计重置次数(数值漂移校正)
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    /** @brief 频率bin结果 */
    struct BinResult {
        double frequency = 0.0;     ///< 频率(Hz)
        double magnitude = 0.0;     ///< 幅度
        double phase = 0.0;         ///< 相位(rad)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MovingDFT(QObject* parent = nullptr);

    /**
     * @brief 初始化滑动DFT
     * @param sampleRate 采样率(Hz)
     * @param binCount 频率bin数量
     */
    void initialize(double sampleRate, int binCount);

    /**
     * @brief 输入单个采样值，O(1)更新所有频率bin
     * @param sample 新采样值
     */
    void pushSample(double sample);

    /**
     * @brief 批量输入采样值
     * @param samples 采样数组
     */
    void pushSamples(const QVector<double>& samples);

    /**
     * @brief 获取当前所有频率bin的结果
     * @return 频率bin结果列表
     */
    QList<BinResult> currentSpectrum() const;

    /**
     * @brief 获取指定频率bin的结果
     * @param index bin索引
     * @return bin结果
     */
    BinResult binAt(int index) const;

    /**
     * @brief 获取频率bin数量
     * @return bin数量
     */
    int binCount() const;

    /**
     * @brief 重置所有bin状态(消除数值漂移)
     */
    void resetBins();

    /**
     * @brief 获取统计信息
     * @return 统计引用
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 频谱更新信号
     * @param sampleIndex 累计采样索引
     * @param binCount bin数量
     */
    void spectrumUpdated(quint64 sampleIndex, int binCount);

private:
    /**
     * @brief 计算旋转因子 e^(-j*2*pi*k/N)
     * @param k bin索引
     * @return 旋转因子(复数)
     */
    std::complex<double> twiddleFactor(int k) const;

    double m_sampleRate;                        ///< 采样率
    int m_binCount;                             ///< 频率bin数量
    quint64 m_sampleIndex;                      ///< 累计采样索引
    double m_prevSample;                        ///< 上一个采样值
    QVector<std::complex<double>> m_bins;       ///< 频率bin状态
    int m_resetInterval;                        ///< 重置间隔(防止漂移)
    int m_samplesSinceReset;                    ///< 距上次重置的采样数

    Stats m_stats;
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};
