/**
 * @file ChromaticIndex.h
 * @brief 边着色(Vizing定理) — Edge Coloring via Vizing's Theorem
 *
 * 功能: 基于Vizing定理计算图的边色数(chromatic index)。
 *       简单图色数为Δ或Δ+1，通过Misra & Gries贪心算法实现边着色。
 *
 * 协作: GraphColoring(顶点着色) / BipartiteMatching(二分图匹配)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>

/**
 * @brief 边着色器，基于Vizing定理计算chromatic index
 */
class ChromaticIndex : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalColorings = 0;         ///< 累计着色次数
        quint64 totalEdgesColored = 0;      ///< 累计着色边数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        int chromaticIndex = 0;             ///< 最近一次的边色数
    };

    explicit ChromaticIndex(QObject* parent = nullptr);

    /**
     * @brief 添加无向边
     * @param u 端点u
     * @param v 端点v
     */
    void addEdge(int u, int v);

    /**
     * @brief 执行边着色(Misra & Gries算法)
     * @return 每条边的颜色编号(从1开始)，0表示未着色
     */
    QVector<int> colorEdges();

    /**
     * @brief 获取最大度数Δ
     */
    int maxDegree() const { return m_maxDegree; }

    /**
     * @brief 获取边列表
     */
    QVector<QPair<int, int>> edges() const { return m_edges; }

    /**
     * @brief 清空图
     */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 着色完成 @param colors 使用的颜色数 @param edges 边数 */
    void coloringCompleted(int colors, int edges);

private:
    /** @brief Misra & Gries算法核心 */
    int misraGries(QVector<int>& edgeColors);

    /** @brief 找到顶点的空闲颜色 */
    int freeColor(int vertex, const QVector<QMap<int, int>>& vertexColors,
                  int maxColor) const;

    /** @brief 构造极大fan */
    QVector<int> buildFan(int startVertex, int edgeIdx,
                          const QVector<int>& edgeColors,
                          const QVector<QMap<int, int>>& vertexColors,
                          int maxColor) const;

    QVector<QPair<int, int>> m_edges;
    int m_maxDegree = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
