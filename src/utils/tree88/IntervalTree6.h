#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 区间树(Interval Tree)
 *
 * 高效查询与指定区间重叠的所有区间。
 */
class IntervalTree6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalIntervalsInserted = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IntervalTree6(QObject* parent = nullptr);

    /** @brief 插入区间[lo, hi]附带数据 */
    void insert(int lo, int hi, const QVariant& data);

    /** @brief 查询与点重叠的所有区间 */
    QVector<QPair<QPair<int, int>, QVariant>> queryPoint(int point) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void queryCompleted(int point, int overlapCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct IntervalNode* m_root = nullptr;
};
