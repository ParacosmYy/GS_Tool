#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 多频段动态压缩器实现
 *
 * 将音频信号按频率划分为多个子带，对每个子带独立施加压缩/扩展处理，
 * 适用于广播、母带处理和多轨混音中的频谱平衡控制。
 */
class MultibandComp5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalProcessed = 0; double avgProcessingTimeMs = 0.0; };

    explicit MultibandComp5(QObject* parent = nullptr);

    /** @brief 设置频段分界点频率列表(Hz)，N个分界点产生N+1个频段 */
    void setCrossoverFrequencies(const QVector<double>& frequencies);

    /** @brief 设置指定频段的压缩比(>1压缩，<1扩展) */
    void setBandRatio(int bandIndex, double ratio);

    /** @brief 设置指定频段的起音时间(attack)和释放时间(release)，单位ms */
    void setBandTiming(int bandIndex, double attackMs, double releaseMs);

    /** @brief 对输入音频帧执行多频段压缩处理 */
    QVector<double> process(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回处理的帧数和激活频段数 */
    void processingCompleted(int frameCount, int activeBands);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<double> m_crossovers;
    QVector<double> m_ratios;
    QVector<QPair<double, double>> m_timings;
};
