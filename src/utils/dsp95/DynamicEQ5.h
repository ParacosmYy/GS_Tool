#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 动态均衡器
 *
 * 结合参数均衡与动态处理的智能均衡器,根据信号电平自动
 * 调节增益,实现共振峰控制、嘶声消除与自适应频谱塑形。
 */
class DynamicEQ5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit DynamicEQ5(QObject* parent = nullptr);

    /** @brief 设置中心频率(Hz) */
    void setFreq(double freq);

    /** @brief 设置增益(dB) */
    void setGain(double gain);

    /** @brief 设置Q值(带宽) */
    void setQ(double q);

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
    double m_freq = 1000.0;
    double m_gain = 0.0;
    double m_q = 1.0;
};
