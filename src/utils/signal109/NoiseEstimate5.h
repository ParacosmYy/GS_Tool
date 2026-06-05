#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 噪声估计器实现 (版本5)
 *
 * 提供多种噪声功率估计方法，支持实时噪声底估计和SNR计算。
 */
class NoiseEstimate5 : public QObject {
    Q_OBJECT
public:
    /// 噪声估计方法
    enum Method { MinimumStat = 0, Percentile = 1, MovingAverage = 2 };

    /// 统计信息结构
    struct Stats {
        int totalEstimates = 0;         ///< 总估计次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double lastNoiseFloorDb = 0.0;  ///< 最近一次噪声底(dB)
    };

    explicit NoiseEstimate5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 从功率谱估计噪声底
     * @param powerSpectrum 功率谱密度
     * @param method 估计方法
     * @return 噪声底功率(dB)
     */
    double estimate(const QVector<double>& powerSpectrum, Method method = MinimumStat);

    /**
     * @brief 计算信噪比
     * @param signalPower 信号功率(dB)
     * @param noiseFloorDb 噪声底(dB)
     * @return 信噪比(dB)
     */
    double computeSNR(double signalPower, double noiseFloorDb) const;

    /**
     * @brief 实时更新噪声估计
     * @param newSpectrum 新的功率谱帧
     * @return 更新后的噪声底(dB)
     */
    double updateOnline(const QVector<double>& newSpectrum);

    /**
     * @brief 获取平滑后的噪声谱
     * @return 噪声功率谱估计
     */
    QVector<double> noiseSpectrum() const { return m_noiseSpectrum; }

signals:
    /// 估计完成信号
    void estimateCompleted(double noiseFloorDb);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_noiseSpectrum;
    double m_noiseFloorDb = -100.0;
};
