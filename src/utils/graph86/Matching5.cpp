/**
 * @file Matching5.cpp
 * @brief 一般图匹配算法实现
 *
 * 实现最大基数匹配和最大权匹配算法，
 * 支持加权无向图的最优匹配。
 */

#include "utils/graph86/Matching5.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
Matching5::Matching5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param n 顶点数
 */
void Matching5::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
}

/**
 * @brief 添加带权边
 * @param u 端点1
 * @param v 端点2
 * @param weight 边权重
 */
void Matching5::addEdge(int u, int v, double weight)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    // 避免重复边
    bool exists = false;
    for (auto& edge : m_adj[u]) {
        if (edge.first == v) { edge.second = weight; exists = true; break; }
    }
    if (!exists) {
        m_adj[u].append({v, weight});
        m_adj[v].append({u, weight});
    }
}

/**
 * @brief 计算最大基数匹配
 * @return 匹配边列表
 */
QVector<QPair<int,int>> Matching5::maxCardinality()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> match(m_n, -1);
    int size = 0;

    for (int u = 0; u < m_n; ++u) {
        if (match[u] != -1) continue;
        QVector<bool> visited(m_n, false);
        if (augment(u, visited, match)) size++;
    }

    // 提取匹配边
    QVector<QPair<int,int>> result;
    for (int u = 0; u < m_n; ++u) {
        if (match[u] > u) {
            result.append({u, match[u]});
        }
    }

    m_matchSize = result.size();
    m_totalWeight = 0.0;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(m_matchSize, m_totalWeight);
    return result;
}

/**
 * @brief 计算最大权匹配（贪心近似）
 * @return 匹配边列表
 */
QVector<QPair<int,int>> Matching5::maxWeight()
{
    QElapsedTimer timer;
    timer.start();

    // 收集所有边并按权重降序排列
    struct Edge { int u, v; double w; };
    QVector<Edge> edges;
    for (int u = 0; u < m_n; ++u) {
        for (const auto& e : m_adj[u]) {
            if (e.first > u) {
                edges.append({u, e.first, e.second});
            }
        }
    }
    std::sort(edges.begin(), edges.end(),
              [](const Edge& a, const Edge& b) { return a.w > b.w; });

    // 贪心匹配
    QVector<bool> matched(m_n, false);
    QVector<QPair<int,int>> result;
    m_totalWeight = 0.0;

    for (const auto& edge : edges) {
        if (!matched[edge.u] && !matched[edge.v]) {
            matched[edge.u] = true;
            matched[edge.v] = true;
            result.append({edge.u, edge.v});
            m_totalWeight += edge.w;
        }
    }

    m_matchSize = result.size();

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(m_matchSize, m_totalWeight);
    return result;
}

/**
 * @brief 重置统计信息
 */
void Matching5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief DFS增广路径搜索
 * @param u 当前顶点
 * @param visited 访问标记
 * @param match 匹配状态
 * @return 是否找到增广路径
 */
bool Matching5::augment(int u, QVector<bool>& visited, QVector<int>& match)
{
    for (const auto& edge : m_adj[u]) {
        int v = edge.first;
        if (v >= 0 && v < visited.size() && !visited[v]) {
            visited[v] = true;
            if (match[v] == -1 || augment(match[v], visited, match)) {
                match[v] = u;
                match[u] = v;
                return true;
            }
        }
    }
    return false;
}
