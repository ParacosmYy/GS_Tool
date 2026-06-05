/**
 * @file VertexCover4.cpp
 * @brief 顶点覆盖问题求解器实现
 *
 * 实现图的最小顶点覆盖近似算法，使用贪心策略和
 * 2-近似算法。顶点覆盖是NP-hard问题，本实现提供
 * 高效的近似解。支持两种求解策略：标准2-近似贪心
 * 和基于最大匹配的改进算法。同时支持覆盖验证。
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
 */
VertexCover4::VertexCover4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量，自动初始化邻接表
 *
 * 清空现有邻接表并重新分配n个顶点的邻接表空间。
 * 所有已添加的边将被清除，需重新构建图。
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
 * 在邻接表中双向添加边关系。忽略自环(u==v)和越界索引。
 * 重复添加同一条边不会产生重复记录，保证邻接表无重边。
 */
void VertexCover4::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return;

    /* 避免重复边 */
    if (!m_adj[u].contains(v)) {
        m_adj[u].append(v);
    }
    if (!m_adj[v].contains(u)) {
        m_adj[v].append(u);
    }
}

/**
 * @brief 求解最小顶点覆盖(基于最大匹配的2-近似)
 * @return 覆盖中的顶点索引集合
 *
 * 使用基于最大匹配的2-近似算法:
 * 1. 收集所有无向边
 * 2. 贪心构建极大匹配: 每次选择一条未覆盖边，加入匹配
 * 3. 将匹配中每条边的两个端点都加入覆盖
 *
 * 近似比保证: |cover| <= 2 * |OPT|
 * 因为匹配中的边互不相交，任何覆盖必须至少包含每条匹配边的一个端点。
 * 时间复杂度: O(V + E)
 */
QVector<int> VertexCover4::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return QVector<int>();

    /* 阶段1: 收集所有无向边(去重: 仅存储u < v) */
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
        emit solveCompleted(0);
        return QVector<int>();
    }

    /* 阶段2: 贪心构建极大匹配 */
    QVector<bool> vertexUsed(m_n, false);    /* 顶点是否已被匹配使用 */
    QVector<QPair<int, int>> matching;       /* 匹配中的边 */

    for (int e = 0; e < totalEdges; ++e) {
        int u = edges[e].first;
        int v = edges[e].second;

        /* 两个端点都未被使用才能加入匹配 */
        if (!vertexUsed[u] && !vertexUsed[v]) {
            matching.append({u, v});
            vertexUsed[u] = true;
            vertexUsed[v] = true;
        }
    }

    /* 阶段3: 将匹配中每条边的两端都加入覆盖 */
    QVector<int> cover;
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
 * @return true表示该集合覆盖图中所有边
 *
 * 验证逻辑: 遍历每条无向边(u,v)，检查至少有一个端点
 * 在cover集合中。若存在未被覆盖的边则返回false。
 * 空图(无边)总是被任何集合(包括空集)覆盖。
 */
bool VertexCover4::isVertexCover(const QVector<int>& cover) const
{
    QSet<int> coverSet;
    for (int v : cover) {
        coverSet.insert(v);
    }

    for (int u = 0; u < m_n; ++u) {
        for (int v : m_adj[u]) {
            if (u < v) {
                /* 边(u,v)未被覆盖: 两个端点都不在cover中 */
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
 * 清零所有累计统计数据(totalSolves, totalVertices)
 * 和计时累加器，平均处理时间归零。
 */
void VertexCover4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
