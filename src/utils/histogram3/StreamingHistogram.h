/**
 * @file StreamingHistogram.h
 * @brief 流式直方图 — 固定分箱的增量更新
 *
 * 功能: 流式(在线)直方图，支持增量更新而不需重排数据，
 *       统计更新次数/数据量/耗时。
 */
#ifndef STREAMINGHISTOGRAM_H
#define STREAMINGHISTOGRAM_H

#include <QObject>
#include <QVector>

class StreamingHistogram : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalDataPoints = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit StreamingHistogram(int nBins = 50, double minVal = 0.0,
                                 double maxVal = 1.0,
                                 QObject* parent = nullptr);

    /** @brief 增量更新 @param value 新值 */
    void update(double value);

    /** @brief 批量更新 @param values 值序列 */
    void updateBatch(const QVector<double>& values);

    /** @brief 合并另一个流式直方图 @param other 另一个直方图 */
    void merge(const StreamingHistogram& other);

    /** @brief 获取分箱计数 @return 计数数组 */
    QVector<int> binCounts() const;

    /** @brief 获取分箱边界 @return 边界数组 */
    QVector<double> binEdges() const;

    /** @brief 总数据点数 */
    int totalPoints() const { return m_totalPoints; }

    /** @brief 估计百分位 @param p 百分位(0~100) @return 估计值 */
    double estimatePercentile(double p) const;

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void binUpdated(int binIndex, int newCount);

private:
    int m_nBins;
    double m_minVal;
    double m_maxVal;
    double m_binWidth;
    QVector<int> m_counts;
    int m_totalPoints;
    Stats m_stats;
    double m_timeSum;
};

#endif // STREAMINGHISTOGRAM_H
