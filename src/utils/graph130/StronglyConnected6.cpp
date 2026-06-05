#include "StronglyConnected6.h"
#include <QElapsedTimer>
#include <QStack>
#include <algorithm>

/**
 * @brief 构造函数，初始化强连通分量引擎
 * @param parent 父对象指针
 */
StronglyConnected6::StronglyConnected6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void StronglyConnected6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Kosaraju算法求强连通分量
 *
 * 第一遍DFS获取逆后序，第二遍在转置图上按逆后序DFS。
 * 时间复杂度O(V+E)。
 *
 * @param adjacencyList 有向图邻接表
 * @return 各顶点所属的SCC编号
 */
QVector<int> StronglyConnected6::kosaraju(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    QVector<int> labels(n, -1);

    if (n == 0) {
        emit solveCompleted(0);
        return labels;
    }

    /* 第一遍DFS：获取逆后序 */
    QVector<bool> visited(n, false);
    QVector<int> order;

    std::function<void(int)> dfs1 = [&](int u) {
        visited[u] = true;
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n && !visited[v]) dfs1(v);
        }
        order.append(u);
    };

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) dfs1(i);
    }

    /* 构建转置图 */
    QVector<QVector<int>> transpose(n);
    for (int u = 0; u < n; ++u) {
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n) transpose[v].append(u);
        }
    }

    /* 第二遍DFS：在转置图上按逆后序遍历 */
    visited.fill(false);
    int sccId = 0;

    std::function<void(int)> dfs2 = [&](int u) {
        visited[u] = true;
        labels[u] = sccId;
        for (int v : transpose[u]) {
            if (v >= 0 && v < n && !visited[v]) dfs2(v);
        }
    };

    for (int i = order.size() - 1; i >= 0; --i) {
        if (!visited[order[i]]) {
            dfs2(order[i]);
            sccId++;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(sccId);
    return labels;
}

/**
 * @brief Tarjan算法求强连通分量
 *
 * 使用DFS遍历，维护disc和low数组。
 * 当low[u]==disc[u]时弹栈得到一个SCC。
 * 时间复杂度O(V+E)。
 *
 * @param adjacencyList 有向图邻接表
 * @return 各顶点所属的SCC编号
 */
QVector<int> StronglyConnected6::tarjan(const QVector<QVector<int>>& adjacencyList)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyList.size();
    QVector<int> labels(n, -1);

    if (n == 0) {
        emit solveCompleted(0);
        return labels;
    }

    QVector<int> disc(n, -1), low(n, 0);
    QStack<int> stk;
    QVector<bool> onStack(n, false);
    int time = 0;
    int sccId = 0;

    std::function<void(int)> dfs = [&](int u) {
        disc[u] = low[u] = time++;
        stk.push(u);
        onStack[u] = true;

        for (int v : adjacencyList[u]) {
            if (v < 0 || v >= n) continue;
            if (disc[v] == -1) {
                dfs(v);
                low[u] = qMin(low[u], low[v]);
            } else if (onStack[v]) {
                low[u] = qMin(low[u], disc[v]);
            }
        }

        /* 找到SCC根节点 */
        if (low[u] == disc[u]) {
            while (true) {
                int v = stk.top();
                stk.pop();
                onStack[v] = false;
                labels[v] = sccId;
                if (v == u) break;
            }
            sccId++;
        }
    };

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) dfs(i);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(sccId);
    return labels;
}

/**
 * @brief 构建SCC缩图
 *
 * 将每个SCC缩为一个超级顶点，构建DAG。
 * 跨SCC的边保留为超级顶点之间的有向边。
 *
 * @param adjacencyList 原图邻接表
 * @param sccLabels 各顶点的SCC编号
 * @return 缩图后的DAG邻接表
 */
QVector<QVector<int>> StronglyConnected6::condensationGraph(
    const QVector<QVector<int>>& adjacencyList, const QVector<int>& sccLabels)
{
    const int n = adjacencyList.size();
    if (n == 0 || sccLabels.size() != n) return {};

    /* 确定SCC数量 */
    int maxScc = 0;
    for (int s : sccLabels) { if (s + 1 > maxScc) maxScc = s + 1; }

    QVector<QVector<int>> dag(maxScc);

    for (int u = 0; u < n; ++u) {
        for (int v : adjacencyList[u]) {
            if (v >= 0 && v < n && sccLabels[u] != sccLabels[v]) {
                int su = sccLabels[u], sv = sccLabels[v];
                if (su >= 0 && su < maxScc && sv >= 0 && sv < maxScc) {
                    if (!dag[su].contains(sv)) dag[su].append(sv);
                }
            }
        }
    }

    return dag;
}

/**
 * @brief 对SCC缩图进行拓扑排序
 *
 * 使用Kahn算法（BFS）对DAG进行拓扑排序。
 *
 * @param dag 缩图DAG邻接表
 * @return 拓扑排序结果
 */
QVector<int> StronglyConnected6::topologicalSort(const QVector<QVector<int>>& dag)
{
    const int n = dag.size();
    QVector<int> inDegree(n, 0);

    for (int u = 0; u < n; ++u) {
        for (int v : dag[u]) {
            if (v >= 0 && v < n) inDegree[v]++;
        }
    }

    QVector<int> result;
    QVector<int> queue;
    for (int i = 0; i < n; ++i) {
        if (inDegree[i] == 0) queue.append(i);
    }

    int head = 0;
    while (head < queue.size()) {
        int u = queue[head++];
        result.append(u);
        for (int v : dag[u]) {
            if (v >= 0 && v < n) {
                inDegree[v]--;
                if (inDegree[v] == 0) queue.append(v);
            }
        }
    }

    return result;
}
