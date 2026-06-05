#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 多频段动态压缩器
 *
 * 将音频信号按频段分割后分别进行动态范围压缩，
 * 支持独立设置各频段的阈值、比率和启释放时间。
 */
class MultibandComp4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit MultibandComp4(QObject* parent = nullptr);

    /** @brief 设置频段数量 */
    void setBandCount(int count);

    /** @brief 设置指定频段的压缩阈值 */
    void setThreshold(int band, double threshold);

    /** @brief 处理音频采样数据 */
    void process(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号 */
    void processingCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_bandCount = 4;
};
