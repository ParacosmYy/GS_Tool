/**
 * @file VertexCover4.cpp
 * @brief 顶点覆盖问题求解器实现
 *
 * 实现图的最小顶点覆盖近似算法，使用基于最大匹配的
 * 2-近似策略。顶点覆盖是经典NP-hard问题，本实现通过
 * 贪心极大匹配提供2-近似保证的解。支持动态建图、
 * 覆盖验证和统计追踪功能。
 */

#include "utils/graph87/VertexCover4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 初始化空图，顶点数为0，邻接表为空，覆盖大小为0。
 * 所有内部状态初始化为默认值。
 */
VertexCover4::VertexCover4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量，必须非负
 *
 * 清空现有邻接表并重新分配n个顶点的邻接表空间。
 * 所有已添加的边将被清除，需要重新构建图结构。
 */
void VertexCover4::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
}

/**
 * @brief 添加无向边
 * @param u 第一个顶点索引(0-based)
 * @param v 第二个顶点索引(0-based)
 *
 * 在邻接表中双向添加边关系。安全检查包括:
 * - 忽略自环(u == v)
 * - 忽略越界索引(u >= n 或 v >= n)
 * - 防止重复边
 */
void VertexCover4::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return;

    /* 避免重复边: 双向检查 */
    if (!m_adj[u].contains(v)) {
        m_adj[u].append(v);
    }
    if (!m_adj[v].contains(u)) {
        m_adj[v].append(u);
    }
}

/**
 * @brief 求解最小顶点覆盖(基于极大匹配的2-近似)
 * @return 覆盖中的顶点索引集合
 *
 * 算法流程:
 * 1. 收集所有无向边(去重存储 u < v)
 * 2. 贪心构建极大匹配:
 *    - 遍历所有边，选择两端点均未使用的边加入匹配
 *    - 匹配中的边互不相交(共享端点)
 * 3. 将匹配中每条边的两个端点都加入覆盖
 *
 * 近似比证明:
 * - 匹配M中的边互不相交
 * - 任何顶点覆盖C必须覆盖M中的每条边
 * - 因此 |C| >= |M|
 * - 本算法返回 |cover| = 2*|M|
 * - 所以 |cover| <= 2*|C|, 即2-近似比
 *
 * 时间复杂度: O(V + E)
 */
QVector<int> VertexCover4::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) {
        m_coverSize = 0;
        emit solveCompleted(0);
        return QVector<int>();
    }

    /* ===== 阶段1: 收集所有无向边(去重) ===== */
    QVector<QPair<int, int>> edges;
    for (int u = 0; u < m_n; ++u) {
        for (int v : m_adj[u]) {
            if (u < v) {
                edges.append({u, v});
            }
        }
    }

    int totalEdges = edges.size();
    if (totalEdges == 0) {
        m_coverSize = 0;

        qint64 elapsed = timer.elapsed();
        m_stats.totalSolves++;
        m_stats.totalVertices += m_n;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

        emit solveCompleted(0);
        return QVector<int>();
    }

    /* ===== 阶段2: 贪心构建极大匹配 ===== */
    QVector<bool> vertexUsed(m_n, false);    /* 顶点是否已被匹配占用 */
    QVector<QPair<int, int>> matching;       /* 匹配边集合 */

    for (int e = 0; e < totalEdges; ++e) {
        int u = edges[e].first;
        int v = edges[e].second;

        /* 两个端点都未被匹配使用时，将此边加入匹配 */
        if (!vertexUsed[u] && !vertexUsed[v]) {
            matching.append({u, v});
            vertexUsed[u] = true;
            vertexUsed[v] = true;
        }
    }

    /* ===== 阶段3: 从匹配生成覆盖 ===== */
    /* 将匹配中每条边的两个端点加入覆盖集 */
    QVector<int> cover;
    cover.reserve(matching.size() * 2);
    for (const auto& edge : matching) {
        cover.append(edge.first);
        cover.append(edge.second);
    }

    m_coverSize = cover.size();

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_coverSize);
    return cover;
}

/**
 * @brief 检查给定顶点集合是否构成有效顶点覆盖
 * @param cover 候选覆盖集合(顶点索引数组)
 * @return true表示该集合覆盖图中所有边，false表示存在未覆盖的边
 *
 * 验证逻辑: 将cover转换为HashSet，然后遍历每条无向边(u,v)，
 * 检查至少有一个端点在coverSet中。若存在两端点都不在cover中
 * 的边，则该覆盖不完整，返回false。
 *
 * 空图(无边)的任何集合(包括空集)都是有效覆盖。
 */
bool VertexCover4::isVertexCover(const QVector<int>& cover) const
{
    /* 构建快速查找集合 */
    QSet<int> coverSet;
    coverSet.reserve(cover.size());
    for (int v : cover) {
        coverSet.insert(v);
    }

    /* 检查每条无向边是否被覆盖 */
    for (int u = 0; u < m_n; ++u) {
        for (int v : m_adj[u]) {
            if (u < v) {
                if (!coverSet.contains(u) && !coverSet.contains(v)) {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * @brief 重置统计信息
 *
 * 清零所有累计统计数据，包括求解次数、顶点总数、
 * 平均处理时间和计时累加器。
 */
void VertexCover4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
