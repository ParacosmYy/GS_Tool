/**
 * @file Autocorrelation.h
 * @brief 快速自相关分析引擎 — 基于FFT的高效自相关计算
 *
 * 功能: 利用FFT实现O(N log N)快速自相关计算，支持偏差校正、
 *       周期检测、基频估计。适用于串口数据周期性分析、
 *       信号重复模式检测、通信协议定时恢复。
 *
 * 协作: SpectrumAnalyzer(频谱) / PeakDetector(周期峰值)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 快速自相关分析引擎 — FFT加速 + 偏差校正
 */
class Autocorrelation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalComputations = 0;             ///< 累计自相关计算次数
        int totalSamplesProcessed = 0;         ///< 累计处理采样数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /** @brief 偏差校正模式 */
    enum class BiasMode {
        None,           ///< 无校正(原始)
        Unbiased,       ///< 无偏校正(除以N-lag)
        Normalized      ///< 归一化(零滞后=1.0)
    };
    Q_ENUM(BiasMode)

    /** @brief 周期检测结果 */
    struct PeriodResult {
        double period = 0.0;       ///< 检测到的周期(样本数)
        double confidence = 0.0;   ///< 置信度[0,1]
        double frequency = 0.0;    ///< 对应频率(Hz)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Autocorrelation(QObject* parent = nullptr);

    /**
     * @brief 设置采样率
     * @param rate 采样率(Hz)
     */
    void setSampleRate(double rate);

    /**
     * @brief 设置偏差校正模式
     * @param mode 校正模式
     */
    void setBiasMode(BiasMode mode);

    /**
     * @brief 计算完整自相关函数(基于FFT)
     * @param data 输入信号
     * @return 自相关序列(滞后0 ~ N-1)
     */
    QVector<double> compute(const QVector<double>& data);

    /**
     * @brief 计算指定滞后范围的自相关
     * @param data 输入信号
     * @param maxLag 最大滞后
     * @return 自相关序列(滞后0 ~ maxLag)
     */
    QVector<double> computeUpToLag(const QVector<double>& data, int maxLag);

    /**
     * @brief 检测信号中的主周期
     * @param data 输入信号
     * @param minPeriod 最小周期(样本数)
     * @param maxPeriod 最大周期(样本数)
     * @return 周期检测结果
     */
    PeriodResult detectPeriod(const QVector<double>& data,
                              int minPeriod = 2,
                              int maxPeriod = 0);

    /**
     * @brief 从自相关序列检测周期
     * @param autocorr 自相关序列
     * @param minLag 最小滞后
     * @param maxLag 最大滞后
     * @return 周期检测结果
     */
    PeriodResult detectPeriodFromAutocorr(const QVector<double>& autocorr,
                                          int minLag = 2,
                                          int maxLag = 0);

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
     * @brief 自相关计算完成
     * @param sampleCount 采样数
     * @param maxLag 最大滞后
     */
    void autocorrelationComputed(int sampleCount, int maxLag);

private:
    /**
     * @brief 基2 FFT(原地)
     * @param real 实部数组
     * @param imag 虚部数组
     */
    void fft(QVector<double>& real, QVector<double>& imag) const;

    /**
     * @brief 补零到2的幂
     * @param n 当前大小
     * @return >= n的最小2的幂
     */
    static int nextPowerOf2(int n);

    double m_sampleRate;                ///< 采样率
    BiasMode m_biasMode;                ///< 偏差校正模式

    Stats m_stats;
    double m_timeSum = 0.0;             ///< 处理时间累加器
};
