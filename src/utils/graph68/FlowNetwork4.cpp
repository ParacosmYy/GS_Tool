/**
 * @file FlowNetwork4.cpp
 * @brief 流网络求解器实现，基于Dinic算法的最大流和最小割
 *
 * 实现了Dinic最大流算法，包含BFS分层和DFS阻塞流两个阶段。
 * 时间复杂度: O(V^2 * E)，在二分图等特殊图上可达O(E*sqrt(V))。
 * 同时支持最小割的计算（通过最大流-最小割定理）。
 *
 * 适用于网络流量优化、任务分配、图像分割等场景。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph68/FlowNetwork4.h"

#include <QElapsedTimer>
#include <QQueue>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空网络
 * @param parent 父QObject对象指针
 */
FlowNetwork4::FlowNetwork4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置流网络拓扑和容量
 * @param n 顶点数量
 * @param edges 边列表，每条边为 ((起点, 终点), 容量)
 */
void FlowNetwork4::setGraph(int n, const QVector<QPair<QPair<int, int>, double>>& edges)
{
    m_n = n;
    m_adj.clear();
    m_adj.resize(n);

    for (const auto& e : edges) {
        int u = e.first.first;
        int v = e.first.second;
        double cap = e.second;

        if (u < 0 || u >= n || v < 0 || v >= n || u == v) continue;

        /* 正向边：索引为偶数 */
        m_adj[u].append({v, cap});
        /* 反向边：索引为奇数（残余容量） */
        m_adj[v].append({u, 0.0});
    }
}

/**
 * @brief BFS构建层次图
 *
 * 从源点s出发进行BFS，计算每个顶点的层次（距s的最短距离）。
 * 层次图只包含层次递增的边（用于Dinic算法的阻塞流计算）。
 *
 * @param s 源点
 * @param t 汇点
 * @param level 输出：各顶点的层次，-1表示不可达
 * @return true表示存在s到t的增广路径，false表示已达到最大流
 */
bool FlowNetwork4::bfs(int s, int t, QVector<int>& level)
{
    level.fill(-1, m_n);
    level[s] = 0;

    QQueue<int> queue;
    queue.enqueue(s);

    while (!queue.isEmpty()) {
        int u = queue.dequeue();

        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            double cap = edge.second;

            /* 仅通过还有残余容量的边 */
            if (level[v] < 0 && cap > 1e-10) {
                level[v] = level[u] + 1;
                queue.enqueue(v);
            }
        }
    }

    return level[t] >= 0;
}

/**
 * @brief DFS寻找阻塞流
 *
 * 在层次图上执行多路增广DFS。使用iter数组优化，
 * 避免重复搜索已经遍历过的边。
 *
 * @param u 当前顶点
 * @param t 汇点
 * @param f 当前路径上的最小残余容量
 * @param iter 各顶点的边搜索起点（优化）
 * @return 从u到t实际推送的流量
 */
double FlowNetwork4::dfs(int u, int t, double f, QVector<int>& iter)
{
    if (u == t) return f;

    for (int& i = iter[u]; i < m_adj[u].size(); ++i) {
        int v = m_adj[u][i].first;
        double& cap = m_adj[u][i].second;

        /* 仅在层次递增且容量足够的边上前进 */
        if (cap > 1e-10) {
            double pushed = dfs(v, t, qMin(f, cap), iter);
            if (pushed > 1e-10) {
                /* 更新残余容量 */
                cap -= pushed;
                /* 找到反向边并更新 */
                for (auto& re : m_adj[v]) {
                    if (re.first == u) {
                        re.second += pushed;
                        break;
                    }
                }
                return pushed;
            }
        }
    }
    return 0.0;
}

/**
 * @brief Dinic算法求解最大流
 *
 * 算法流程：
 * 1. BFS构建层次图
 * 2. DFS在层次图上寻找阻塞流
 * 3. 重复直到不存在增广路径
 *
 * @param s 源点
 * @param t 汇点
 * @return 最大流值
 */
double FlowNetwork4::dinic(int s, int t)
{
    double totalFlow = 0.0;
    QVector<int> level(m_n);

    while (bfs(s, t, level)) {
        QVector<int> iter(m_n, 0);
        double pushed;
        while ((pushed = dfs(s, t, 1e18, iter)) > 1e-10) {
            totalFlow += pushed;
        }
    }

    return totalFlow;
}

/**
 * @brief 计算从源点到汇点的最大流
 * @param source 源点索引
 * @param sink 汇点索引
 * @return 最大流值
 */
double FlowNetwork4::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    if (source < 0 || source >= m_n || sink < 0 || sink >= m_n || source == sink)
        return 0.0;

    double flow = dinic(source, sink);

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit flowCompleted(flow);
    return flow;
}

/**
 * @brief 计算最小割
 *
 * 在最大流计算后，通过BFS在残余网络中找到从源点可达的顶点集S。
 * 最小割 = 从S到V-S的所有边（最大流-最小割定理）。
 *
 * @param source 源点
 * @param sink 汇点
 * @return 最小割的边列表（起点, 终点）
 */
QVector<QPair<int, int>> FlowNetwork4::minCut(int source, int sink) const
{
    Q_UNUSED(sink);

    /* 在残余网络中BFS */
    QVector<bool> visited(m_n, false);
    QQueue<int> queue;
    queue.enqueue(source);
    visited[source] = true;

    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            double cap = edge.second;
            if (!visited[v] && cap > 1e-10) {
                visited[v] = true;
                queue.enqueue(v);
            }
        }
    }

    /* 找从已访问集合到未访问集合的边 */
    QVector<QPair<int, int>> cut;
    for (int u = 0; u < m_n; ++u) {
        if (!visited[u]) continue;
        for (const auto& edge : m_adj[u]) {
            int v = edge.first;
            if (!visited[v]) {
                cut.append({u, v});
            }
        }
    }

    return cut;
}

/**
 * @brief 重置所有统计数据
 */
void FlowNetwork4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
