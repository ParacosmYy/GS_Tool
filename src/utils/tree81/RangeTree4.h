#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief RangeTree4 - 多维范围树
 *
 * 支持O(log^d n + k)范围查询的多维空间数据结构，
 * 使用分层二叉搜索树实现高维正交范围搜索。
 */
class RangeTree4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsInserted = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RangeTree4(QObject* parent = nullptr);

    /** @brief 设置维度数 */
    void setDimensions(int dimensions);

    /** @brief 批量构建范围树 */
    void build(const QVector<QVector<double>>& points);

    /** @brief 正交范围查询: 返回区域内所有点 */
    QVector<QPair<QVector<double>, int>> queryRange(
        const QVector<double>& lower, const QVector<double>& upper) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void queryCompleted(int resultCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimensions = 2;
    struct RTNode;
    RTNode* m_root = nullptr;
};
