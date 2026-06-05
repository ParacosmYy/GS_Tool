/**
 * @file SegmentTree.h
 * @brief 线段树 — 区间查询+单点更新
 *
 * 功能: 支持区间求和/最小值/最大值查询，单点更新，
 *       统计查询/更新次数/耗时。
 */
#ifndef SEGMENTTREE_H
#define SEGMENTTREE_H

#include <QObject>
#include <QVector>

class SegmentTree : public QObject {
    Q_OBJECT
public:
    /** 查询类型 */
    enum class QueryType {
        Sum,   ///< 区间和
        Min,   ///< 区间最小
        Max    ///< 区间最大
    };
    Q_ENUM(QueryType)

    /** 统计 */
    struct Stats {
        quint64 totalQueries = 0;
        quint64 totalUpdates = 0;
        int     treeSize = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree(QueryType type = QueryType::Sum,
                          QObject* parent = nullptr);

    /** @brief 从数组构建线段树 @param data 输入数据 */
    void build(const QVector<double>& data);

    /** @brief 区间查询 [left, right] @param left 左边界 @param right 右边界 @return 结果 */
    double query(int left, int right);

    /** @brief 单点更新 @param index 索引 @param value 新值 */
    void update(int index, double value);

    /** @brief 增量更新 @param index 索引 @param delta 增量 */
    void updateDelta(int index, double delta);

    /** @brief 原始数据大小 */
    int size() const { return m_size; }

    /** @brief 查询类型 */
    QueryType queryType() const { return m_type; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void queried(int left, int right, double result);
    void updated(int index, double value);

private:
    void buildImpl(int node, int start, int end);
    double queryImpl(int node, int start, int end, int l, int r);
    void updateImpl(int node, int start, int end, int idx, double val);

    double neutralElement() const;
    double combine(double a, double b) const;

    QueryType m_type;
    int m_size;
    QVector<double> m_data;
    QVector<double> m_tree;
    Stats m_stats;
    double m_timeSum;
};

#endif // SEGMENTTREE_H
