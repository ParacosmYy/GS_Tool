#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 音高追踪器(Pitch Tracker)实现
 *
 * 基于自相关函数和YIN算法追踪信号的基频(F0)随时间变化轨迹，
 * 支持有声/无声判定和倍频消除，适用于语音分析、乐器调音和旋律提取。
 */
class PitchTrack7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalTracked = 0; double avgProcessingTimeMs = 0.0; };

    explicit PitchTrack7(QObject* parent = nullptr);

    /** @brief 设置搜索的最小和最大基频范围(Hz) */
    void setFrequencyRange(double minHz, double maxHz);

    /** @brief 设置分析帧长和跳步大小(样本数) */
    void setFrameParams(int frameSize, int hopSize);

    /** @brief 对输入音频信号执行基频追踪，返回(时间帧, 频率Hz)序列 */
    QVector<QPair<int, double>> track(const QVector<double>& audio);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 追踪完成信号，返回检测到的有声帧数 */
    void trackingCompleted(int voicedFrameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_minFreq = 50.0;
    double m_maxFreq = 2000.0;
    int m_frameSize = 2048;
    int m_hopSize = 512;
};
