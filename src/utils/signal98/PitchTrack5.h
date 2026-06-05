#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 音高追踪器
 *
 * 基于自相关或YIN算法实时追踪信号的基频(F0)轨迹,
 * 适用于语音基频分析、乐器调音与旋律提取。
 */
class PitchTrack5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalTracked = 0; double avgProcessingTimeMs = 0.0; };

    explicit PitchTrack5(QObject* parent = nullptr);

    /** @brief 设置最小追踪频率(Hz) */
    void setMinFreq(double freq);

    /** @brief 设置最大追踪频率(Hz) */
    void setMaxFreq(double freq);

    /** @brief 对输入信号执行音高追踪 */
    void track(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 追踪完成信号,返回估计基频 */
    void tracked(double fundamentalFreq);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_minFreq = 50.0;
    double m_maxFreq = 2000.0;
};
