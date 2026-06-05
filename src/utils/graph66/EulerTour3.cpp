/**
 * @file EulerTour3.cpp
 * @brief 欧拉游走树（Euler Tour Tree）实现
 *
 * 基于欧拉游走的树结构，支持最近公共祖先（LCA）、距离、路径查询。
 * 通过对树执行DFS欧拉游走，构建欧拉序列和深度序列，
 * 再使用稀疏表（Sparse Table）实现O(1)的LCA查询。
 *
 * 核心数据结构:
 * - m_euler: 欧拉游走序列（节点编号）
 * - m_depth: 对应深度序列
 * - m_first: 每个节点在欧拉序列中首次出现的位置
 * - m_sparse: RMQ稀疏表，用于快速区间最小值查询
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/graph66/EulerTour3.h"

#include <QElapsedTimer>
#include <algorithm>
#include <QtMath>

/**
 * @brief 构造函数
 * @param parent 父QObject对象指针
 */
EulerTour3::EulerTour3(QObject* parent)
    : QObject(parent)
    , m_n(0)
    , m_root(0)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置树结构并构建欧拉游走
 *
 * 接受树的边列表，构建邻接表，执行DFS生成欧拉序列，
 * 并预处理稀疏表以支持O(1)的RMQ查询。
 *
 * @param n 顶点数量，编号 0 到 n-1
 * @param root 根节点编号
 * @param edges 边列表，每对表示一条无向边
 */
void EulerTour3::setTree(int n, int root, const QVector<QPair<int,int>>& edges)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    m_root = qBound(0, root, n - 1);

    /* 构建邻接表 */
    m_adj.clear();
    m_adj.resize(n);
    for (const auto& edge : edges) {
        int u = edge.first;
        int v = edge.second;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            m_adj[u].append(v);
            m_adj[v].append(u);
        }
    }

    /* 初始化DFS相关数组 */
    m_euler.clear();
    m_depth.resize(n, 0);
    m_first.resize(n, -1);

    /* 执行DFS生成欧拉序列 */
    dfs(m_root, -1, 0);

    /* 构建稀疏表用于RMQ查询 */
    int eulerLen = m_euler.size();
    int logLen = 1;
    while ((1 << logLen) <= eulerLen) {
        logLen++;
    }
    logLen = qMax(1, logLen);

    m_sparse.clear();
    m_sparse.resize(logLen, QVector<int>(eulerLen, 0));

    /* 第0层: 原始深度序列 */
    for (int i = 0; i < eulerLen; ++i) {
        m_sparse[0][i] = i;
    }

    /* 逐层构建: m_sparse[j][i] = 区间[i, i+2^j]中深度最小的索引 */
    for (int j = 1; j < logLen; ++j) {
        for (int i = 0; i + (1 << j) <= eulerLen; ++i) {
            int left = m_sparse[j - 1][i];
            int right = m_sparse[j - 1][i + (1 << (j - 1))];
            m_sparse[j][i] = (m_depth[m_euler[left]] <= m_depth[m_euler[right]])
                                  ? left : right;
        }
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_stats.totalBuilds++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalBuilds > 0)
        ? m_timeSum / m_stats.totalBuilds : 0.0;

    emit buildCompleted(n);
}

/**
 * @brief DFS遍历生成欧拉序列
 *
 * 从节点u开始深度优先遍历，将访问的每个节点加入欧拉序列。
 * 每次进入和回溯时都记录节点，确保子树信息完整。
 *
 * @param u 当前节点
 * @param p 父节点（-1表示根）
 * @param d 当前深度
 */
void EulerTour3::dfs(int u, int p, int d)
{
    m_depth[u] = d;
    m_first[u] = m_euler.size();
    m_euler.append(u);

    for (int v : m_adj[u]) {
        if (v != p) {
            dfs(v, u, d + 1);
            m_euler.append(u);  ///< 回溯时再次记录
        }
    }
}

/**
 * @brief 区间最小值查询（RMQ）
 *
 * 使用稀疏表在O(1)时间内查询深度序列中区间[i, j]的
 * 最小深度值对应的欧拉序列索引。
 *
 * @param i 区间左端点
 * @param j 区间右端点
 * @return 最小深度对应的欧拉序列索引
 */
int EulerTour3::rmq(int i, int j) const
{
    if (i > j) {
        std::swap(i, j);
    }

    int len = j - i + 1;
    int k = 0;
    while ((1 << (k + 1)) <= len) {
        k++;
    }

    int left = m_sparse[k][i];
    int right = m_sparse[k][j - (1 << k) + 1];

    return (m_depth[m_euler[left]] <= m_depth[m_euler[right]]) ? left : right;
}

/**
 * @brief 查询两个节点的最近公共祖先（LCA）
 *
 * 基于欧拉序列的LCA查询: LCA(u,v) = 欧拉序列中 first[u] 到 first[v]
 * 区间内深度最小的节点。使用RMQ稀疏表实现O(1)查询。
 *
 * @param u 第一个节点编号
 * @param v 第二个节点编号
 * @return 最近公共祖先的节点编号
 */
int EulerTour3::lca(int u, int v) const
{
    QElapsedTimer timer;
    timer.start();

    if (u < 0 || u >= m_n || v < 0 || v >= m_n) {
        return -1;
    }

    if (u == v) {
        return u;
    }

    /* 在欧拉序列中查询区间[first[u], first[v]]的深度最小值 */
    int idx = rmq(m_first[u], m_first[v]);
    int result = m_euler[idx];

    /* 更新查询统计 */
    const_cast<EulerTour3*>(this)->m_stats.totalQueries++;
    double elapsed = timer.elapsed();
    const_cast<EulerTour3*>(this)->m_timeSum += elapsed;
    const_cast<EulerTour3*>(this)->m_stats.avgProcessingTimeMs =
        (m_stats.totalBuilds + m_stats.totalQueries > 0)
            ? m_timeSum / (m_stats.totalBuilds + m_stats.totalQueries) : 0.0;

    return result;
}

/**
 * @brief 计算两个节点之间的距离
 *
 * 距离 = depth[u] + depth[v] - 2 * depth[lca(u,v)]
 *
 * @param u 第一个节点
 * @param v 第二个节点
 * @return 路径上的边数
 */
int EulerTour3::distance(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) {
        return -1;
    }

    int ancestor = lca(u, v);
    return m_depth[u] + m_depth[v] - 2 * m_depth[ancestor];
}

/**
 * @brief 查询两个节点之间的路径
 *
 * 通过LCA节点将路径分为两段，分别从u和v向上追溯到LCA，
 * 然后合并为完整路径。
 *
 * @param u 起始节点
 * @param v 目标节点
 * @return 从u到v的路径节点列表
 */
QVector<int> EulerTour3::path(int u, int v) const
{
    QVector<int> result;

    if (u < 0 || u >= m_n || v < 0 || v >= m_n) {
        return result;
    }

    if (u == v) {
        result.append(u);
        return result;
    }

    int ancestor = lca(u, v);

    /* 从u追溯到LCA */
    QVector<int> pathU;
    int cur = u;
    while (cur != ancestor) {
        pathU.append(cur);
        /* 向上追溯: 找到深度减1的邻接节点 */
        for (int neighbor : m_adj[cur]) {
            if (m_depth[neighbor] == m_depth[cur] - 1) {
                cur = neighbor;
                break;
            }
        }
    }
    pathU.append(ancestor);

    /* 从v追溯到LCA */
    QVector<int> pathV;
    cur = v;
    while (cur != ancestor) {
        pathV.append(cur);
        for (int neighbor : m_adj[cur]) {
            if (m_depth[neighbor] == m_depth[cur] - 1) {
                cur = neighbor;
                break;
            }
        }
    }

    /* 合并: pathU正序 + pathV逆序 */
    result = pathU;
    for (int i = pathV.size() - 1; i >= 0; --i) {
        result.append(pathV[i]);
    }

    return result;
}

/**
 * @brief 重置所有统计计数器
 */
void EulerTour3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
