/**
 * @file EulerianPath.h
 * @brief 欧拉路径/回路检测与Hierholzer算法
 *
 * 实现图论中的欧拉路径和欧拉回路相关算法:
 *   - 检测无向图/有向图的欧拉路径/回路存在性
 *   - Hierholzer算法求解欧拉路径/回路
 *   - 支持邻接表和边列表输入
 *   - Fleury算法作为备用(逐步删边)
 *
 * 协作: CycleDetector(环检测) / DataFlowMeter(流图分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QSet>

/**
 * @class EulerianPath
 * @brief 欧拉路径/回路检测与求解
 *
 * 判定图是否存在欧拉路径或回路，若存在则输出一条经过
 * 每条边恰好一次的路径。
 */
class EulerianPath : public QObject
{
    Q_OBJECT

public:
    /** @brief 图类型 */
    enum class GraphType {
        Undirected,  ///< 无向图
        Directed     ///< 有向图
    };
    Q_ENUM(GraphType)

    /** @brief 欧拉性质 */
    enum class EulerianProperty {
        None,        ///< 非欧拉图
        Path,        ///< 有欧拉路径(无回路)
        Circuit      ///< 有欧拉回路
    };
    Q_ENUM(EulerianProperty)

    /** @brief 边结构 */
    struct Edge {
        int from = 0;     ///< 起点
        int to = 0;       ///< 终点
        int id = 0;       ///< 边ID(用于区分平行边)
        double weight = 1.0; ///< 权重(可选)
    };

    /** @brief 求解结果 */
    struct EulerResult {
        EulerianProperty property = EulerianProperty::None; ///< 欧拉性质
        QList<int> vertexPath;       ///< 顶点路径序列
        QList<int> edgePath;         ///< 边ID路径序列
        int totalEdges = 0;          ///< 总边数
        int totalVertices = 0;       ///< 总顶点数
        bool hasEulerian = false;    ///< 是否存在欧拉路径/回路
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalChecks = 0;           ///< 累计检测次数
        quint64 totalSolves = 0;           ///< 累计求解次数
        quint64 totalEdgesProcessed = 0;   ///< 累计处理边数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit EulerianPath(QObject* parent = nullptr);

    /**
     * @brief 设置图类型
     * @param type 无向/有向
     */
    void setGraphType(GraphType type);

    /**
     * @brief 添加边
     * @param from 起点
     * @param to 终点
     * @param weight 权重
     */
    void addEdge(int from, int to, double weight = 1.0);

    /**
     * @brief 批量设置边
     * @param edges 边列表
     */
    void setEdges(const QList<Edge>& edges);

    /**
     * @brief 检测欧拉性质(不求解路径)
     * @param vertexCount 顶点数
     * @return 欧拉性质
     */
    EulerianProperty checkEulerian(int vertexCount) const;

    /**
     * @brief 求解欧拉路径/回路(Hierholzer算法)
     * @param vertexCount 顶点数
     * @return 欧拉求解结果
     */
    EulerResult solve(int vertexCount);

    /**
     * @brief 清空图
     */
    void clearGraph();

    /**
     * @brief 获取连通分量数
     * @param vertexCount 顶点数
     * @return 连通分量数
     */
    int connectedComponents(int vertexCount) const;

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检测完成 @param property 欧拉性质 */
    void checkCompleted(EulerianProperty property);

    /** @brief 求解完成 @param result 结果 */
    void solveCompleted(const EulerResult& result);

private:
    /** @brief 深度优先搜索计算连通分量 */
    void dfs(int v, QSet<int>& visited,
             const QMap<int, QList<QPair<int, int>>>& adj) const;

    /** @brief Hierholzer算法核心 */
    void hierholzer(int start,
                    QMap<int, QList<QPair<int, int>>>& adj,
                    QList<int>& circuit);

    /** @brief 构建邻接表 */
    QMap<int, QList<QPair<int, int>>> buildAdjacencyList() const;

    /** @brief 找到欧拉路径起点 */
    int findStartVertex(int vertexCount) const;

    GraphType m_graphType = GraphType::Undirected; ///< 图类型
    QList<Edge> m_edges;                           ///< 边列表
    int m_nextEdgeId = 0;                          ///< 下一个边ID

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};
