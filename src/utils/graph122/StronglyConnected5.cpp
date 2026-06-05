#include "StronglyConnected5.h"
#include <QElapsedTimer>
#include <QSet>
#include <algorithm>

/**
 * @brief 构造函数，初始化强连通分量求解器
 * @param parent 父对象指针
 */
StronglyConnected5::StronglyConnected5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void StronglyConnected5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Tarjan算法求强连通分量
 *
 * 基于DFS的Tarjan算法，使用disc(发现时间)和low(可回溯的最早祖先)数组。
 * 当low[v] == disc[v]时，从栈中弹出该SCC的所有节点。
 * 时间复杂度O(V+E)。
 *
 * @param adjacencyList 邻接表表示的有向图
 * @return 每个顶点的分量编号
 */
QVector<int> StronglyConnected5::tarjan(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    if (n == 0) {
        emit searchCompleted(0);
        return {};
    }

    QVector<int> disc(n, -1);   /* 发现时间，-1表示未访问 */
    QVector<int> low(n, 0);     /* 可回溯的最早祖先 */
    QVector<bool> onStack(n, false);
    QVector<int> componentId(n, -1);
    QVector<int> stack;
    int timer_count = 0;
    int compId = 0;

    /* 递归DFS函数 */
    std::function<void(int)> dfs = [&](int u) {
        disc[u] = low[u] = timer_count++;
        stack.push_back(u);
        onStack[u] = true;

        for (int v : adjacencyList[u]) {
            if (v < 0 || v >= n) continue;
            if (disc[v] == -1) {
                /* 未访问的邻居 */
                dfs(v);
                low[u] = qMin(low[u], low[v]);
            } else if (onStack[v]) {
                /* 在栈中的邻居（回边） */
                low[u] = qMin(low[u], disc[v]);
            }
        }

        /* 如果u是SCC的根节点 */
        if (low[u] == disc[u]) {
            while (true) {
                int w = stack.back();
                stack.pop_back();
                onStack[w] = false;
                componentId[w] = compId;
                if (w == u) break;
            }
            compId++;
        }
    };

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) dfs(i);
    }

    m_componentCount = compId;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearches++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
    m_stats.componentCount = compId;

    emit searchCompleted(compId);
    return componentId;
}

/**
 * @brief Kosaraju算法求强连通分量
 *
 * 两遍DFS：
 * 1. 对原图执行DFS，按完成时间记录顺序
 * 2. 构建转置图，按逆完成时间顺序执行DFS
 *
 * @param adjacencyList 邻接表
 * @return 每个顶点的分量编号
 */
QVector<int> StronglyConnected5::kosaraju(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    if (n == 0) {
        emit searchCompleted(0);
        return {};
    }

    /* 第一遍DFS：计算完成顺序 */
    QVector<bool> visited(n, false);
    QVector<int> finishOrder;

    std::function<void(int)> dfs1 = [&](int u) {
        visited[u] = true;
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n && !visited[v]) dfs1(v);
        }
        finishOrder.push_back(u);
    };

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) dfs1(i);
    }

    /* 构建转置图 */
    QVector<QVector<int>> transpose(n);
    for (int u = 0; u < n; ++u) {
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n) {
                transpose[v].push_back(u);
            }
        }
    }

    /* 第二遍DFS：按逆完成顺序 */
    QVector<int> componentId(n, -1);
    int compId = 0;
    visited.assign(n, false);

    std::function<void(int, int)> dfs2 = [&](int u, int cid) {
        visited[u] = true;
        componentId[u] = cid;
        for (int v : transpose[u]) {
            if (!visited[v]) dfs2(v, cid);
        }
    };

    for (int i = n - 1; i >= 0; --i) {
        int u = finishOrder[i];
        if (!visited[u]) {
            dfs2(u, compId++);
        }
    }

    m_componentCount = compId;
    m_stats.componentCount = compId;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSearches++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(compId);
    return componentId;
}

/**
 * @brief 构建缩图
 * @param adjacencyList 原图邻接表
 * @param componentIds 分量编号
 * @return 缩图的邻接表
 */
QVector<QVector<int>> StronglyConnected5::condensationGraph(
    const QVector<QVector<int>>& adjacencyList, const QVector<int>& componentIds) const
{
    int maxComp = 0;
    for (int c : componentIds) maxComp = qMax(maxComp, c);
    maxComp++;

    QSet<QPair<int, int>> edgeSet;
    QVector<QVector<int>> result(maxComp);

    for (int u = 0; u < adjacencyList.size(); ++u) {
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < componentIds.size()) {
                int cu = componentIds[u];
                int cv = componentIds[v];
                if (cu != cv && !edgeSet.contains({cu, cv})) {
                    edgeSet.insert({cu, cv});
                    result[cu].append(cv);
                }
            }
        }
    }
    return result;
}

/**
 * @brief 检查图是否为强连通图
 */
bool StronglyConnected5::isStronglyConnected(const QVector<QVector<int>>& adjacencyList) const
{
    if (adjacencyList.isEmpty()) return true;
    /* 简单检查：从节点0出发的DFS能否到达所有节点 */
    int n = adjacencyList.size();
    QVector<bool> visited(n, false);
    QVector<int> stack = {0};
    visited[0] = true;
    int count = 1;

    while (!stack.isEmpty()) {
        int u = stack.back();
        stack.pop_back();
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n && !visited[v]) {
                visited[v] = true;
                count++;
                stack.push_back(v);
            }
        }
    }
    return count == n;
}
