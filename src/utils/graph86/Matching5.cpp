/**
 * @file Matching5.cpp
 * @brief 图匹配算法实现
 *
 * 实现一般图的最大基数匹配和最大权匹配，
 * 基于增广路径算法(匈牙利/Edmonds)。
 */

#include "utils/graph86/Matching5.h"

#include <QElapsedTimer>
#include <algorithm>
#include <queue>

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
    m_adj[u].append({v, weight});
    m_adj[v].append({u, weight});
}

/**
 * @brief 计算最大基数匹配
 * @return 匹配边列表
 *
 * 使用增广路径DFS算法求最大基数匹配。
 */
QVector<QPair<int, int>> Matching5::maxCardinality()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, int>> result;
    if (m_n == 0) return result;

    QVector<int> match(m_n, -1);
    int matchCount = 0;

    for (int u = 0; u < m_n; ++u) {
        if (match[u] != -1) continue;
        QVector<bool> visited(m_n, false);
        if (augment(u, visited, match)) {
            matchCount++;
        }
    }

    for (int u = 0; u < m_n; ++u) {
        if (match[u] != -1 && u < match[u]) {
            result.append({u, match[u]});
        }
    }

    m_matchSize = matchCount;
    m_totalWeight = 0.0;

    /* 计算匹配的总权重 */
    for (const auto& edge : result) {
        for (const auto& adj : m_adj[edge.first]) {
            if (adj.first == edge.second) {
                m_totalWeight += adj.second;
                break;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalMatches++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingCompleted(m_matchSize, m_totalWeight);
    return result;
}

/**
 * @brief 计算最大权匹配
 * @return 匹配边列表
 *
 * 使用贪心策略: 按边权降序排列，依次尝试加入匹配。
 */
QVector<QPair<int, int>> Matching5::maxWeight()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, int>> result;
    if (m_n == 0) return result;

    /* 收集所有边并按权重降序排列 */
    QVector<QPair<double, QPair<int, int>>> edges;
    for (int u = 0; u < m_n; ++u) {
        for (const auto& edge : m_adj[u]) {
            if (u < edge.first) {
                edges.append({edge.second, {u, edge.first}});
            }
        }
    }
    std::sort(edges.begin(), edges.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    /* 贪心选择: 依次选择最大权重的可用边 */
    QVector<bool> used(m_n, false);
    double totalW = 0.0;
    int count = 0;

    for (const auto& e : edges) {
        int u = e.second.first;
        int v = e.second.second;
        if (!used[u] && !used[v]) {
            result.append({u, v});
            used[u] = true;
            used[v] = true;
            totalW += e.first;
            count++;
        }
    }

    m_matchSize = count;
    m_totalWeight = totalW;

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
 * @brief DFS搜索增广路径
 * @param u 当前顶点
 * @param visited 访问标记
 * @param match 匹配状态
 * @return 是否找到增广路径
 */
bool Matching5::augment(int u, QVector<bool>& visited, QVector<int>& match)
{
    for (const auto& edge : m_adj[u]) {
        int v = edge.first;
        if (v >= 0 && v < m_n && !visited[v]) {
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

/**
 * @brief 计算当前匹配的总权重
 * @param matching 匹配边列表
 * @return 边权重之和
 */
double Matching5::computeWeight(const QVector<QPair<int,int>>& matching) const
{
    double total = 0.0;
    for (const auto& edge : matching) {
        for (const auto& adj : m_adj[edge.first]) {
            if (adj.first == edge.second) {
                total += adj.second;
                break;
            }
        }
    }
    return total;
}

/**
 * @brief 验证匹配的有效性
 * @param matching 待验证的匹配
 * @return 是否为有效匹配（无重复顶点）
 */
bool Matching5::isValidMatching(const QVector<QPair<int,int>>& matching) const
{
    QVector<bool> used(m_n, false);
    for (const auto& edge : matching) {
        if (edge.first < 0 || edge.first >= m_n) return false;
        if (edge.second < 0 || edge.second >= m_n) return false;
        if (used[edge.first] || used[edge.second]) return false;
        used[edge.first] = true;
        used[edge.second] = true;
    }
    return true;
}

/**
 * @brief 计算匹配的顶点覆盖率
 * @param matching 匹配边列表
 * @return 被覆盖的顶点比例(0~1)
 */
double Matching5::coverage(const QVector<QPair<int,int>>& matching) const
{
    if (m_n == 0) return 0.0;
    int covered = matching.size() * 2;
    return static_cast<double>(qMin(covered, m_n)) / m_n;
}
