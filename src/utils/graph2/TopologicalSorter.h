/**
 * @file TopologicalSorter.h
 * @brief 拓扑排序器 — Kahn算法+环检测
 *
 * 功能: Kahn算法拓扑排序，环检测，并行层级提取，
 *       统计节点/边数/环检测次数/耗时。
 */
#ifndef TOPOLOGICALSORTER_H
#define TOPOLOGICALSORTER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>

class TopologicalSorter : public QObject {
    Q_OBJECT
public:
    /** 排序结果 */
    struct SortResult {
        QVector<int> order;        ///< 拓扑序列
        bool hasCycle = false;     ///< 是否存在环
        QVector<int> cycleNodes;   ///< 环中的节点
        QVector<QVector<int>> levels; ///< 按层级划分(并行拓扑)
    };

    /** 统计 */
    struct Stats {
        quint64 totalSorts = 0;
        quint64 totalCyclesDetected = 0;
        int     totalNodes = 0;
        int     totalEdges = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit TopologicalSorter(QObject* parent = nullptr);

    /** @brief 添加有向边 @param from 起点 @param to 终点 */
    void addEdge(int from, int to);

    /** @brief 添加节点 @param node 节点ID */
    void addNode(int node);

    /** @brief 执行拓扑排序 @return 排序结果 */
    SortResult sort();

    /** @brief 检测环 @return 存在环返回true */
    bool hasCycle();

    /** @brief 获取并行层级 @return 每层可并行的节点 */
    QVector<QVector<int>> computeLevels() const;

    /** @brief 清空图 */
    void clear();

    /** @brief 节点数 */
    int nodeCount() const { return m_nodes.size(); }

    /** @brief 边数 */
    int edgeCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sortCompleted(int nodeCount, bool hasCycle);
    void cycleDetected(const QVector<int>& cycleNodes);

private:
    QSet<int> m_nodes;
    QMap<int, QVector<int>> m_adj;      ///< 邻接表
    QMap<int, int> m_inDegree;          ///< 入度

    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // TOPOLOGICALSORTER_H
