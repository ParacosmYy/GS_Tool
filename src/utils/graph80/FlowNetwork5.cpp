/**
 * @file FlowNetwork5.cpp
 * @brief 流网络最大流/最小费用流算法实现（第5版）
 *
 * 实现Dinic算法求解最大流和最小割问题。使用BFS构建层次图，
 * DFS在层次图中寻找阻塞流。支持多组增广路的并行推进。
 * 同时提供基于最小割的边划分功能。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph80/FlowNetwork5.h"

#include <QElapsedTimer>
#include <QQueue>
#include <QtMath>
#include <queue>
#include <limits>

/**
 * @brief 构造函数，初始化流网络求解器
 * @param parent 父QObject对象指针
 */
FlowNetwork5::FlowNetwork5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量
 */
void FlowNetwork5::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
}

/**
 * @brief 添加有向边
 * @param u 起点
 * @param v 终点
 * @param cap 边的容量
 */
void FlowNetwork5::addEdge(int u, int v, double cap)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;

    /* 正向边 */
    m_adj[u].append({v, m_adj[v].size(), cap, 0.0});
    /* 反向边（残余图） */
    m_adj[v].append({u, m_adj[u].size() - 1, 0.0, 0.0});
}

/**
 * @brief 设置源点
 * @param s 源点索引
 */
void FlowNetwork5::setSource(int s)
{
    m_source = qBound(0, s, m_n - 1);
}

/**
 * @brief 设置汇点
 * @param t 汇点索引
 */
void FlowNetwork5::setSink(int t)
{
    m_sink = qBound(0, t, m_n - 1);
}

/**
 * @brief BFS构建层次图
 *
 * 从源点出发，按BFS顺序为每个可达顶点分配层次。
 * 每层只保留容量>0的边。
 *
 * @param level 输出的层次数组
 * @return 汇点是否可达（即是否存在增广路）
 */
double FlowNetwork5::bfsLevelGraph(QVector<int>& level)
{
    level.fill(-1);
    level[m_source] = 0;

    QQueue<int> queue;
    queue.enqueue(m_source);

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        for (const auto& edge : m_adj[u]) {
            if (level[edge.to] < 0 && edge.cap > 1e-12) {
                level[edge.to] = level[u] + 1;
                queue.enqueue(edge.to);
            }
        }
    }

    return (level[m_sink] >= 0) ? 1.0 : 0.0;
}

/**
 * @brief DFS在层次图中寻找阻塞流
 *
 * 按层次递增方向DFS，使用ptr数组记录当前弧优化。
 *
 * @param u 当前顶点
 * @param pushed 当前路径上的最小残余容量
 * @param ptr 当前弧指针数组
 * @param level 层次数组
 * @return 实际推进的流量
 */
double FlowNetwork5::dfsBlocking(int u, double pushed, QVector<int>& ptr,
                                   const QVector<int>& level)
{
    if (u == m_sink) return pushed;

    for (int& i = ptr[u]; i < m_adj[u].size(); ++i) {
        auto& edge = m_adj[u][i];
        if (level[edge.to] == level[u] + 1 && edge.cap > 1e-12) {
            double flow = dfsBlocking(edge.to, qMin(pushed, edge.cap), ptr, level);
            if (flow > 1e-12) {
                /* 更新残余容量 */
                edge.cap -= flow;
                m_adj[edge.to][edge.rev].cap += flow;
                return flow;
            }
        }
    }

    return 0.0;
}

/**
 * @brief 使用Dinic算法计算最大流
 *
 * 重复执行BFS构建层次图和DFS寻找阻塞流，
 * 直到无法从源点到达汇点为止。
 *
 * @return 最大流值
 */
double FlowNetwork5::maxFlow()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) {
        emit flowComputed(0.0, 0);
        return 0.0;
    }

    double totalFlow = 0.0;
    int augmentations = 0;
    QVector<int> level(m_n);

    while (bfsLevelGraph(level) > 0) {
        QVector<int> ptr(m_n, 0);
        double flow;
        while ((flow = dfsBlocking(m_source,
                 std::numeric_limits<double>::max(), ptr, level)) > 1e-12) {
            totalFlow += flow;
            augmentations++;
        }
    }

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit flowComputed(totalFlow, augmentations);
    return totalFlow;
}

/**
 * @brief 计算最小费用最大流
 *
 * 使用最短增广路算法（SPFA/Bellman-Ford寻找最小费用路径）。
 * 在最大流的基础上优化总费用。
 *
 * @return 最小费用值
 */
double FlowNetwork5::minCostMaxFlow()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return 0.0;

    double totalCost = 0.0;
    double totalFlow = 0.0;

    /* 使用SPFA寻找最小费用增广路 */
    while (true) {
        QVector<double> dist(m_n, std::numeric_limits<double>::max());
        QVector<int> prevNode(m_n, -1);
        QVector<int> prevEdge(m_n, -1);
        QVector<bool> inQueue(m_n, false);

        dist[m_source] = 0.0;
        QQueue<int> queue;
        queue.enqueue(m_source);
        inQueue[m_source] = true;

        while (!queue.isEmpty()) {
            int u = queue.dequeue();
            inQueue[u] = false;
            for (int i = 0; i < m_adj[u].size(); ++i) {
                const auto& edge = m_adj[u][i];
                if (edge.cap > 1e-12 && dist[u] + edge.cost < dist[edge.to] - 1e-12) {
                    dist[edge.to] = dist[u] + edge.cost;
                    prevNode[edge.to] = u;
                    prevEdge[edge.to] = i;
                    if (!inQueue[edge.to]) {
                        queue.enqueue(edge.to);
                        inQueue[edge.to] = true;
                    }
                }
            }
        }

        if (prevNode[m_sink] == -1) break;

        /* 沿增广路推进流 */
        double flow = std::numeric_limits<double>::max();
        for (int v = m_sink; v != m_source; v = prevNode[v]) {
            flow = qMin(flow, m_adj[prevNode[v]][prevEdge[v]].cap);
        }

        for (int v = m_sink; v != m_source; v = prevNode[v]) {
            auto& edge = m_adj[prevNode[v]][prevEdge[v]];
            edge.cap -= flow;
            m_adj[v][edge.rev].cap += flow;
            totalCost += edge.cost * flow;
        }

        totalFlow += flow;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    return totalCost;
}

/**
 * @brief 查找最小割的边集
 *
 * 在最大流计算后，通过BFS从源点遍历残余图。
 * 可达顶点集S和不可达顶点集T之间的边即为最小割边。
 *
 * @return 最小割的边集合
 */
QVector<QPair<int,int>> FlowNetwork5::minCut()
{
    QVector<QPair<int,int>> cut;

    if (m_n == 0) return cut;

    /* 在残余图中从源点BFS */
    QVector<bool> visited(m_n, false);
    QQueue<int> queue;
    queue.enqueue(m_source);
    visited[m_source] = true;

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        for (const auto& edge : m_adj[u]) {
            if (!visited[edge.to] && edge.cap > 1e-12) {
                visited[edge.to] = true;
                queue.enqueue(edge.to);
            }
        }
    }

    /* 找到从已访问到未访问的边 */
    for (int u = 0; u < m_n; ++u) {
        if (!visited[u]) continue;
        for (const auto& edge : m_adj[u]) {
            if (!visited[edge.to] && edge.cost == 0.0) {
                /* 原始边（非反向边） */
                cut.append({u, edge.to});
            }
        }
    }

    return cut;
}

/**
 * @brief 获取当前统计信息
 * @return 求解统计结构
 */
FlowNetwork5::Stats FlowNetwork5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void FlowNetwork5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
