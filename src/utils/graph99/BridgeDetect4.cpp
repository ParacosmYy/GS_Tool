#include "BridgeDetect4.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @class BridgeDetect4
 * @brief 桥边检测器实现
 *
 * 基于Tarjan算法检测无向图中的桥边(割边)。
 * 桥边是删除后会增加图连通分量数的边。
 *
 * 算法使用DFS遍历，维护每个节点的发现时间(disc)和
 * 通过回边可达的最早祖先时间(low)。若 low[v] > disc[u]，
 * 则边(u,v)为桥边。时间复杂度O(V+E)。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
BridgeDetect4::BridgeDetect4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 查找无向图中所有桥边
 *
 * 执行Tarjan DFS，对每个未访问节点递归计算disc和low值。
 * 当子节点的low值严格大于父节点的disc值时，该边为桥边。
 *
 * @param adjacency 邻接表，adjacency[u]包含u的所有邻居
 * @return 所有桥边的列表，每对(u,v)表示一条桥边
 */
QVector<QPair<int, int>> BridgeDetect4::findBridges(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, int>> bridges;
    int n = adjacency.size();
    if (n == 0) {
        return bridges;
    }

    /* DFS状态数组 */
    QVector<int> disc(n, -1);   /* 发现时间，-1表示未访问 */
    QVector<int> low(n, -1);    /* 最早可达祖先时间 */
    QVector<int> parent(n, -1); /* DFS树中的父节点 */
    int time = 0;

    /* Tarjan DFS内部递归函数 */
    std::function<void(int)> dfs = [&](int u) {
        disc[u] = low[u] = time++;

        for (int v : adjacency[u]) {
            if (disc[v] == -1) {
                /* v未访问，(u,v)是树边 */
                parent[v] = u;
                dfs(v);

                /* 更新u的low值 */
                low[u] = qMin(low[u], low[v]);

                /* 判断桥边: v无法通过其他路径到达u的祖先 */
                if (low[v] > disc[u]) {
                    bridges.append({qMin(u, v), qMax(u, v)});
                    m_stats.totalBridgesFound++;
                    emit bridgeFound(qMin(u, v), qMax(u, v));
                }
            } else if (v != parent[u]) {
                /* v已访问且非父节点，(u,v)是回边 */
                low[u] = qMin(low[u], disc[v]);
            }
        }
    };

    /* 对所有连通分量执行DFS */
    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) {
            dfs(i);
        }
    }

    m_stats.totalGraphsScanned++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalGraphsScanned);

    return bridges;
}

/**
 * @brief 检查指定边是否为桥边
 *
 * 通过临时删除该边后检查连通性来判断。也可直接调用
 * findBridges()后在结果中查找。
 *
 * @param adjacency 邻接表
 * @param u 边的端点之一
 * @param v 边的另一端点
 * @return true如果(u,v)是桥边
 */
bool BridgeDetect4::isBridge(const QVector<QVector<int>>& adjacency, int u, int v) const
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(timer)

    int n = adjacency.size();
    if (u < 0 || u >= n || v < 0 || v >= n) {
        return false;
    }

    /* 使用BFS检查删除(u,v)后u和v是否仍连通 */
    QVector<bool> visited(n, false);
    QVector<int> queue;
    queue.append(u);
    visited[u] = true;

    while (!queue.isEmpty()) {
        int cur = queue.takeFirst();
        for (int neighbor : adjacency[cur]) {
            /* 跳过被删除的边 */
            if ((cur == u && neighbor == v) || (cur == v && neighbor == u)) {
                continue;
            }
            if (!visited[neighbor]) {
                if (neighbor == v) {
                    return false; /* 仍连通，非桥边 */
                }
                visited[neighbor] = true;
                queue.append(neighbor);
            }
        }
    }

    return true; /* 不连通，是桥边 */
}

/**
 * @brief 重置所有统计数据
 *
 * 将桥边计数、图扫描计数和计时归零。
 */
void BridgeDetect4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
