#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief KDTree5 - KD树多维空间索引
 *
 * 支持批量构建和动态插入的KD树，提供k近邻搜索、
 * 范围搜索和最近邻查询，适用于高维数据索引。
 */
class KDTree5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsInserted = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KDTree5(QObject* parent = nullptr);

    /** @brief 批量构建KD树(O(n log n)) */
    void build(const QVector<QVector<double>>& points);

    /** @brief 插入单个点 */
    void insert(const QVector<double>& point, int data = 0);

    /** @brief k近邻搜索 */
    QVector<QPair<QVector<double>, int>> kNearest(const QVector<double>& query, int k) const;

    /** @brief 范围搜索: 矩形范围内所有点 */
    QVector<QPair<QVector<double>, int>> rangeSearch(
        const QVector<double>& lower, const QVector<double>& upper) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int pointCount, int dimensions);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct KDNode;
    KDNode* m_root = nullptr;
    int m_dimensions = 0;
};
