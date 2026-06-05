#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 噪声估计工具类
 *
 * 基于滑动窗口估计信号中的噪声水平，
 * 用于信号去噪前的噪声基线评估。
 */
class NoiseEstimate3 : public QObject {
    Q_OBJECT
public:
    /// 估计统计信息
    struct Stats {
        int totalEstimations = 0;   ///< 总估计次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit NoiseEstimate3(QObject* parent = nullptr);

    /** @brief 设置滑动窗口大小(采样点数) */
    void setWindowSize(int size);

    /** @brief 对输入信号帧估计噪声水平 */
    double estimate(const QVector<double>& frame);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 估计完成信号，返回噪声估计值 */
    void estimated(double noiseLevel);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_windowSize = 256;
};
