#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 移相效果器(Phaser)
 *
 * 通过级联全通滤波器产生频率响应的梳状陷波，
 * 配合LFO调制实现经典移相/相位扫描音效。
 */
class Phaser5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Phaser5(QObject* parent = nullptr);

    /** @brief 设置LFO调制速率(Hz) */
    void setRate(double rate);

    /** @brief 设置调制深度 */
    void setDepth(double depth);

    /** @brief 设置反馈增益 */
    void setFeedback(double feedback);

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
    double m_rate = 0.5;
    double m_depth = 0.7;
    double m_feedback = 0.5;
};
