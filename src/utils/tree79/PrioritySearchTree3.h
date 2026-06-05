#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief PrioritySearchTree3 - 优先搜索树
 *
 * 结合二叉搜索树和优先队列的数据结构，支持
 * 三维范围查询(x范围+优先级阈值)，O(log n)查询。
 */
class PrioritySearchTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsInserted = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrioritySearchTree3(QObject* parent = nullptr);

    /** @brief 插入点(x坐标, y坐标/优先级, 关联数据) */
    void insert(double x, double y, int data = 0);

    /** @brief 查询x在[xMin,xMax]范围内且y小于yMax的所有点 */
    QVector<QPair<QPair<double,double>, int>> query(double xMin, double xMax, double yMax) const;

    /** @brief 获取树中点总数 */
    int count() const;

    /** @brief 清空树 */
    void clear();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void queryCompleted(int resultCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct PSTNode;
    PSTNode* m_root = nullptr;
};
