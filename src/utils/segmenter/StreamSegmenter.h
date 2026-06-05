/**
 * @file StreamSegmenter.h
 * @brief 数据流分段器 — 将连续数据流按规则自动分段
 *
 * 功能: 支持固定窗口/阈值跳变/空闲间隔/自定义标记四种分段策略，
 *       统计段数/平均段长/最大最小段长等指标。
 */
#ifndef STREAMSEGMENTER_H
#define STREAMSEGMENTER_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @class StreamSegmenter
 * @brief 将连续数据流按不同策略自动分段，提取段特征
 */
class StreamSegmenter : public QObject {
    Q_OBJECT
public:
    /** 分段策略 */
    enum class Strategy {
        FixedWindow,    ///< 固定窗口大小
        ThresholdJump,  ///< 阈值跳变检测
        IdleGap,        ///< 空闲间隔检测
        CustomMarker    ///< 自定义标记分隔
    };

    /** 单个数据段 */
    struct Segment {
        int startIndex;         ///< 段起始索引
        int endIndex;           ///< 段结束索引(含)
        double mean;            ///< 段内均值
        double stddev;          ///< 段内标准差
        double min;             ///< 段内最小值
        double max;             ///< 段内最大值
    };

    /** 分段统计 */
    struct Stats {
        quint64 totalSegments = 0;      ///< 总段数
        double  avgSegmentLength = 0.0; ///< 平均段长
        int     maxSegmentLength = 0;   ///< 最大段长
        int     minSegmentLength = 0;   ///< 最小段长
        quint64 totalPointsProcessed = 0; ///< 已处理点数
        double  averageProcessingTimeMs = 0.0; ///< 平均处理耗时
    };

    explicit StreamSegmenter(QObject* parent = nullptr);

    /** 设置分段策略和参数 */
    void setStrategy(Strategy s);
    void setWindowSize(int size);
    void setJumpThreshold(double threshold);
    void setIdleGapThreshold(double gap);
    void setCustomMarker(double marker);

    /** 执行分段 */
    QList<Segment> segment(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void segmentFound(const Segment& seg);
    void segmentationComplete(int segmentCount);

private:
    QList<Segment> fixedWindow(const QVector<double>& data);
    QList<Segment> thresholdJump(const QVector<double>& data);
    QList<Segment> idleGap(const QVector<double>& data);
    QList<Segment> customMarker(const QVector<double>& data);
    Segment computeSegment(const QVector<double>& data, int start, int end);

    Strategy m_strategy;
    int m_windowSize;
    double m_jumpThreshold;
    double m_idleGapThreshold;
    double m_customMarker;
    Stats m_stats;
    double m_timeSum;
};

#endif // STREAMSEGMENTER_H
