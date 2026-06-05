/**
 * @file EulerTour.h
 * @brief 欧拉游历引擎 — 树的欧拉遍历/LCA/进出时间戳
 *
 * 功能: 对树结构执行欧拉游历，记录进出时间戳，
 *       支持基于Sparse Table的LCA查询。
 *
 * 协作: CycleDetector(环检测) / StateTracker(状态树遍历)
 */
#ifndef EULERTOUR_H
#define EULERTOUR_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 欧拉游历 — 树遍历与LCA查询
 */
class EulerTour : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalTours = 0;             ///< 累计游历次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit EulerTour(QObject* parent = nullptr);

    /**
     * @brief 执行欧拉游历
     * @param adj 邻接表(树结构)
     * @param start 起始节点
     * @return 欧拉序列(节点索引列表)
     */
    QVector<int> tour(const QVector<QVector<int>>& adj, int start);

    /**
     * @brief 获取最近一次游历的进出时间戳
     * @return 每个节点的(进入时间, 离开时间)列表
     */
    QVector<int> inOutTimes() const { return m_inTime; }

    /**
     * @brief 查询两节点的最近公共祖先
     * @param u 节点u
     * @param v 节点v
     * @return LCA节点索引，-1表示无效
     */
    int lca(int u, int v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 游历完成信号 @param nodeCount 遍历节点数 */
    void tourCompleted(int nodeCount);

private:
    /** @brief DFS递归游历 */
    void dfs(const QVector<QVector<int>>& adj, int node, int parent, int depth);

    /** @brief 构建Sparse Table用于RMQ */
    void buildSparseTable();

    /** @brief RMQ查询区间最小深度位置 */
    int rmq(int l, int r) const;

    QVector<int>    m_eulerSeq;     ///< 欧拉序列
    QVector<int>    m_depth;        ///< 对应深度
    QVector<int>    m_firstOcc;     ///< 每个节点首次出现位置
    QVector<int>    m_inTime;       ///< 进入时间戳
    QVector<int>    m_outTime;      ///< 离开时间戳
    int             m_timer;        ///< 时间戳计数器

    /* Sparse Table */
    QVector<QVector<int>> m_sparseTable;   ///< ST表
    QVector<int>          m_logTable;      ///< 预计算log2

    double m_timeSum;           ///< 累计耗时(ms)
    mutable Stats m_stats;      ///< 可变统计
};

#endif // EULERTOUR_H
