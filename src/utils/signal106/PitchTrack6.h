#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 音高追踪器
 *
 * 基于自相关/YIN算法实时追踪音频信号的基频(F0)轨迹，
 * 适用于语音处理和音乐音高分析。
 */
class PitchTrack6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalTracked = 0; double avgProcessingTimeMs = 0.0; };

    explicit PitchTrack6(QObject* parent = nullptr);

    /** @brief 设置最低追踪频率(Hz) */
    void setMinFreq(double freq);

    /** @brief 设置最高追踪频率(Hz) */
    void setMaxFreq(double freq);

    /** @brief 追踪音频帧的基频序列 */
    QVector<double> track(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 音高追踪完成信号 */
    void tracked(double fundamentalFreq);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_minFreq = 50.0;
    double m_maxFreq = 2000.0;
};
