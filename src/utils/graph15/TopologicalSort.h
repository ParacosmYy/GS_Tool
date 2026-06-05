/**
 * @file TopologicalSort.h
 * @brief 拓扑排序 — Kahn算法与DFS双模式，含环检测
 *
 * 功能:
 *   - Kahn算法(BFS): 基于入度的迭代排序，自然检测环
 *   - DFS递归: 深度优先后序逆序排序
 *   - 环检测: 在排序过程中检测有向图中的环
 *   - 支持统计排序次数、顶点数、环检测次数
 *   - 提供详细的拓扑层级信息
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QPair>

/**
 * @class TopologicalSort
 * @brief 拓扑排序引擎 — Kahn与DFS双模式，含环检测
 *
 * 有向无环图(DAG)的拓扑排序。Kahn算法适合需要层级信息的场景，
 * DFS适合简单排序。两者均能检测环的存在。
 */
class TopologicalSort : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSorts = 0;            /**< 总排序次数 */
        int totalVertices = 0;         /**< 总处理顶点数 */
        int totalCyclesDetected = 0;   /**< 总检测到环的次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 边列表: (from, to) */
    using EdgeList = QVector<QPair<int, int>>;

    /** @brief 排序结果 */
    struct SortResult {
        QVector<int> order;           /**< 拓扑序 */
        bool hasCycle = false;        /**< 是否存在环 */
        QVector<int> cycleNodes;      /**< 环中涉及的节点(若有) */
        QVector<QVector<int>> levels; /**< 拓扑层级(Kahn模式) */
    };

    /** @brief 构造函数 */
    explicit TopologicalSort(QObject* parent = nullptr);

    /**
     * @brief Kahn算法拓扑排序(BFS)
     * @param numVertices 顶点数
     * @param edges 边列表
     * @return 排序结果(含层级信息)
     */
    SortResult sortKahn(int numVertices, const EdgeList& edges);

    /**
     * @brief DFS拓扑排序
     * @param numVertices 顶点数
     * @param edges 边列表
     * @return 排序结果(不含层级信息)
     */
    SortResult sortDFS(int numVertices, const EdgeList& edges);

    /**
     * @brief 检测图中是否有环
     * @param numVertices 顶点数
     * @param edges 边列表
     * @return 是否有环
     */
    bool hasCycle(int numVertices, const EdgeList& edges);

    /**
     * @brief 获取统计信息
     */
    Stats stats() const;

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /** @brief 排序完成信号 */
    void sortCompleted(int vertexCount, bool hasCycle);

private:
    /** @brief 构建邻接表 */
    QMap<int, QVector<int>> buildAdjList(int numVertices,
                                          const EdgeList& edges) const;

    /** @brief DFS环检测与拓扑序 */
    bool dfsVisit(int node, const QMap<int, QVector<int>>& adj,
                  QMap<int, int>& state, QVector<int>& order,
                  QVector<int>& cycleNodes) const;

    mutable Stats m_stats;       /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
};
