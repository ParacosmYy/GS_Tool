/**
 * @file DataSegmentAnalyzer.h
 * @brief 数据分段分析引擎 — 自动识别数据流中的分段边界
 *
 * 功能: 支持4种分段策略(阈值/方差/趋势/自定义)，自动检测
 *       数据流中的分段点，统计每段特征(均值/方差/长度/趋势)。
 *
 * 协作: DataAggregator(数据流分析) / ChartWidget(分段可视化)
 */
#ifndef DATASEGMENTANALYZER_H
#define DATASEGMENTANALYZER_H

#include <QObject>
#include <QList>
#include <QVector>

/**
 * @brief 数据分段分析引擎 — 自动识别数据流中的分段边界
 */
class DataSegmentAnalyzer : public QObject {
    Q_OBJECT

public:
    /** @brief 分段策略 */
    enum class SegmentPolicy {
        Threshold,   ///< 阈值分段: 值超过阈值时产生新段
        Variance,    ///< 方差分段: 局部方差突变时产生新段
        Trend,       ///< 趋势分段: 趋势方向变化时产生新段
        Fixed        ///< 固定长度分段: 每N个点一段
    };
    Q_ENUM(SegmentPolicy)

    /** @brief 数据段 */
    struct Segment {
        int startIndex = 0;     ///< 起始索引
        int endIndex = 0;       ///< 结束索引(含)
        double mean = 0.0;      ///< 段均值
        double variance = 0.0;  ///< 段方差
        double slope = 0.0;     ///< 线性趋势斜率
        double minValue = 0.0;  ///< 段最小值
        double maxValue = 0.0;  ///< 段最大值
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalPointsAnalyzed = 0;///< 累计分析点数
        quint64 totalSegmentsFound = 0; ///< 累计发现段数
        double  averageSegmentLength = 0.0; ///< 平均段长度
        int     peakSegmentLength = 0;  ///< 峰值段长度
        int     shortestSegmentLength = 0; ///< 最短段长度
    };

    explicit DataSegmentAnalyzer(QObject* parent = nullptr);

    /** @brief 设置分段策略 @param policy 策略 */
    void setPolicy(SegmentPolicy policy);

    /** @brief 设置分段阈值(阈值/方差模式) @param threshold 阈值 */
    void setThreshold(double threshold);

    /** @brief 设置固定段长度(固定模式) @param length 点数 */
    void setFixedLength(int length);

    /** @brief 设置最小段长度(避免噪声) @param length 最小点数 */
    void setMinSegmentLength(int length);

    /** @brief 分析数据流，返回检测到的段 @param data 数据 @return 段列表 */
    QList<Segment> analyze(const QVector<double>& data);

    /** @brief 获取最近一次分析结果 @return 段列表 */
    QList<Segment> lastSegments() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分段完成 @param segments 检测到的段列表 */
    void segmentsDetected(const QList<Segment>& segments);

private:
    QList<Segment> analyzeThreshold(const QVector<double>& data);
    QList<Segment> analyzeVariance(const QVector<double>& data);
    QList<Segment> analyzeTrend(const QVector<double>& data);
    QList<Segment> analyzeFixed(const QVector<double>& data);

    Segment computeSegment(const QVector<double>& data, int start, int end) const;

    SegmentPolicy m_policy;     ///< 分段策略
    double m_threshold;         ///< 分段阈值
    int m_fixedLength;          ///< 固定段长度
    int m_minSegmentLength;     ///< 最小段长度
    QList<Segment> m_lastSegments; ///< 最近一次分析结果

    Stats m_stats;
};

#endif // DATASEGMENTANALYZER_H
