#include "Biconnected4.h"
#include <QElapsedTimer>
#include <algorithm>
#include <stack>

/**
 * @brief 构造函数，初始化双连通分量检测器
 * @param parent 父QObject对象指针
 */
Biconnected4::Biconnected4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 查找图的所有双连通分量
 *
 * 基于Tarjan算法使用DFS查找双连通分量(BCC)：
 * 1. DFS遍历维护每个顶点的发现时间(disc)和
 *    通过该顶点可达的最小发现时间(low)
 * 2. 当low[child] >= disc[parent]时，parent是割点
 * 3. 使用栈记录DFS过程中的边，遇到割点时弹出栈中边
 *    直到当前边，形成双连通分量
 *
 * @param adjacency 邻接表表示的图
 * @return 所有双连通分量的边集列表
 */
QVector<QVector<int>> Biconnected4::findComponents(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacency.size();
    if (n == 0) return {};

    QVector<int> disc(n, -1);     ///< 发现时间，-1表示未访问
    QVector<int> low(n, 0);       ///< low值
    QVector<int> parent(n, -1);   ///< DFS树中的父节点
    QVector<bool> visited(n, false);

    QVector<QVector<int>> components;  ///< 结果：所有双连通分量
    std::stack<QPair<int, int>> edgeStack;  ///< 边栈

    int time = 0;

    /// 对每个未访问的顶点执行DFS
    for (int start = 0; start < n; ++start) {
        if (disc[start] != -1) continue;

        /// 迭代式DFS（使用栈模拟递归）
        struct DFSFrame {
            int u;
            int neighborIdx;
        };
        std::stack<DFSFrame> dfsStack;
        dfsStack.push({start, 0});
        disc[start] = low[start] = time++;
        parent[start] = -1;

        while (!dfsStack.empty()) {
            auto& frame = dfsStack.top();
            int u = frame.u;

            /// 遍历u的所有邻居
            bool pushed = false;
            while (frame.neighborIdx < adjacency[u].size()) {
                int v = adjacency[u][frame.neighborIdx];
                ++frame.neighborIdx;

                if (v < 0 || v >= n) continue;

                if (disc[v] == -1) {
                    /// v未访问，树边
                    parent[v] = u;
                    disc[v] = low[v] = time++;
                    edgeStack.push(qMakePair(u, v));
                    dfsStack.push({v, 0});
                    pushed = true;
                    break;
                } else if (v != parent[u] && disc[v] < disc[u]) {
                    /// 回边（到祖先）
                    edgeStack.push(qMakePair(u, v));
                    low[u] = qMin(low[u], disc[v]);
                }
            }

            if (pushed) continue;

            /// u的所有邻居已处理完毕，回溯
            dfsStack.pop();

            if (!dfsStack.empty()) {
                int p = dfsStack.top().u;  ///< parent of u
                low[p] = qMin(low[p], low[u]);

                /// 检查p是否为割点
                if ((parent[p] == -1 && adjacency[p].size() > 1) ||
                    (parent[p] != -1 && low[u] >= disc[p])) {
                    /// 弹出栈中边直到(u, p)形成双连通分量
                    QVector<int> component;
                    while (!edgeStack.empty()) {
                        auto edge = edgeStack.top();
                        edgeStack.pop();
                        component.append(edge.first);
                        component.append(edge.second);
                        if ((edge.first == p && edge.second == u) ||
                            (edge.first == u && edge.second == p)) break;
                    }
                    if (!component.isEmpty()) {
                        components.append(component);
                        emit componentFound(components.size() - 1, component.size() / 2);
                    }
                }
            }
        }
    }

    /// 更新统计信息
    m_stats.totalComponentsFound += components.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalComponentsFound);

    return components;
}

/**
 * @brief 查找所有割点（关节点）
 *
 * 割点是删除后使图不连通的顶点。判定条件：
 * 1. 根节点有>=2个子树 -> 是割点
 * 2. 非根节点u，若存在子节点v使得low[v] >= disc[u] -> 是割点
 *
 * @param adjacency 邻接表
 * @return 割点索引列表
 */
QVector<int> Biconnected4::findArticulationPoints(const QVector<QVector<int>>& adjacency) const
{
    const int n = adjacency.size();
    if (n == 0) return {};

    QVector<int> disc(n, -1), low(n, 0), parent(n, -1);
    QVector<bool> isArticulation(n, false);
    int time = 0;

    /// 递归DFS辅助函数
    std::function<void(int)> dfs = [&](int u) {
        int children = 0;
        disc[u] = low[u] = time++;

        for (int v : adjacency[u]) {
            if (v < 0 || v >= n) continue;
            if (disc[v] == -1) {
                ++children;
                parent[v] = u;
                dfs(v);
                low[u] = qMin(low[u], low[v]);

                /// 割点判定
                if (parent[u] == -1 && children > 1) isArticulation[u] = true;
                if (parent[u] != -1 && low[v] >= disc[u]) isArticulation[u] = true;
            } else if (v != parent[u]) {
                low[u] = qMin(low[u], disc[v]);
            }
        }
    };

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) dfs(i);
    }

    QVector<int> articulations;
    for (int i = 0; i < n; ++i) {
        if (isArticulation[i]) articulations.append(i);
    }

    return articulations;
}

/**
 * @brief 获取当前统计数据
 * @return 包含分量数、割点数和平均耗时的Stats结构
 */
Biconnected4::Stats Biconnected4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void Biconnected4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
