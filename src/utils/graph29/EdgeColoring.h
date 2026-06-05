/**
 * @file EdgeColoring.h
 * @brief 边着色引擎 — Vizing定理近似 + 匹配着色
 *
 * 功能: 实现图边着色算法，基于Vizing定理(Delta <= chi' <= Delta+1)
 *       给出Delta+1着色的近似解。支持匹配分解着色、贪心边着色、
 *       色指数计算。适用于任务调度、寄存器分配、频率分配等场景。
 *
 * 协作: GraphAnalyzer(图分析) / VertexCover(顶点覆盖)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 边着色 — Vizing定理近似
 *
 * 色指数 chi'(G): 给图G的每条边着色使得相邻边颜色不同的最小颜色数。
 * Vizing定理: Delta(G) <= chi'(G) <= Delta(G) + 1
 * 其中Delta(G)是最大度数。
 */
class EdgeColoring : public QObject {
    Q_OBJECT

public:
    /** @brief 边着色结果 */
    struct ColoringResult {
        QMap<QPair<int, int>, int> edgeColors;  ///< 边 -> 颜色编号
        int chromaticIndex = 0;                  ///< 色指数(使用的颜色数)
        int maxDegree = 0;                       ///< 最大度数
        int vertexCount = 0;                     ///< 顶点数
        int edgeCount = 0;                       ///< 边数
        bool isOptimal = false;                  ///< 是否达到最优(Delta着色)
        bool success = false;                   ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalColorings = 0;          ///< 总着色次数
        quint64 totalEdgesProcessed = 0;     ///< 总处理边数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit EdgeColoring(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~EdgeColoring() override;

    // ── 核心算法 ──

    /**
     * @brief Vizing贪心边着色(Delta+1近似)
     * @param edges 边列表(顶点对)
     * @param numVertices 顶点数(顶点编号0~numVertices-1)
     * @return 着色结果
     */
    ColoringResult colorVizing(const QVector<QPair<int, int>>& edges,
                                int numVertices);

    /**
     * @brief 匹配分解边着色
     * @param edges 边列表
     * @param numVertices 顶点数
     * @return 着色结果
     */
    ColoringResult colorByMatchings(const QVector<QPair<int, int>>& edges,
                                     int numVertices);

    /**
     * @brief 二分图最优边着色(Delta着色)
     * @param edges 边列表
     * @param numVertices 顶点数
     * @param leftSize 左部顶点数
     * @return 着色结果(二分图保证Delta最优)
     */
    ColoringResult colorBipartite(const QVector<QPair<int, int>>& edges,
                                  int numVertices, int leftSize);

    // ── 查询 ──

    /**
     * @brief 验证着色是否合法
     * @param edges 边列表
     * @param edgeColors 边着色映射
     * @return 是否合法(无相邻边同色)
     */
    bool verifyColoring(const QVector<QPair<int, int>>& edges,
                        const QMap<QPair<int, int>, int>& edgeColors) const;

    /**
     * @brief 计算最大度数
     * @param edges 边列表
     * @param numVertices 顶点数
     * @return 最大度数
     */
    static int computeMaxDegree(const QVector<QPair<int, int>>& edges,
                                int numVertices);

    // ── 统计 ──

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 着色完成 @param colors 使用颜色数 @param edges 边数 */
    void coloringCompleted(int colors, int edges);

private:
    /**
     * @brief 构建邻接表
     * @param edges 边列表
     * @param numVertices 顶点数
     * @return 邻接表(顶点 -> 相邻顶点列表)
     */
    static QVector<QVector<int>> buildAdjacencyList(
        const QVector<QPair<int, int>>& edges, int numVertices);

    /**
     * @brief 查找最大匹配(Hopcroft-Karp简化版)
     * @param adj 邻接表
     * @param leftVertices 左部顶点集合
     * @param numVertices 顶点数
     * @return 匹配边列表
     */
    QVector<QPair<int, int>> findMaxMatching(
        const QVector<QVector<int>>& adj,
        const QSet<int>& leftVertices,
        int numVertices) const;

    Stats m_stats;                    ///< 操作统计
    double m_timeSum = 0.0;          ///< 累计耗时
};
