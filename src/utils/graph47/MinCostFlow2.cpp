/**
 * @file MinCostFlow2.cpp
 * @brief 最小费用流增强实现 — SPFA最短路/连续最短路增广
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph47/MinCostFlow2.h"

#include <QElapsedTimer>

#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
MinCostFlow2::MinCostFlow2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("MinCostFlow2"));
}

/**
 * @brief 构建流网络图
 *
 * 使用链式前向星存储有向图。每条正向边同时添加反向边（初始容量0）。
 *
 * @param n 顶点数
 * @param from 边的起点数组
 * @param to 边的终点数组
 * @param capacity 边的容量数组
 * @param cost 边的单位费用数组
 */
void MinCostFlow2::setGraph(int n, const QVector<int>& from, const QVector<int>& to,
                             const QVector<double>& capacity, const QVector<double>& cost)
{
    m_n = n;
    int m = from.size();
    m_head.assign(n, -1);
    m_to.clear();
    m_next.clear();
    m_cap.clear();
    m_cost.clear();
    m_flow.clear();

    /* 链式前向星建图，每条边加正向和反向 */
    for (int i = 0; i < m; ++i) {
        int u = from[i];
        int v = to[i];

        /* 正向边 */
        m_to.append(v);
        m_next.append(m_head[u]);
        m_cap.append(capacity[i]);
        m_cost.append(cost[i]);
        m_flow.append(0.0);
        m_head[u] = m_to.size() - 1;

        /* 反向边 */
        m_to.append(u);
        m_next.append(m_head[v]);
        m_cap.append(0.0);
        m_cost.append(-cost[i]);
        m_flow.append(0.0);
        m_head[v] = m_to.size() - 1;
    }
}

/**
 * @brief SPFA求最短路
 *
 * 在残量网络中寻找从source到sink的最短费用路径。
 * 使用SLF(Small Label First)优化deque队列。
 *
 * @param source 源点
 * @param sink 汇点
 * @param dist 输出距离数组
 * @param prev 输出前驱边索引数组
 * @return 是否存在从source到sink的可达路径
 */
bool MinCostFlow2::spfa(int source, int sink, QVector<double>& dist, QVector<int>& prev)
{
    dist.assign(m_n, std::numeric_limits<double>::max());
    prev.assign(m_n, -1);
    QVector<bool> inQueue(m_n, false);

    std::deque<int> q;
    dist[source] = 0.0;
    inQueue[source] = true;
    q.push_back(source);

    while (!q.empty()) {
        int u = q.front();
        q.pop_front();
        inQueue[u] = false;

        for (int e = m_head[u]; e != -1; e = m_next[e]) {
            /* 残量 > 0 才是可通行边 */
            double residual = m_cap[e] - m_flow[e];
            if (residual < 1e-12) continue;

            int v = m_to[e];
            double newDist = dist[u] + m_cost[e];

            if (newDist < dist[v] - 1e-12) {
                dist[v] = newDist;
                prev[v] = e;

                if (!inQueue[v]) {
                    inQueue[v] = true;
                    /* SLF优化: 小标签优先入队首 */
                    if (!q.empty() && dist[v] < dist[q.front()]) {
                        q.push_front(v);
                    } else {
                        q.push_back(v);
                    }
                }
            }
        }
    }

    return dist[sink] < std::numeric_limits<double>::max() / 2.0;
}

/**
 * @brief 求解最小费用流
 *
 * 使用连续最短路算法(SPFA + 增广)逐步发送流量，直到满足需求量。
 * 每次增广沿最短费用路径发送尽可能多的流量。
 *
 * @param source 源点
 * @param sink 汇点
 * @param demand 需求流量
 * @return 流结果(总费用、总流量、迭代次数)
 */
MinCostFlow2::FlowResult MinCostFlow2::minCostFlow(int source, int sink, double demand)
{
    QElapsedTimer timer;
    timer.start();

    FlowResult result;

    if (m_n == 0 || source < 0 || sink < 0 || source >= m_n || sink >= m_n) {
        return result;
    }

    /* 重置流量 */
    for (int i = 0; i < m_flow.size(); ++i) {
        m_flow[i] = 0.0;
    }

    double remaining = demand;

    while (remaining > 1e-12) {
        QVector<double> dist;
        QVector<int> prev;

        if (!spfa(source, sink, dist, prev)) {
            break; /* 无可达路径 */
        }

        /* 沿最短路回溯找到瓶颈残量 */
        double bottleneck = remaining;
        for (int v = sink; v != source;) {
            int e = prev[v];
            double residual = m_cap[e] - m_flow[e];
            bottleneck = qMin(bottleneck, residual);
            v = m_to[e ^ 1]; /* 反向边指向的节点即为u */
        }

        /* 沿最短路增广 */
        for (int v = sink; v != source;) {
            int e = prev[v];
            m_flow[e] += bottleneck;
            m_flow[e ^ 1] -= bottleneck; /* 反向边同步更新 */
            v = m_to[e ^ 1];
        }

        result.totalFlow += bottleneck;
        result.totalCost += bottleneck * dist[sink];
        remaining -= bottleneck;
        result.iterations++;
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFlows++;
    m_stats.totalVerticesProcessed += m_n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFlows;

    emit flowComplete(result.totalCost, result.totalFlow);
    return result;
}

/**
 * @brief 求解最小费用最大流
 *
 * 不断增广直到无法找到更短路径，获得最小费用前提下的最大流。
 *
 * @param source 源点
 * @param sink 汇点
 * @return 流结果
 */
MinCostFlow2::FlowResult MinCostFlow2::minCostMaxFlow(int source, int sink)
{
    /* 使用极大需求量驱动最大流 */
    return minCostFlow(source, sink, std::numeric_limits<double>::max());
}

/**
 * @brief 获取所有有流量的边
 * @return (起点, 终点) 对列表
 */
QVector<QPair<int, int>> MinCostFlow2::flowEdges() const
{
    QVector<QPair<int, int>> edges;
    /* 只遍历正向边(偶数索引) */
    for (int e = 0; e < m_flow.size(); e += 2) {
        if (m_flow[e] > 1e-12) {
            int u = m_to[e ^ 1]; /* 反向边的终点 = 正向边的起点 */
            int v = m_to[e];
            edges.append({u, v});
        }
    }
    return edges;
}

/**
 * @brief 重置所有累积统计信息
 */
void MinCostFlow2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
