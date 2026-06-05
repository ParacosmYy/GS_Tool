#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief IndependentSet4 - 最大独立集求解器
 *
 * 寻找图中互不相邻的最大顶点子集(MIS)，
 * 使用分支定界和启发式近似算法。
 */
class IndependentSet4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSetsComputed = 0;
        int totalVertices = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IndependentSet4(QObject* parent = nullptr);

    /** @brief 设置图并求解最大独立集 */
    QVector<int> solve(const QVector<QVector<int>>& adjacency);

    /** @brief 获取最大独立集大小 */
    int maxSize() const;

    /** @brief 检查给定顶点集合是否为独立集 */
    bool isIndependent(const QVector<int>& vertices,
                       const QVector<QVector<int>>& adjacency) const;

    /** @brief 使用贪心近似求解 */
    QVector<int> greedyApprox(const QVector<QVector<int>>& adjacency);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void setComputed(int setSize, int vertexCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_maxSize = 0;
    QVector<int> m_solution;
};
