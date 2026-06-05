/**
 * @file StronglyConnected2.h
 * @brief 强连通分量 — Tarjan迭代/Kosaraju BFS/缩图DAG
 *
 * 功能: 实现Tarjan迭代算法和Kosaraju BFS算法求解有向图
 *       强连通分量，支持构建分量DAG(缩图)和拓扑排序。
 *
 * 协作: CycleDetector(环检测) / StateTracker(状态分析)
 */
#ifndef STRONGLYCONNECTED2_H
#define STRONGLYCONNECTED2_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>
#include <QPair>
#include <QSet>

/**
 * @brief 强连通分量分析器
 */
class StronglyConnected2 : public QObject {
    Q_OBJECT

public:
    /** @brief 算法选择 */
    enum class Algorithm {
        TarjanIterative,    ///< Tarjan迭代实现
        KosarajuBFS         ///< Kosaraju BFS实现
    };
    Q_ENUM(Algorithm)

    /** @brief 分量信息 */
    struct Component {
        int id = -1;                 ///< 分量ID
        QList<int> vertices;         ///< 分量内顶点列表
        bool isCyclic = false;       ///< 是否包含环
        int inDegree = 0;            ///< 缩图中的入度
        int outDegree = 0;           ///< 缩图中的出度
    };

    /** @brief DAG边(缩图) */
    struct DagEdge {
        int from = -1;               ///< 源分量ID
        int to = -1;                 ///< 目标分量ID
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalComputations = 0;       ///< 累计计算次数
        quint64 totalVerticesProcessed = 0;  ///< 累计处理顶点数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
        int     maxComponentsFound = 0;      ///< 单次最大分量数
    };

    explicit StronglyConnected2(QObject* parent = nullptr);

    /** @brief 设置有向图邻接表 @param adjacency 邻接表(顶点->邻居列表) */
    void setGraph(const QMap<int, QList<int>>& adjacency);

    /** @brief 计算强连通分量 @param algorithm 算法 @return 每个顶点的分量ID */
    QMap<int, int> compute(Algorithm algorithm = Algorithm::TarjanIterative);

    /** @brief 获取分量详情 @return 分量列表 */
    QList<Component> components() const;

    /** @brief 构建缩图DAG @return DAG边列表 */
    QList<DagEdge> buildCondensationDag();

    /** @brief 缩图拓扑排序 @return 分量ID列表(拓扑序) */
    QList<int> topologicalOrder() const;

    /** @brief 判断两顶点是否在同一分量 @param u 顶点u @param v 顶点v @return 是否同分量 */
    bool sameComponent(int u, int v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param numComponents 分量数 @param numEdges DAG边数 */
    void computationComplete(int numComponents, int numEdges);

private:
    QMap<int, int> tarjanIterative();
    QMap<int, int> kosarajuBFS();
    QMap<int, QList<int>> transposeGraph() const;

    QMap<int, QList<int>> m_adjacency;     ///< 邻接表
    QMap<int, int> m_vertexToComponent;    ///< 顶点->分量ID
    QList<Component> m_components;         ///< 分量详情
    QList<DagEdge> m_dagEdges;             ///< DAG边

    Stats m_stats;
    double m_timeSum = 0.0;                ///< 处理时间累加器
};

#endif // STRONGLYCONNECTED2_H
