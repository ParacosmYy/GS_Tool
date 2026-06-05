#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态均衡器
 *
 * 根据输入信号电平自动调节指定频段的增益，
 * 结合了参数均衡和动态处理的特性。
 */
class DynamicEQ6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit DynamicEQ6(QObject* parent = nullptr);

    /** @brief 设置中心频率(Hz) */
    void setFreq(double freq);

    /** @brief 设置增益(dB) */
    void setGain(double gain);

    /** @brief 设置Q值(带宽) */
    void setQ(double q);

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
    double m_freq = 1000.0;
    double m_gain = 0.0;
    double m_q = 1.0;
};
