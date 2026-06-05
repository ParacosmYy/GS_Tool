#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 噪声水平估计器
 *
 * 基于滑动窗口或统计方法估计信号中的噪声水平，
 * 支持多种估计策略(minima/percentile/mmse)。
 */
class NoiseEstimate4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        double avgNoiseLevel = 0.0;  ///< 平均噪声水平(dB)
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit NoiseEstimate4(QObject* parent = nullptr);

    /** @brief 设置分析窗口大小 */
    void setWindowSize(int size);
    /** @brief 设置估计方法(minima/percentile/mmse) */
    void setMethod(const QString& method);
    /** @brief 估计噪声水平 */
    double estimate(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 估计完成，返回噪声水平(dB) */
    void estimated(double noiseLevel);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_windowSize = 100;
    QString m_method = "minima";
};
