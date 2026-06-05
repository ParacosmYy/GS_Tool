#include "Biconnected8.h"
#include <QElapsedTimer>
#include <QStack>
#include <algorithm>

/**
 * @brief 构造函数，初始化双连通分量引擎v8
 * @param parent 父对象指针
 */
Biconnected8::Biconnected8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Biconnected8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 查找图中所有割点（关节点）
 *
 * 基于DFS遍历，维护disc和low数组。
 * 根节点有>=2个子树或非根节点u存在子节点v使得low[v]>=disc[u]则为割点。
 * 时间复杂度O(V+E)。
 *
 * @param adjacencyList 图的邻接表
 * @return 割点索引集合
 */
QVector<int> Biconnected8::findArticulationPoints(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    QVector<int> artPoints;

    if (n == 0) {
        emit solveCompleted(0);
        return artPoints;
    }

    QVector<int> disc(n, -1), low(n, 0);
    QVector<bool> isArt(n, false);
    int time = 0;

    std::function<void(int, int)> dfs = [&](int u, int parent) {
        disc[u] = low[u] = time++;
        int children = 0;

        for (int v : adjacencyList[u]) {
            if (v < 0 || v >= n) continue;
            if (disc[v] == -1) {
                children++;
                dfs(v, u);
                low[u] = qMin(low[u], low[v]);

                if (parent == -1 && children > 1) isArt[u] = true;
                if (parent != -1 && low[v] >= disc[u]) isArt[u] = true;
            } else if (v != parent) {
                low[u] = qMin(low[u], disc[v]);
            }
        }
    };

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) dfs(i, -1);
    }

    for (int i = 0; i < n; ++i) {
        if (isArt[i]) artPoints.append(i);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(artPoints.size());
    return artPoints;
}

/**
 * @brief 查找图中所有桥（割边）
 *
 * 边(u,v)是桥当且仅当low[v]>disc[u]。
 * 时间复杂度O(V+E)。
 *
 * @param adjacencyList 图的邻接表
 * @return 桥的端点对列表
 */
QVector<QPair<int, int>> Biconnected8::findBridges(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    QVector<QPair<int, int>> bridges;

    if (n == 0) {
        emit solveCompleted(0);
        return bridges;
    }

    QVector<int> disc(n, -1), low(n, 0);
    int time = 0;

    std::function<void(int, int)> dfs = [&](int u, int parent) {
        disc[u] = low[u] = time++;

        for (int v : adjacencyList[u]) {
            if (v < 0 || v >= n) continue;
            if (disc[v] == -1) {
                dfs(v, u);
                low[u] = qMin(low[u], low[v]);

                /* 桥的条件：low[v] > disc[u] */
                if (low[v] > disc[u]) {
                    bridges.append({qMin(u, v), qMax(u, v)});
                }
            } else if (v != parent) {
                low[u] = qMin(low[u], disc[v]);
            }
        }
    };

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) dfs(i, -1);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(bridges.size());
    return bridges;
}

/**
 * @brief 计算双连通分量
 *
 * 使用DFS遍历配合边栈，当发现割点时弹出栈中边作为一个双连通分量。
 * 时间复杂度O(V+E)。
 *
 * @param adjacencyList 图的邻接表
 * @return 各双连通分量包含的边列表
 */
QVector<QVector<QPair<int, int>>> Biconnected8::biconnectedComponents(
    const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    QVector<QVector<QPair<int, int>>> components;

    if (n == 0) {
        emit solveCompleted(0);
        return components;
    }

    QVector<int> disc(n, -1), low(n, 0);
    QStack<QPair<int, int>> edgeStack;
    int time = 0;

    std::function<void(int, int)> dfs = [&](int u, int parent) {
        disc[u] = low[u] = time++;
        int children = 0;

        for (int v : adjacencyList[u]) {
            if (v < 0 || v >= n) continue;

            if (disc[v] == -1) {
                children++;
                edgeStack.push({u, v});
                dfs(v, u);
                low[u] = qMin(low[u], low[v]);

                /* 如果u是割点，弹出一个双连通分量 */
                if ((parent == -1 && children > 1) ||
                    (parent != -1 && low[v] >= disc[u])) {
                    QVector<QPair<int, int>> comp;
                    while (!edgeStack.empty()) {
                        auto e = edgeStack.top();
                        edgeStack.pop();
                        comp.append(e);
                        if ((e.first == u && e.second == v) ||
                            (e.first == v && e.second == u)) break;
                    }
                    components.append(comp);
                }
            } else if (v != parent && disc[v] < disc[u]) {
                low[u] = qMin(low[u], disc[v]);
                edgeStack.push({u, v});
            }
        }
    };

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) {
            dfs(i, -1);
            if (!edgeStack.empty()) {
                QVector<QPair<int, int>> comp;
                while (!edgeStack.empty()) {
                    comp.append(edgeStack.top());
                    edgeStack.pop();
                }
                components.append(comp);
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(components.size());
    return components;
}

/**
 * @brief 检查图是否为双连通图
 *
 * 双连通图没有割点且连通。
 *
 * @param adjacencyList 图的邻接表
 * @return 是否双连通
 */
bool Biconnected8::isBiconnected(const QVector<QVector<int>>& adjacencyList) const
{
    const int n = adjacencyList.size();
    if (n <= 1) return true;

    /* 检查连通性 */
    QVector<bool> visited(n, false);
    QVector<int> queue = {0};
    visited[0] = true;
    int head = 0;

    while (head < queue.size()) {
        int u = queue[head++];
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n && !visited[v]) {
                visited[v] = true;
                queue.append(v);
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) return false;
    }

    /* 检查无割点 */
    QVector<int> disc(n, -1), low(n, 0);
    int time = 0;

    std::function<bool(int, int)> dfs = [&](int u, int parent) -> bool {
        disc[u] = low[u] = time++;
        int children = 0;

        for (int v : adjacencyList[u]) {
            if (v < 0 || v >= n) continue;
            if (disc[v] == -1) {
                children++;
                if (!dfs(v, u)) return false;
                low[u] = qMin(low[u], low[v]);

                if (parent == -1 && children > 1) return false;
                if (parent != -1 && low[v] >= disc[u]) return false;
            } else if (v != parent) {
                low[u] = qMin(low[u], disc[v]);
            }
        }
        return true;
    };

    return dfs(0, -1);
}
