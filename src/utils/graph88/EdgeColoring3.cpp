/**
 * @file EdgeColoring3.cpp
 * @brief 欧拉回路/路径算法实现
 *
 * 实现Hierholzer算法计算欧拉回路和欧拉路径，
 * 支持有根和无根图模式。
 */

#include "utils/graph88/EdgeColoring3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
EdgeColoring3::EdgeColoring3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置是否有根
 * @param rooted true=从指定起点出发，false=自动选择起点
 */
void EdgeColoring3::setRooted(bool rooted)
{
    m_rooted = rooted;
}

/**
 * @brief 设置顶点数量
 * @param n 顶点数
 */
void EdgeColoring3::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
    m_hasTour = false;
    m_tourLen = 0;
}

/**
 * @brief 添加无向边
 * @param u 端点1
 * @param v 端点2
 *
 * 每条无向边用两个有向半边表示，共享相同的边编号，
 * 便于后续标记已使用的边。
 */
void EdgeColoring3::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;

    /* 计算当前边编号 */
    int edgeId = 0;
    for (const auto& adj : m_adj) edgeId += adj.size();
    edgeId /= 2;

    m_adj[u].append({v, edgeId});
    m_adj[v].append({u, edgeId});
}

/**
 * @brief 计算欧拉回路/路径
 * @return 欧拉回路/路径的顶点序列
 *
 * 使用Hierholzer算法:
 * 1. 检查欧拉条件(所有顶点度数为偶数，或恰好两个奇数度)
 * 2. 选择起始顶点
 * 3. DFS搜索扩展回路/路径
 */
QVector<int> EdgeColoring3::compute()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> tour;
    if (m_n == 0) {
        m_hasTour = false;
        m_tourLen = 0;
        return tour;
    }

    /* 阶段1: 统计度数 */
    QVector<int> degree(m_n, 0);
    int edgeCount = 0;
    for (int u = 0; u < m_n; ++u) {
        degree[u] = m_adj[u].size();
        edgeCount += degree[u];
    }
    edgeCount /= 2;

    /* 阶段2: 检查欧拉条件 */
    int oddDegree = 0;
    int start = -1;
    for (int i = 0; i < m_n; ++i) {
        if (degree[i] % 2 != 0) {
            oddDegree++;
            if (start < 0) start = i;
        } else if (degree[i] > 0 && start < 0) {
            start = i;
        }
    }

    /* 欧拉回路条件: 0个奇数度顶点 */
    /* 欧拉路径条件: 恰好2个奇数度顶点 */
    if (oddDegree != 0 && oddDegree != 2) {
        m_hasTour = false;
        m_tourLen = 0;
        qint64 elapsed = timer.elapsed();
        m_stats.totalComputations++;
        m_stats.totalNodes += m_n;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computed(0, false);
        return tour;
    }

    /* 检查连通性: 确保有边的顶点是连通的 */
    if (start >= 0) {
        QVector<bool> visited(m_n, false);
        QVector<int> stack;
        stack.append(start);
        visited[start] = true;
        int visitedCount = 0;

        while (!stack.isEmpty()) {
            int v = stack.back();
            stack.removeLast();
            visitedCount++;
            for (const auto& edge : m_adj[v]) {
                if (!visited[edge.first]) {
                    visited[edge.first] = true;
                    stack.append(edge.first);
                }
            }
        }

        /* 统计有边的顶点数 */
        int activeVertices = 0;
        for (int i = 0; i < m_n; ++i) {
            if (degree[i] > 0) activeVertices++;
        }

        if (visitedCount != activeVertices) {
            m_hasTour = false;
            m_tourLen = 0;
            qint64 elapsed = timer.elapsed();
            m_stats.totalComputations++;
            m_stats.totalNodes += m_n;
            m_timeSum += elapsed;
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
            emit computed(0, false);
            return tour;
        }
    }

    m_hasTour = true;

    /* 阶段3: Hierholzer算法 */
    QVector<bool> usedEdges(edgeCount, false);
    findEulerTour(start, tour, usedEdges);

    m_tourLen = tour.size();

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalComputations++;
    m_stats.totalNodes += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_tourLen, m_hasTour);
    return tour;
}

/**
 * @brief 重置统计信息
 */
void EdgeColoring3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Hierholzer DFS搜索欧拉回路
 * @param start 起始顶点
 * @param tour 输出的欧拉回路
 * @param used 边使用标记
 *
 * 使用栈模拟递归DFS，每次找到未使用的边就继续深入。
 * 当顶点没有未使用的边时，将其加入路径。
 * 最后反转路径得到欧拉回路。
 */
void EdgeColoring3::findEulerTour(int start, QVector<int>& tour, QVector<bool>& used)
{
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();

        /* 查找一条未使用的边 */
        bool found = false;
        while (!m_adj[v].isEmpty()) {
            auto edge = m_adj[v].back();
            m_adj[v].pop_back();

            if (edge.second < used.size() && !used[edge.second]) {
                used[edge.second] = true;
                stack.append(edge.first);
                found = true;
                break;
            }
        }

        if (!found) {
            tour.append(v);
            stack.removeLast();
        }
    }

    /* 反转路径得到正确顺序 */
    std::reverse(tour.begin(), tour.end());
}
