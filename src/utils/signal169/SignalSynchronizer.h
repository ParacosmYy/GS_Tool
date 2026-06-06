/**
 * @file SignalSynchronizer.h
 * @brief 信号同步(互相关峰值检测+分数延迟估计) — Signal Synchronization via Cross-Correlation Peak Detection with Fractional Delay Estimation
 *
 * 功能: 通过互相关函数检测信号间的时延差，支持整数采样和分数采样精度，
 *       使用抛物线插值和频域相位估计实现亚采样级延迟对齐。
 *
 * 协作: CrossCorrelator(互相关) / FftEngine(FFT) / SignalResampler(重采样)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 信号同步器
 */
class SignalSynchronizer : public QObject {
    Q_OBJECT

public:
    /** @brief 同步结果 */
    struct SyncResult {
        double delaySamples = 0.0;  ///< 估计延迟(样本数，含分数部分)
        int integerDelay = 0;       ///< 整数延迟
        double fractionalDelay = 0.0;///< 分数延迟
        double correlationPeak = 0.0;///< 相关峰值
        double snrEstimate = 0.0;   ///< 信噪比估计(dB)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSyncs = 0;          ///< 累计同步次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
        int lastSignalLength = 0;        ///< 最近信号长度
    };

    explicit SignalSynchronizer(QObject* parent = nullptr);
    ~SignalSynchronizer() override;

    /** @brief 设置最大搜索延迟范围 */
    void setMaxDelay(int maxDelay);

    /**
     * @brief 时域互相关同步
     * @param reference 参考信号
     * @param signal 待同步信号
     * @return 同步结果
     */
    SyncResult synchronize(const QVector<double>& reference,
                           const QVector<double>& signal);

    /**
     * @brief 频域互相关同步(FFT加速)
     * @param reference 参考信号
     * @param signal 待同步信号
     * @return 同步结果
     */
    SyncResult synchronizeFFT(const QVector<double>& reference,
                              const QVector<double>& signal);

    /**
     * @brief 对齐信号(应用分数延迟)
     * @param signal 待对齐信号
     * @param delay 延迟(样本数，正=信号滞后)
     * @return 对齐后信号
     */
    QVector<double> alignSignal(const QVector<double>& signal, double delay);

    /** @brief 计算时域互相关 */
    QVector<double> crossCorrelate(const QVector<double>& a,
                                   const QVector<double>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 同步完成 @param delay 估计延迟 */
    void syncCompleted(double delay);

private:
    /** @brief 抛物线插值求分数峰值 */
    double parabolicInterp(double ym1, double y0, double yp1) const;

    /** @brief 执行FFT(就地) */
    void fft(QVector<double>& real, QVector<double>& imag);

    /** @brief 执行IFFT(就地) */
    void ifft(QVector<double>& real, QVector<double>& imag);

    /** @brief sinc插值内核 */
    static double sinc(double x);

    int m_maxDelay = 1024;

    Stats m_stats;
    double m_timeSum = 0.0;
};
