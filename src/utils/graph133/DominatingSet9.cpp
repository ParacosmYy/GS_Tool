#include "DominatingSet9.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化支配集引擎v9
 * @param parent 父对象指针
 */
DominatingSet9::DominatingSet9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DominatingSet9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 贪心法求近似最小支配集
 *
 * 每次选择能新支配最多未支配顶点的顶点加入集合。
 * 时间复杂度O(V^2)，近似比为O(ln V)。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 支配集顶点索引
 */
QVector<int> DominatingSet9::greedyMDS(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> domSet;

    if (n == 0) {
        emit solveCompleted(0);
        return domSet;
    }

    QVector<bool> dominated(n, false);

    for (int step = 0; step < n; ++step) {
        /* 检查是否所有顶点已被支配 */
        bool allDominated = true;
        for (int i = 0; i < n; ++i) { if (!dominated[i]) { allDominated = false; break; } }
        if (allDominated) break;

        /* 选择能新支配最多顶点的顶点 */
        int best = -1;
        int bestGain = 0;

        for (int v = 0; v < n; ++v) {
            int gain = dominated[v] ? 0 : 1;
            for (int u = 0; u < qMin(n, adjacencyMatrix[v].size()); ++u) {
                if (adjacencyMatrix[v][u] != 0 && !dominated[u]) gain++;
            }
            if (gain > bestGain) {
                bestGain = gain;
                best = v;
            }
        }

        if (best < 0 || bestGain == 0) break;

        domSet.append(best);
        dominated[best] = true;
        for (int u = 0; u < qMin(n, adjacencyMatrix[best].size()); ++u) {
            if (adjacencyMatrix[best][u] != 0) dominated[u] = true;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(domSet.size());
    return domSet;
}

/**
 * @brief 求最小连通支配集
 *
 * 先求贪心支配集，然后通过添加最短路径上的顶点使其连通。
 * 最终结果保证既是支配集又是连通子图。
 *
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 连通支配集顶点索引
 */
QVector<int> DominatingSet9::connectedMDS(const QVector<QVector<int>>& adjacencyMatrix)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacencyMatrix.size();
    QVector<int> result;

    if (n == 0) {
        emit solveCompleted(0);
        return result;
    }

    /* 先求贪心支配集 */
    QVector<int> domSet = greedyMDS(adjacencyMatrix);

    if (domSet.size() <= 1) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        emit solveCompleted(domSet.size());
        return domSet;
    }

    /* 构建支配集内部的BFS连通 */
    QSet<int> inSet;
    for (int v : domSet) inSet.insert(v);

    result = domSet;

    /* 对支配集中不连通的顶对，找最短路径并添加中间顶点 */
    QVector<bool> connected(n, false);
    if (!result.isEmpty()) connected[result[0]] = true;

    bool changed = true;
    while (changed) {
        changed = false;
        for (int v : result) {
            if (connected[v]) continue;
            /* BFS找最近的已连通支配集顶点 */
            QVector<int> parent(n, -1);
            QVector<int> dist(n, -1);
            QSet<int> visited;

            /* 从所有已连通的支配集顶点开始BFS */
            QVector<int> queue;
            for (int u = 0; u < n; ++u) {
                if (connected[u] && inSet.contains(u)) {
                    queue.append(u);
                    dist[u] = 0;
                    parent[u] = u;
                }
            }

            int head = 0;
            while (head < queue.size()) {
                int u = queue[head++];
                for (int nb = 0; nb < qMin(n, adjacencyMatrix[u].size()); ++nb) {
                    if (adjacencyMatrix[u][nb] != 0 && dist[nb] < 0) {
                        dist[nb] = dist[u] + 1;
                        parent[nb] = u;
                        queue.append(nb);
                    }
                }
            }

            if (dist[v] >= 0) {
                /* 回溯路径并添加 */
                int cur = v;
                while (cur >= 0 && parent[cur] != cur) {
                    if (!connected[cur]) {
                        connected[cur] = true;
                        if (!inSet.contains(cur)) {
                            result.append(cur);
                            inSet.insert(cur);
                        }
                    }
                    cur = parent[cur];
                }
                connected[v] = true;
                changed = true;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolveOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolveOps;

    emit solveCompleted(result.size());
    return result;
}

/**
 * @brief 验证给定集合是否为支配集
 *
 * 检查每个顶点是否在集合中或与集合中某顶点相邻。
 *
 * @param candidateSet 候选集合
 * @param adjacencyMatrix 图的邻接矩阵
 * @return 是否为有效支配集
 */
bool DominatingSet9::isDominatingSet(const QVector<int>& candidateSet,
                                      const QVector<QVector<int>>& adjacencyMatrix) const
{
    const int n = adjacencyMatrix.size();
    if (n == 0) return candidateSet.isEmpty();

    QVector<bool> dominated(n, false);
    for (int v : candidateSet) {
        if (v < 0 || v >= n) return false;
        dominated[v] = true;
        for (int u = 0; u < qMin(n, adjacencyMatrix[v].size()); ++u) {
            if (adjacencyMatrix[v][u] != 0) dominated[u] = true;
        }
    }

    for (int i = 0; i < n; ++i) {
        if (!dominated[i]) return false;
    }
    return true;
}

/**
 * @brief 计算支配集的近似比
 * @param greedySize 贪心解大小
 * @param optimalSize 最优解大小
 * @return 近似比
 */
double DominatingSet9::approximationRatio(int greedySize, int optimalSize) const
{
    if (optimalSize <= 0) return 0.0;
    return static_cast<double>(greedySize) / optimalSize;
}
