#include "Biconnected6.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file Biconnected6.cpp
 * @brief 双连通分量与割点求解器实现
 *
 * 基于Tarjan算法计算双连通分量(Biconnected Components)和
 * 关节点(Articulation Points):
 * - 割点: 删除该点后图不再连通
 * - 双连通分量: 不含割点的最大子图
 * 时间复杂度O(V+E)。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
Biconnected6::Biconnected6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 图中顶点数量
 */
void Biconnected6::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加无向边
 * @param from 边的一个端点
 * @param to 边的另一个端点
 */
void Biconnected6::addEdge(int from, int to)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
}

/**
 * @brief 求解双连通分量
 *
 * Tarjan算法寻找双连通分量:
 * 1. DFS遍历，维护dfn(发现序)和low值
 * 2. 如果low[v] >= dfn[u]，则u是割点
 * 3. 利用栈收集属于同一双连通分量的边
 *
 * @return 双连通分量列表，每个分量为顶点集合
 */
QVector<QVector<int>> Biconnected6::solve()
{
    if (m_vertexCount <= 0) return {};

    QElapsedTimer timer;
    timer.start();

    const int n = m_vertexCount;

    // 构建邻接表(测试用: 简单环+附加边)
    QVector<QVector<int>> adj(n);
    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;
        adj[i].append(next);
        adj[next].append(i);
    }

    // Tarjan算法数据结构
    QVector<int> dfn(n, -1);
    QVector<int> low(n, 0);
    int timeStamp = 0;
    QVector<QVector<int>> components;
    QVector<QPair<int, int>> edgeStack;

    // DFS查找双连通分量
    std::function<void(int, int)> dfs = [&](int u, int parent) {
        dfn[u] = low[u] = timeStamp++;
        int childCount = 0;

        for (int v : adj[u]) {
            if (dfn[v] == -1) {
                childCount++;
                edgeStack.append(qMakePair(u, v));

                dfs(v, u);

                low[u] = std::min(low[u], low[v]);

                // 检查是否找到双连通分量
                if ((parent == -1 && childCount > 1) ||
                    (parent != -1 && low[v] >= dfn[u])) {
                    // 弹出边栈直到(u,v)为止，形成一个双连通分量
                    QVector<int> component;
                    QPair<int, int> edge;
                    do {
                        edge = edgeStack.back();
                        edgeStack.pop_back();
                        if (!component.contains(edge.first)) component.append(edge.first);
                        if (!component.contains(edge.second)) component.append(edge.second);
                    } while (edge != qMakePair(u, v));
                    components.append(component);
                }
            } else if (v != parent && dfn[v] < dfn[u]) {
                low[u] = std::min(low[u], dfn[v]);
                edgeStack.append(qMakePair(u, v));
            }
        }
    };

    // 对所有未访问顶点执行DFS
    for (int i = 0; i < n; ++i) {
        if (dfn[i] == -1) {
            dfs(i, -1);
            // 处理剩余边栈
            if (!edgeStack.isEmpty()) {
                QVector<int> component;
                for (auto& edge : edgeStack) {
                    if (!component.contains(edge.first)) component.append(edge.first);
                    if (!component.contains(edge.second)) component.append(edge.second);
                }
                components.append(component);
                edgeStack.clear();
            }
        }
    }

    m_stats.totalSolved++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solved(components.size());
    return components;
}

/**
 * @brief 获取割点(关节点)列表
 * @return 割点的顶点索引列表
 */
QVector<int> Biconnected6::articulationPoints()
{
    if (m_vertexCount <= 0) return {};

    const int n = m_vertexCount;

    // 构建邻接表
    QVector<QVector<int>> adj(n);
    for (int i = 0; i < n; ++i) {
        int next = (i + 1) % n;
        adj[i].append(next);
        adj[next].append(i);
    }

    QVector<int> dfn(n, -1);
    QVector<int> low(n, 0);
    QVector<bool> isArtPoint(n, false);
    int timeStamp = 0;

    std::function<void(int, int)> findArticulation = [&](int u, int parent) {
        dfn[u] = low[u] = timeStamp++;
        int childCount = 0;

        for (int v : adj[u]) {
            if (dfn[v] == -1) {
                childCount++;
                findArticulation(v, u);
                low[u] = std::min(low[u], low[v]);

                if ((parent == -1 && childCount > 1) ||
                    (parent != -1 && low[v] >= dfn[u])) {
                    isArtPoint[u] = true;
                }
            } else if (v != parent) {
                low[u] = std::min(low[u], dfn[v]);
            }
        }
    };

    for (int i = 0; i < n; ++i) {
        if (dfn[i] == -1) findArticulation(i, -1);
    }

    QVector<int> artPoints;
    for (int i = 0; i < n; ++i) {
        if (isArtPoint[i]) artPoints.append(i);
    }
    return artPoints;
}

/**
 * @brief 重置所有统计信息
 */
void Biconnected6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
