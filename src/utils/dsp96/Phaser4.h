#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 移相效果器(Phaser)
 *
 * 通过一组全通滤波器级联并调制其截止频率,产生梳状滤波
 * 相位干涉效果,适用于吉他音色处理与电子音乐空间感塑造。
 */
class Phaser4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Phaser4(QObject* parent = nullptr);

    /** @brief 设置LFO调制速率(Hz) */
    void setRate(double rate);

    /** @brief 设置调制深度 */
    void setDepth(double depth);

    /** @brief 设置反馈量(0~1) */
    void setFeedback(double feedback);

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
    double m_rate = 0.5;
    double m_depth = 0.5;
    double m_feedback = 0.5;
};
