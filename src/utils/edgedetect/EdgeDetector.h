/**
 * @file EdgeDetector.h
 * @brief 信号边沿检测器 — 上升/下降沿检测+消抖
 *
 * 功能: 检测信号上升沿、下降沿或双边沿，支持可配置消抖采样数，
 *       统计总采样数、上升沿与下降沿计数，输出边沿时刻索引。
 */
#ifndef EDGEDETECTOR_H
#define EDGEDETECTOR_H

#include <QObject>
#include <QVector>
#include <deque>

/**
 * @brief 信号边沿检测器(含消抖)
 */
class EdgeDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 边沿类型 */
    enum EdgeType {
        Rising  = 0x01,  ///< 仅上升沿
        Falling = 0x02,  ///< 仅下降沿
        Both    = 0x03   ///< 上升沿+下降沿
    };
    Q_ENUM(EdgeType)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSamples = 0;       ///< 累计处理采样数
        quint64 totalRisingEdges = 0;   ///< 累计检测到的上升沿数
        quint64 totalFallingEdges = 0;  ///< 累计检测到的下降沿数
    };

    /**
     * @brief 构造函数
     * @param type 检测的边沿类型
     * @param parent 父对象
     */
    explicit EdgeDetector(EdgeType type = Both, QObject* parent = nullptr);

    /**
     * @brief 处理一个采样值
     * @param value 当前采样值
     * @param threshold 阈值(高于此值为高电平)
     */
    void process(double value, double threshold);

    /**
     * @brief 设置消抖采样数(连续N个采样确认后才触发)
     * @param samples 消抖采样数(≥1)
     */
    void setDebounceSamples(int samples);

    /**
     * @brief 获取所有边沿发生的采样索引
     * @return 边沿时刻索引列表
     */
    QVector<int> getEdgeTimes() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 检测到上升沿 @param sampleIndex 上升沿发生的采样索引 */
    void risingEdge(int sampleIndex);
    /** @brief 检测到下降沿 @param sampleIndex 下降沿发生的采样索引 */
    void fallingEdge(int sampleIndex);

private:
    /**
     * @brief 检查消抖缓冲区是否全部为指定电平
     * @param level 目标电平(true=高, false=低)
     * @return 是否满足消抖条件
     */
    bool checkDebounce(bool level) const;

    EdgeType m_edgeType;               ///< 检测边沿类型
    int m_debounceSamples;             ///< 消抖采样数
    bool m_lastStableLevel;            ///< 上一次稳定电平
    int m_sampleIndex;                 ///< 当前采样索引
    std::deque<bool> m_debounceBuffer; ///< 消抖缓冲区
    QVector<int> m_risingEdgeTimes;    ///< 上升沿时刻列表
    QVector<int> m_fallingEdgeTimes;   ///< 下降沿时刻列表
    mutable Stats m_stats;             ///< 统计信息(mutable支持const方法)
};

#endif // EDGEDETECTOR_H
