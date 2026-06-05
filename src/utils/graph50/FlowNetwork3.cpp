/**
 * @file FlowNetwork3.cpp
 * @brief 最小费用最大流实现 - 基于SPFA最短增广路径算法
 *
 * 使用邻接表存储网络图，通过SPFA寻找费用最短的增广路径，
 * 逐步增广直到无法从源到达汇点，保证最小费用最大流。
 * 支持负费用边，使用SLF优化加速SPFA松弛。
 */

#include "utils/graph50/FlowNetwork3.h"

#include <QElapsedTimer>
#include <QVector>
#include <deque>
#include <algorithm>
#include <limits>

/**
 * @brief 内部边结构(不暴露到头文件)
 */
struct FlowEdge3 {
    int to;         /**< 目标节点索引 */
    int rev;        /**< 反向边在目标节点邻接表中的位置 */
    double cap;     /**< 剩余容量 */
    double cost;    /**< 单位费用(可正可负) */
};

/**
 * @brief 构造函数，初始化空网络
 * @param parent 父QObject
 */
FlowNetwork3::FlowNetwork3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置网络图结构
 *
 * 从边列表构建邻接表，每条有向边同时创建容量为0的反向边。
 * 反向边的费用为正向边的相反数，用于流量回退。
 *
 * @param n 节点数量(编号0~n-1)
 * @param from 边的起始节点列表
 * @param to 边的终止节点列表
 * @param cap 边的容量列表(必须 >= 0)
 * @param cost 边的单位费用列表
 */
void FlowNetwork3::setGraph(int n, const QVector<int>& from, const QVector<int>& to,
                            const QVector<double>& cap, const QVector<double>& cost)
{
    m_n = n;

    /* 重建内部邻接表(存储在头文件预留的成员中) */
    /* 使用m_rowPtr作为每个节点出边数量计数 */
    /* 由于头文件成员限制，将图结构编码到现有字段 */
    m_rowPtr.clear();
    m_colIdx.clear();
    m_values.clear();

    m_rowPtr.resize(n, 0);

    const int edgeCount = qMin(qMin(from.size(), to.size()),
                               qMin(cap.size(), cost.size()));

    /* 统计每个节点的出边数(正向+反向) */
    for (int i = 0; i < edgeCount; ++i) {
        if (from[i] >= 0 && from[i] < n && to[i] >= 0 && to[i] < n) {
            m_rowPtr[from[i]]++; /* 正向边 */
            m_rowPtr[to[i]]++;   /* 反向边 */
        }
    }

    /* 计算前缀和 */
    QVector<int> head = m_rowPtr;
    m_rowPtr.resize(n + 1, 0);
    for (int i = 0; i < n; ++i)
        m_rowPtr[i + 1] = m_rowPtr[i] + head[i];

    int totalEdges = m_rowPtr[n];

    /* 分配空间: 每条边存 {to, rev, cap, cost} = 4个double */
    m_colIdx.resize(totalEdges, 0);
    m_values.resize(totalEdges * 3, 0.0);

    /* 临时记录每个节点的当前写入位置 */
    QVector<int> pos = m_rowPtr;

    /* 构建双向边 */
    for (int i = 0; i < edgeCount; ++i) {
        if (from[i] >= 0 && from[i] < n && to[i] >= 0 && to[i] < n) {
            int u = from[i], v = to[i];
            double c = qMax(0.0, cap[i]);
            double w = cost[i];

            /* 正向边 u->v */
            int fwdIdx = pos[u];
            /* 反向边 v->u */
            int revIdx = pos[v];

            m_colIdx[fwdIdx] = v;
            m_values[fwdIdx * 3] = c;        /* cap */
            m_values[fwdIdx * 3 + 1] = w;    /* cost */
            m_values[fwdIdx * 3 + 2] = revIdx; /* rev */

            m_colIdx[revIdx] = u;
            m_values[revIdx * 3] = 0.0;      /* 初始容量0 */
            m_values[revIdx * 3 + 1] = -w;   /* 反向费用 */
            m_values[revIdx * 3 + 2] = fwdIdx; /* rev */

            pos[u]++;
            pos[v]++;
        }
    }
}

/**
 * @brief 执行最小费用最大流计算
 *
 * 使用SPFA(Shortest Path Faster Algorithm)在残量网络中寻找
 * 从source到sink的费用最短增广路径:
 * 1. 初始化距离数组，将源点入队
 * 2. 对队列中每个节点松弛其出边
 * 3. 使用SLF(Small Label First)优化: 新节点距离小于队首时插入队首
 * 4. 找到增广路径后，沿路径更新残量网络
 * 5. 重复直到无法从源到达汇点
 *
 * @param source 源节点索引
 * @param sink   汇节点索引
 * @return QPair<总费用, 总流量>
 */
QPair<double, double> FlowNetwork3::minCostMaxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    double totalCost = 0.0;
    double totalFlow = 0.0;

    if (m_n <= 0 || source == sink || source < 0 || source >= m_n ||
        sink < 0 || sink >= m_n) {
        m_stats.totalFlows++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalFlows > 0) ?
            m_timeSum / m_stats.totalFlows : 0.0;
        emit flowComplete(0.0, 0.0);
        return {0.0, 0.0};
    }

    const int nodeCount = m_n;

    /* SPFA主循环: 反复寻找增广路径 */
    const double INF = std::numeric_limits<double>::max();

    while (true) {
        /* 距离数组 */
        QVector<double> dist(nodeCount, INF);
        QVector<int> prevNode(nodeCount, -1);
        QVector<int> prevEdgeIdx(nodeCount, -1);
        QVector<bool> inQueue(nodeCount, false);
        /* 记录每个节点入队次数，用于负环检测 */
        QVector<int> visitCount(nodeCount, 0);

        dist[source] = 0.0;
        std::deque<int> queue;
        queue.push_back(source);
        inQueue[source] = true;
        visitCount[source] = 1;

        /* SPFA松弛 */
        while (!queue.empty()) {
            int u = queue.front();
            queue.pop_front();
            inQueue[u] = false;

            /* 遍历u的所有出边 */
            for (int ei = m_rowPtr[u]; ei < m_rowPtr[u + 1]; ++ei) {
                int v = m_colIdx[ei];
                double cap = m_values[ei * 3];
                double cost = m_values[ei * 3 + 1];

                /* 只处理有剩余容量的边 */
                if (cap < 1e-12) continue;

                /* 松弛条件 */
                if (dist[u] + cost < dist[v] - 1e-12) {
                    dist[v] = dist[u] + cost;
                    prevNode[v] = u;
                    prevEdgeIdx[v] = ei;

                    if (!inQueue[v]) {
                        /* SLF优化: 新距离小于队首距离时插入队首 */
                        if (!queue.empty() && dist[v] < dist[queue.front()])
                            queue.push_front(v);
                        else
                            queue.push_back(v);
                        inQueue[v] = true;
                        visitCount[v]++;

                        /* 负环保护: 如果某节点入队超过n次则终止 */
                        if (visitCount[v] > nodeCount + 1)
                            goto spfa_done;
                    }
                }
            }
        }
    spfa_done:

        /* 如果汇点不可达则结束 */
        if (dist[sink] >= INF - 1.0)
            break;

        /* 沿增广路径找到瓶颈流量(最小剩余容量) */
        double augFlow = INF;
        for (int v = sink; v != source; v = prevNode[v]) {
            int ei = prevEdgeIdx[v];
            augFlow = qMin(augFlow, m_values[ei * 3]);
        }

        /* 沿路径更新残量网络 */
        for (int v = sink; v != source; v = prevNode[v]) {
            int ei = prevEdgeIdx[v];
            int revIdx = static_cast<int>(m_values[ei * 3 + 2]);

            /* 正向边减少容量 */
            m_values[ei * 3] -= augFlow;
            /* 反向边增加容量 */
            m_values[revIdx * 3] += augFlow;
        }

        totalFlow += augFlow;
        totalCost += augFlow * dist[sink];
    }

    m_stats.totalFlows++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFlows > 0) ?
        m_timeSum / m_stats.totalFlows : 0.0;

    emit flowComplete(totalCost, totalFlow);
    return {totalCost, totalFlow};
}

/**
 * @brief 重置所有统计数据
 */
void FlowNetwork3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
