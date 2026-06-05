/**
 * @file DominatingSet.h
 * @brief 支配集求解 — 贪心近似/单位圆图/连通支配集/最小权重
 *
 * 图支配集算法集合:
 *   - 贪心近似: 每次选覆盖最多未覆盖顶点的顶点
 *   - 单位圆图: 利用几何性质的高效2-近似算法
 *   - 连通支配集(CDS): 保证支配集诱导子图连通
 *   - 最小权重支配集: 带权版本的贪心近似
 * 统计求解次数、迭代步数和支配集大小。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QPair>

/**
 * @brief 支配集求解器
 */
class DominatingSet : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 总求解次数
        quint64 totalIterations = 0;        ///< 总贪心迭代步数
        quint64 totalDominatingsetSize = 0; ///< 总支配集大小累计
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /** 图表示: 邻接表 */
    using AdjList = QVector<QVector<int>>;

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit DominatingSet(QObject* parent = nullptr);

    /**
     * @brief 贪心近似求最小支配集
     * @param adj 邻接表(adj[i] = i的邻居列表)
     * @return 支配集中的顶点索引
     */
    QVector<int> greedyDominatingSet(const AdjList& adj);

    /**
     * @brief 单位圆图支配集(2-近似)
     * @param positions 顶点坐标 [x0,y0,x1,y1,...]
     * @param radius 单位圆半径
     * @return 支配集中的顶点索引
     */
    QVector<int> unitDiskDominatingSet(const QVector<double>& positions, double radius);

    /**
     * @brief 连通支配集(基于贪心+Steiner树连通化)
     * @param adj 邻接表
     * @return 连通支配集
     */
    QVector<int> connectedDominatingSet(const AdjList& adj);

    /**
     * @brief 最小权重支配集(带权贪心)
     * @param adj 邻接表
     * @param weights 每个顶点的权重
     * @return 最小权重支配集
     */
    QVector<int> weightedDominatingSet(const AdjList& adj, const QVector<double>& weights);

    /** @brief 验证给定集合是否为支配集 */
    bool isDominating(const AdjList& adj, const QVector<int>& candidate) const;

    /** @brief 验证给定集合是否连通 */
    bool isConnected(const AdjList& adj, const QVector<int>& candidate) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** 求解完成 */
    void solveComplete(int setSize, int totalVertices, double processingTimeMs);

private:
    /** BFS找最短路径 */
    QVector<int> bfsPath(const AdjList& adj, int src, int dst) const;
    /** 从贪心支配集构建连通支配集 */
    QVector<int> makeConnected(const AdjList& adj, const QVector<int>& domSet);
    /** 构建单位圆图邻接表 */
    AdjList buildUnitDiskGraph(const QVector<double>& positions, double radius) const;

    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};
