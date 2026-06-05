/**
 * @file BipartiteGraph.h
 * @brief 二部图匹配 — Hopcroft-Karp最大匹配算法
 *
 * 功能: 构建二部图并使用Hopcroft-Karp算法求解最大匹配，
 *       支持最小顶点覆盖、最大独立集计算，
 *       适用于任务分配、资源匹配、通信调度等组合优化场景。
 *
 * 协作: DataFlowMeter(数据流) / DataBatchProcessor(批处理调度)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief 二部图 — Hopcroft-Karp最大匹配引擎
 *
 * 典型用法:
 * @code
 *   BipartiteGraph graph;
 *   graph.setPartitions(leftCount, rightCount);
 *   graph.addEdge(0, 1);
 *   auto matching = graph.maxMatching();
 *   int size = graph.maxMatchingSize();
 * @endcode
 */
class BipartiteGraph : public QObject {
    Q_OBJECT

public:
    /** @brief 匹配结果 */
    struct MatchingResult {
        QVector<QPair<int, int>> pairs;    ///< 匹配对(left, right)
        int size = 0;                       ///< 匹配大小
        double elapsedMs = 0.0;            ///< 计算耗时
        int bfsIterations = 0;             ///< BFS迭代次数
    };

    /** @brief 统计数据 */
    struct Stats {
        int totalMatches = 0;                   ///< 累计匹配计算次数
        int totalEdgesProcessed = 0;            ///< 累计处理边数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
        int totalMatchingsFound = 0;            ///< 累计匹配对数
        int totalVerticesProcessed = 0;         ///< 累计处理顶点数
    };

    explicit BipartiteGraph(QObject* parent = nullptr);

    /**
     * @brief 设置左右分区大小
     * @param leftSize 左侧顶点数
     * @param rightSize 右侧顶点数
     */
    void setPartitions(int leftSize, int rightSize);

    /**
     * @brief 添加一条边(left -> right)
     * @param left 左侧顶点索引(从0开始)
     * @param right 右侧顶点索引(从0开始)
     */
    void addEdge(int left, int right);

    /** @brief 清除所有边 */
    void clearEdges();

    /**
     * @brief 使用Hopcroft-Karp算法求最大匹配
     * @return 匹配结果
     */
    MatchingResult maxMatching();

    /**
     * @brief 获取最大匹配大小(不存储完整结果)
     * @return 匹配数
     */
    int maxMatchingSize();

    /**
     * @brief 计算最小顶点覆盖(Konig定理)
     * @param matching 当前匹配结果
     * @return 覆盖顶点索引列表(左侧为负值,右侧为正值)
     */
    QVector<int> minVertexCover(const MatchingResult& matching);

    /**
     * @brief 计算最大独立集
     * @param vertexCover 最小顶点覆盖
     * @return 独立集顶点列表
     */
    QVector<int> maxIndependentSet(const QVector<int>& vertexCover);

    /** @brief 获取邻接表 @return 左侧邻接表 */
    const QMap<int, QVector<int>>& adjacencyList() const;

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 匹配完成 @param result 匹配结果 */
    void matchingCompleted(const MatchingResult& result);

private:
    /** @brief BFS构建层次图 @param dist 距离数组 @return 是否存在增广路径 */
    bool bfs(QVector<int>& dist);

    /** @brief DFS寻找增广路径 @param u 左侧顶点 @param dist 距离数组 @return 是否找到 */
    bool dfs(int u, QVector<int>& dist);

    /** @brief 检查顶点覆盖的正确性 */
    bool verifyVertexCover(const QVector<int>& cover) const;

    int m_leftSize = 0;                        ///< 左侧顶点数
    int m_rightSize = 0;                       ///< 右侧顶点数
    QMap<int, QVector<int>> m_adj;             ///< 左侧邻接表
    QVector<int> m_matchLeft;                  ///< 左侧匹配(right index, -1=未匹配)
    QVector<int> m_matchRight;                 ///< 右侧匹配(left index, -1=未匹配)
    Stats m_stats;                             ///< 统计数据
    double m_timeSum = 0.0;                    ///< 时间累加器
};
