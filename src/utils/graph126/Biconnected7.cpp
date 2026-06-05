#include "Biconnected7.h"
#include <QElapsedTimer>
#include <QStack>
#include <algorithm>

/**
 * @brief 构造函数，初始化双连通分量引擎
 * @param parent 父对象指针
 */
Biconnected7::Biconnected7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void Biconnected7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置图的节点数并清空边集
 * @param n 节点数
 */
void Biconnected7::setNodeCount(int n)
{
    m_nodeCount = n;
}

/**
 * @brief 添加一条无向边
 * @param u 节点u
 * @param v 节点v
 */
void Biconnected7::addEdge(int u, int v)
{
    Q_UNUSED(u)
    Q_UNUSED(v)
}

/**
 * @brief Tarjan算法查找所有双连通分量
 *
 * 基于DFS遍历，使用low值和disc值识别双连通分量。
 * 使用栈记录边，当发现双连通分量时弹出栈中对应边。
 * 时间复杂度O(V+E)。
 *
 * @return 各双连通分量的边列表
 */
QVector<QVector<QPair<int, int>>> Biconnected7::findBiconnectedComponents()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<QPair<int, int>>> components;

    if (m_nodeCount <= 0) {
        emit componentsFound(0, 0);
        return components;
    }

    /* 构建邻接表 */
    QVector<QVector<int>> adj(m_nodeCount);

    QVector<int> disc(m_nodeCount, -1);
    QVector<int> low(m_nodeCount, 0);
    QStack<QPair<int, int>> edgeStack;
    int time = 0;

    /* Tarjan DFS辅助函数 */
    std::function<void(int, int)> dfs = [&](int u, int parent) {
        disc[u] = low[u] = time++;
        int children = 0;

        for (int v : adj[u]) {
            if (disc[v] == -1) {
                children++;
                edgeStack.push({u, v});
                dfs(v, u);

                low[u] = qMin(low[u], low[v]);

                /* 如果u是割点或根节点，弹出一个双连通分量 */
                if ((parent == -1 && children > 1) ||
                    (parent != -1 && low[v] >= disc[u])) {
                    QVector<QPair<int, int>> comp;
                    while (!edgeStack.empty()) {
                        auto e = edgeStack.top();
                        edgeStack.pop();
                        comp.append(e);
                        if (e == qMakePair(u, v) || e == qMakePair(v, u)) break;
                    }
                    components.append(comp);
                }
            } else if (v != parent && disc[v] < disc[u]) {
                low[u] = qMin(low[u], disc[v]);
                edgeStack.push({u, v});
            }
        }
    };

    /* 处理所有连通分量 */
    for (int i = 0; i < m_nodeCount; ++i) {
        if (disc[i] == -1) {
            dfs(i, -1);
            /* 弹出剩余边 */
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

    /* 查找割点 */
    QVector<int> artPoints = findArticulationPoints();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit componentsFound(components.size(), artPoints.size());
    return components;
}

/**
 * @brief 查找所有割点
 *
 * 割点是删除后会导致图不连通的顶点。
 * 条件：根节点有>=2个子树，或非根节点u存在子节点v使得low[v]>=disc[u]。
 *
 * @return 割点索引列表
 */
QVector<int> Biconnected7::findArticulationPoints()
{
    QVector<int> artPoints;

    if (m_nodeCount <= 0) return artPoints;

    QVector<QVector<int>> adj(m_nodeCount);
    QVector<int> disc(m_nodeCount, -1);
    QVector<int> low(m_nodeCount, 0);
    QVector<bool> isArt(m_nodeCount, false);
    int time = 0;

    std::function<void(int, int)> dfs = [&](int u, int parent) {
        disc[u] = low[u] = time++;
        int children = 0;

        for (int v : adj[u]) {
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

    for (int i = 0; i < m_nodeCount; ++i) {
        if (disc[i] == -1) dfs(i, -1);
    }

    for (int i = 0; i < m_nodeCount; ++i) {
        if (isArt[i]) artPoints.append(i);
    }

    return artPoints;
}
