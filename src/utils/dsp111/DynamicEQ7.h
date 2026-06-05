#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态均衡器实现
 *
 * 将均衡器的增益与信号电平关联，当指定频段电平超过阈值时自动
 * 衰减或增强，实现频率敏感的自动增益控制，适用于去齿音和共振抑制。
 */
class DynamicEQ7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit DynamicEQ7(QObject* parent = nullptr);

    /** @brief 添加一个动态EQ频段，指定中心频率(Hz)、Q值和增益范围(dB) */
    void addBand(double centerFreq, double q, double maxGainDb);

    /** @brief 设置指定频段的检测阈值(dB)和作用方向(衰减/增强) */
    void setBandThreshold(int bandIndex, double thresholdDb, bool reduce);

    /** @brief 对输入音频帧执行动态均衡处理 */
    QVector<double> process(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回帧数和激活频段数 */
    void processingCompleted(int frameCount, int activeBands);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
