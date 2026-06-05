#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 多频段动态压缩器
 *
 * 将音频信号划分为多个频段,对每个频段独立施加动态压缩,
 * 实现精细的频谱动态控制,适用于母带处理与广播限制。
 */
class MultibandComp3 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit MultibandComp3(QObject* parent = nullptr);

    /** @brief 设置频段数量 */
    void setBandCount(int count);

    /** @brief 设置指定频段的压缩阈值 */
    void setThreshold(int band, double threshold);

    /** @brief 处理音频采样数据 */
    void process(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 处理完成信号 */
    void processingCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_bandCount = 4;
};
