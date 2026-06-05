/**
 * @file VertexCover2.h
 * @brief 顶点覆盖增强 — 2-近似/分支定界/核化简/参数化算法
 *
 * 功能: 求解图的最小顶点覆盖问题，支持贪心2-近似、精确分支定界、
 *       核化简预处理和Buss核化、参数化求解。
 *
 * 协作: DataClassifier(分类) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 顶点覆盖增强 — 多算法最小顶点覆盖求解器
 */
class VertexCover2 : public QObject {
    Q_OBJECT

public:
    /** @brief 算法选择 */
    enum class Algorithm {
        Greedy2Approx,   ///< 贪心2-近似算法
        BranchAndBound,  ///< 精确分支定界
        Kernelization,   ///< 核化简+参数化
        BussKernel       ///< Buss核化(度数过滤)
    };
    Q_ENUM(Algorithm)

    /** @brief 求解结果 */
    struct Solution {
        QVector<int> vertices;       ///< 顶点覆盖集合
        bool isOptimal = false;      ///< 是否为最优解
        double solveTimeMs = 0.0;    ///< 求解耗时(ms)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;             ///< 累计求解次数
        quint64 totalVerticesProcessed = 0;  ///< 累计处理顶点数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
        quint64 totalOptimalSolves = 0;      ///< 精确解次数
    };

    explicit VertexCover2(QObject* parent = nullptr);

    /** @brief 构建邻接表图 @param edges 边列表(顶点从0开始) @param vertexCount 顶点数 */
    void buildGraph(const QList<QPair<int, int>>& edges, int vertexCount);

    /** @brief 求解顶点覆盖 @param algo 算法 @param k 参数k(核化简用) @return 解 */
    Solution solve(Algorithm algo = Algorithm::Greedy2Approx, int k = -1);

    /** @brief 验证解是否为合法顶点覆盖 @param cover 顶点集合 @return 是否合法 */
    bool verifyCover(const QVector<int>& cover) const;

    /** @brief 获取图信息 @return (顶点数, 边数) */
    QPair<int, int> graphInfo() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param coverSize 覆盖大小 @param isOptimal 是否最优 */
    void solveComplete(int coverSize, bool isOptimal);

private:
    Solution greedy2Approx();
    Solution branchAndBound();
    Solution kernelization(int k);
    Solution bussKernel(int k);

    void bbSearch(const QVector<QPair<int, int>>& remainingEdges,
                  QVector<int>& currentCover,
                  QVector<int>& bestCover, int k);
    void reduceGraph(QVector<QPair<int, int>>& edges,
                     QVector<int>& cover, int k);
    QVector<QPair<int, int>> removeIsolated(
        const QVector<QPair<int, int>>& edges, int vertexCount) const;

    int m_vertexCount = 0;               ///< 顶点数
    QList<QPair<int, int>> m_edges;      ///< 边列表
    QVector<QVector<int>> m_adjList;     ///< 邻接表

    Stats m_stats;
    double m_timeSum = 0.0;
};
