/**
 * @file EulerTour.cpp
 * @brief 欧拉游历引擎实现 — 树遍历与LCA查询
 */

#include "utils/euler2/EulerTour.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
EulerTour::EulerTour(QObject* parent)
    : QObject(parent)
    , m_timer(0)
    , m_timeSum(0.0)
{
}

/** @brief 执行欧拉游历 @param adj 邻接表 @param start 起始节点 @return 欧拉序列 */
QVector<int> EulerTour::tour(const QVector<QVector<int>>& adj, int start)
{
    QElapsedTimer timer;
    timer.start();

    int n = adj.size();
    m_eulerSeq.clear();
    m_depth.clear();
    m_firstOcc.assign(n, -1);
    m_inTime.assign(n, -1);
    m_outTime.assign(n, -1);
    m_timer = 0;

    if (n == 0 || start < 0 || start >= n) {
        ++m_stats.totalTours;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTours;
        emit tourCompleted(0);
        return m_eulerSeq;
    }

    /* DFS遍历，构建欧拉序列 */
    dfs(adj, start, -1, 0);

    /* 构建Sparse Table用于LCA的RMQ查询 */
    buildSparseTable();

    /* 更新统计 */
    ++m_stats.totalTours;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTours;

    emit tourCompleted(n);
    return m_eulerSeq;
}

/** @brief DFS递归游历 @param adj 邻接表 @param node 当前节点 @param parent 父节点 @param d 深度 */
void EulerTour::dfs(const QVector<QVector<int>>& adj, int node,
                    int parent, int d)
{
    m_firstOcc[node] = m_eulerSeq.size();
    m_inTime[node] = m_timer++;
    m_eulerSeq.append(node);
    m_depth.append(d);

    for (int child : adj[node]) {
        if (child == parent) continue;
        dfs(adj, child, node, d + 1);
        /* 回溯时再次记录 */
        m_eulerSeq.append(node);
        m_depth.append(d);
    }

    m_outTime[node] = m_timer++;
}

/** @brief 构建Sparse Table */
void EulerTour::buildSparseTable()
{
    int n = m_depth.size();
    if (n == 0) return;

    /* 预计算log2表 */
    m_logTable.resize(n + 1);
    m_logTable[1] = 0;
    for (int i = 2; i <= n; ++i) {
        m_logTable[i] = m_logTable[i / 2] + 1;
    }

    /* 构建ST表: 存储的是深度最小值的位置索引 */
    int maxLog = m_logTable[n] + 1;
    m_sparseTable.assign(maxLog, QVector<int>(n));

    /* 第0层: 自身 */
    for (int i = 0; i < n; ++i) {
        m_sparseTable[0][i] = i;
    }

    /* 逐层构建 */
    for (int k = 1; k < maxLog; ++k) {
        int len = 1 << k;
        for (int i = 0; i + len <= n; ++i) {
            int left = m_sparseTable[k - 1][i];
            int right = m_sparseTable[k - 1][i + (1 << (k - 1))];
            m_sparseTable[k][i] =
                (m_depth[left] <= m_depth[right]) ? left : right;
        }
    }
}

/** @brief RMQ查询区间最小深度位置 @param l 左边界 @param r 右边界 @return 最小深度位置 */
int EulerTour::rmq(int l, int r) const
{
    if (l > r) qSwap(l, r);
    int k = m_logTable[r - l + 1];
    int left = m_sparseTable[k][l];
    int right = m_sparseTable[k][r - (1 << k) + 1];
    return (m_depth[left] <= m_depth[right]) ? left : right;
}

/** @brief 查询LCA @param u 节点u @param v 节点v @return 最近公共祖先 */
int EulerTour::lca(int u, int v) const
{
    int n = m_firstOcc.size();
    if (u < 0 || u >= n || v < 0 || v >= n) return -1;
    if (m_eulerSeq.isEmpty()) return -1;

    int l = m_firstOcc[u];
    int r = m_firstOcc[v];
    int pos = rmq(l, r);
    return m_eulerSeq[pos];
}

/** @brief 重置统计 */
void EulerTour::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
