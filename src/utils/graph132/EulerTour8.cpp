#include "EulerTour8.h"
#include <QElapsedTimer>
#include <QStack>
#include <algorithm>

/**
 * @brief 构造函数，初始化欧拉回路引擎v8
 * @param parent 父对象指针
 */
EulerTour8::EulerTour8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void EulerTour8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Hierholzer算法构造欧拉回路
 *
 * 从任意顶点出发DFS，将无出边的顶点加入路径，最终反转。
 * 要求所有顶点度数为偶数。
 * 时间复杂度O(V+E)。
 *
 * @param adjacencyList 图的邻接表
 * @return 欧拉回路的顶点序列，不存在则返回空
 */
QVector<int> EulerTour8::hierholzerCircuit(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    QVector<int> circuit;

    if (n == 0) {
        emit solveCompleted(0);
        return circuit;
    }

    /* 检查是否存在欧拉回路（所有度数为偶数） */
    if (!hasEulerCircuit(adjacencyList)) {
        emit solveCompleted(0);
        return {};
    }

    /* 找到一个非孤立顶点作为起点 */
    int start = 0;
    for (int i = 0; i < n; ++i) {
        if (!adjacencyList[i].isEmpty()) { start = i; break; }
    }

    /* 使用可变邻接表（删除已访问的边） */
    QVector<QVector<int>> adj = adjacencyList;
    QVector<int> ptr(n, 0); /* 每个顶点的邻接表指针 */

    QStack<int> stack;
    stack.push(start);

    while (!stack.empty()) {
        int u = stack.top();
        if (ptr[u] < adj[u].size()) {
            int v = adj[u][ptr[u]];
            ptr[u]++;
            stack.push(v);
        } else {
            circuit.append(u);
            stack.pop();
        }
    }

    /* 反转得到欧拉回路 */
    std::reverse(circuit.begin(), circuit.end());

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(circuit.size());
    return circuit;
}

/**
 * @brief 构造欧拉路径
 *
 * 如果恰好有两个奇数度顶点，则存在欧拉路径。
 * 添加虚拟边将其转为欧拉回路问题。
 *
 * @param adjacencyList 图的邻接表
 * @return 欧拉路径的顶点序列，不存在则返回空
 */
QVector<int> EulerTour8::eulerPath(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();

    if (n == 0) {
        emit solveCompleted(0);
        return {};
    }

    /* 统计奇数度顶点 */
    QVector<int> oddVertices;
    for (int i = 0; i < n; ++i) {
        int deg = adjacencyList[i].size();
        if (deg % 2 != 0) oddVertices.append(i);
    }

    /* 欧拉路径恰好有0或2个奇数度顶点 */
    if (oddVertices.size() != 2 && oddVertices.size() != 0) {
        emit solveCompleted(0);
        return {};
    }

    /* 如果有2个奇数度顶点，添加虚拟边 */
    QVector<QVector<int>> adj = adjacencyList;
    int start = 0;
    if (oddVertices.size() == 2) {
        adj[oddVertices[0]].append(oddVertices[1]);
        adj[oddVertices[1]].append(oddVertices[0]);
        start = oddVertices[0];
    } else {
        for (int i = 0; i < n; ++i) {
            if (!adj[i].isEmpty()) { start = i; break; }
        }
    }

    /* Hierholzer算法 */
    QVector<int> ptr(n, 0);
    QStack<int> stack;
    QVector<int> path;
    stack.push(start);

    while (!stack.empty()) {
        int u = stack.top();
        if (ptr[u] < adj[u].size()) {
            int v = adj[u][ptr[u]];
            ptr[u]++;
            stack.push(v);
        } else {
            path.append(u);
            stack.pop();
        }
    }

    std::reverse(path.begin(), path.end());

    /* 如果添加了虚拟边，移除第一步 */
    if (oddVertices.size() == 2 && path.size() > 1) {
        path.removeFirst();
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(path.size());
    return path;
}

/**
 * @brief 检查图是否存在欧拉回路
 *
 * 无向图：所有顶点度数为偶数且图连通。
 * 有向图：所有顶点入度等于出度且图弱连通。
 *
 * @param adjacencyList 图的邻接表
 * @param isDirected 是否为有向图
 * @return 是否存在欧拉回路
 */
bool EulerTour8::hasEulerCircuit(const QVector<QVector<int>>& adjacencyList,
                                  bool isDirected) const
{
    const int n = adjacencyList.size();
    if (n == 0) return true;

    if (!isDirected) {
        /* 无向图：所有度数为偶数 */
        for (int i = 0; i < n; ++i) {
            if (adjacencyList[i].size() % 2 != 0) return false;
        }
    } else {
        /* 有向图：入度等于出度 */
        QVector<int> inDeg(n, 0), outDeg(n, 0);
        for (int u = 0; u < n; ++u) {
            outDeg[u] = adjacencyList[u].size();
            for (int v : adjacencyList[u]) {
                if (v >= 0 && v < n) inDeg[v]++;
            }
        }
        for (int i = 0; i < n; ++i) {
            if (inDeg[i] != outDeg[i]) return false;
        }
    }
    return true;
}

/**
 * @brief 计算各顶点的度数
 *
 * 无向图返回度数，有向图返回出度-入度差。
 *
 * @param adjacencyList 图的邻接表
 * @param isDirected 是否为有向图
 * @return 各顶点的度数
 */
QVector<int> EulerTour8::computeDegrees(const QVector<QVector<int>>& adjacencyList,
                                         bool isDirected) const
{
    const int n = adjacencyList.size();
    if (n == 0) return {};

    if (!isDirected) {
        QVector<int> degrees(n);
        for (int i = 0; i < n; ++i) {
            degrees[i] = adjacencyList[i].size();
        }
        return degrees;
    }

    /* 有向图：返回出度-入度差 */
    QVector<int> balance(n, 0);
    for (int u = 0; u < n; ++u) {
        balance[u] += adjacencyList[u].size();
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n) balance[v]--;
        }
    }
    return balance;
}
