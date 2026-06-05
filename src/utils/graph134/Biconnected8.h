#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Biconnected8 - 双连通分量算法第8代实现
 *
 * 提供割点、桥及双连通分量的识别算法，
 * 支持DFS-based线性时间检测。
 */
class Biconnected8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolveOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit Biconnected8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 查找图中所有割点（关节点）
     * @param adjacencyList 图的邻接表
     * @return 割点索引集合
     */
    QVector<int> findArticulationPoints(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief 查找图中所有桥（割边）
     * @param adjacencyList 图的邻接表
     * @return 桥的端点对列表
     */
    QVector<QPair<int, int>> findBridges(const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief 计算双连通分量
     * @param adjacencyList 图的邻接表
     * @return 各双连通分量包含的边列表
     */
    QVector<QVector<QPair<int, int>>> biconnectedComponents(
        const QVector<QVector<int>>& adjacencyList);

    /**
     * @brief 检查图是否为双连通图
     * @param adjacencyList 图的邻接表
     * @return 是否双连通（无割点且连通）
     */
    bool isBiconnected(const QVector<QVector<int>>& adjacencyList) const;

signals:
    void solveCompleted(int componentCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
