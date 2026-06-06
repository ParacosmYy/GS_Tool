/**
 * @file EulerTour.h
 * @brief 欧拉回路/路径(Hierholzer算法+Fleury回退) — Euler Tour/Circuit Finding via Hierholzer's Algorithm with Fleury's Fallback
 *
 * 功能: 在无向/有向图中寻找欧拉回路(Eulerian Circuit)或欧拉路径(Eulerian Path)，
 *       主算法Hierholzer O(V+E)，对割边检测回退至Fleury算法。
 *
 * 协作: StronglyConnected(强连通) / Bridges2(桥检测) / HamiltonianPath(哈密顿)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 欧拉回路/路径查找器
 */
class EulerTour : public QObject {
    Q_OBJECT

public:
    /** @brief 图类型 */
    enum GraphType {
        Undirected, ///< 无向图
        Directed    ///< 有向图
    };

    /** @brief 欧拉图判定结果 */
    enum EulerType {
        None,       ///< 非欧拉图
        Path,       ///< 存在欧拉路径
        Circuit     ///< 存在欧拉回路
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
        int lastVertexCount = 0;         ///< 最近顶点数
        int lastEdgeCount = 0;           ///< 最近边数
    };

    explicit EulerTour(QObject* parent = nullptr);
    ~EulerTour() override;

    /** @brief 设置图类型 */
    void setGraphType(GraphType type);

    /**
     * @brief 构建图(添加边)
     * @param from 起点
     * @param to 终点
     */
    void addEdge(int from, int to);

    /**
     * @brief 判定欧拉类型
     * @param vertexCount 顶点数
     * @return 欧拉类型
     */
    EulerType checkEuler(int vertexCount) const;

    /**
     * @brief 使用Hierholzer算法查找欧拉回路/路径
     * @param vertexCount 顶点数
     * @return 顶点序列(空表示不存在)
     */
    QVector<int> findEulerTour(int vertexCount);

    /**
     * @brief 使用Fleury算法查找(带回退)
     * @param vertexCount 顶点数
     * @return 顶点序列
     */
    QVector<int> findEulerTourFleury(int vertexCount);

    /** @brief 清空图 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 查找完成 @param edgeCount 边数 @param type 结果类型 */
    void tourFound(int edgeCount, int type);

private:
    /** @brief Hierholzer核心(邻接表版) */
    QVector<int> hierholzer(int vertexCount, int startVertex);

    /** @brief 寻找起始顶点 */
    int findStartVertex(int vertexCount) const;

    /** @brief 检查边是否为桥(用于Fleury) */
    bool isBridge(int vertexCount, int u, int v) const;

    /** @brief DFS计算连通分量大小 */
    int dfsCount(int v, QVector<bool>& visited,
                 const QVector<QVector<int>>& adj) const;

    GraphType m_graphType = Undirected;

    /* Edge list: (from, to) with implicit index */
    struct Edge {
        int to;
        int rev;   ///< 反向边索引
        bool used; ///< 是否已使用(Hierholzer标记)
    };
    QVector<QVector<Edge>> m_adj;

    /* Raw edges for Fleury */
    QVector<QPair<int, int>> m_rawEdges;

    Stats m_stats;
    double m_timeSum = 0.0;
};
