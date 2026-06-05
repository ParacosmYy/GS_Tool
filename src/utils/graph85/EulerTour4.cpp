/**
 * @file EulerTour4.cpp
 * @brief 欧拉回路/路径算法实现
 *
 * 实现Hierholzer算法计算欧拉回路和欧拉路径，
 * 支持有根和无根图模式。
 */

#include "utils/graph85/EulerTour4.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
EulerTour4::EulerTour4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置是否有根
 * @param rooted true=从指定起点出发，false=自动选择起点
 */
void EulerTour4::setRooted(bool rooted)
{
    m_rooted = rooted;
}

/**
 * @brief 设置顶点数量
 * @param n 顶点数
 */
void EulerTour4::setVertexCount(int n)
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
 */
void EulerTour4::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;

    // 每条无向边用两个有向半边表示，共享相同的边编号
    int edgeId = 0;
    for (const auto& adj : m_adj) edgeId += adj.size();
    edgeId /= 2; // 当前边数

    m_adj[u].append({v, edgeId});
    m_adj[v].append({u, edgeId});
}

/**
 * @brief 计算欧拉回路/路径
 * @return 欧拉回路/路径的顶点序列
 */
QVector<int> EulerTour4::compute()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> tour;
    if (m_n == 0) {
        m_hasTour = false;
        m_tourLen = 0;
        return tour;
    }

    // 统计度数
    QVector<int> degree(m_n, 0);
    int edgeCount = 0;
    for (int u = 0; u < m_n; ++u) {
        degree[u] = m_adj[u].size();
        edgeCount += degree[u];
    }
    edgeCount /= 2;

    // 检查连通性（简化：仅检查非孤立点的度数奇偶性）
    int oddDegree = 0;
    int start = 0;
    for (int i = 0; i < m_n; ++i) {
        if (degree[i] % 2 != 0) {
            oddDegree++;
            if (start == 0 && degree[i] > 0) start = i;
        } else if (degree[i] > 0 && start == 0) {
            start = i;
        }
    }

    // 欧拉回路条件：所有顶点度数为偶数
    // 欧拉路径条件：恰好两个顶点度数为奇数
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

    m_hasTour = true;

    // Hierholzer算法
    QVector<bool> usedEdges(edgeCount, false);

    findEulerTour(start, tour, usedEdges);

    m_tourLen = tour.size();

    // 更新统计信息
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
void EulerTour4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Hierholzer DFS搜索欧拉回路
 * @param start 起始顶点
 * @param tour 输出的欧拉回路
 * @param used 边使用标记
 */
void EulerTour4::findEulerTour(int start, QVector<int>& tour, QVector<bool>& used)
{
    // 使用栈模拟DFS
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();

        // 找到一条未使用的边
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

    // 反转路径
    std::reverse(tour.begin(), tour.end());
}
