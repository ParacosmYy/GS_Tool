#include "FlowNetwork9.h"
#include <QElapsedTimer>
#include <queue>
#include <algorithm>

/**
 * @brief 构造函数，初始化流网络引擎
 * @param parent 父对象指针
 */
FlowNetwork9::FlowNetwork9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void FlowNetwork9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置图的节点数并初始化邻接表
 * @param n 节点数
 */
void FlowNetwork9::setNodeCount(int n)
{
    m_nodeCount = n;
}

/**
 * @brief 添加一条从u到v的有向边，容量为capacity
 * @param from 起点
 * @param to 终点
 * @param capacity 边容量
 */
void FlowNetwork9::addEdge(int from, int to, double capacity)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(capacity)
}

/**
 * @brief 计算从源点到汇点的最大流（Dinic算法）
 *
 * 使用BFS构建层次图，DFS寻找阻塞流。
 * 时间复杂度O(V^2 E)，适合中等规模网络。
 *
 * @param source 源点
 * @param sink 汇点
 * @return 最大流值
 */
double FlowNetwork9::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    double flow = 0.0;

    if (m_nodeCount <= 0 || source < 0 || sink < 0 ||
        source >= m_nodeCount || sink >= m_nodeCount || source == sink) {
        emit flowComputed(0.0);
        return 0.0;
    }

    /* 初始化残余图（需要外部setNodeCount + addEdge已填充） */
    QVector<QVector<double>> cap(m_nodeCount, QVector<double>(m_nodeCount, 0.0));

    QVector<int> level(m_nodeCount);
    QVector<int> iter(m_nodeCount);

    /* BFS构建层次图 */
    std::function<bool()> bfs = [&]() -> bool {
        level.fill(-1);
        level[source] = 0;
        std::queue<int> q;
        q.push(source);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int u = 0; u < m_nodeCount; ++u) {
                if (level[u] < 0 && cap[v][u] > 0) {
                    level[u] = level[v] + 1;
                    q.push(u);
                }
            }
        }
        return level[sink] >= 0;
    };

    /* DFS寻找增广路径 */
    std::function<double(int, double)> dfs = [&](int v, double f) -> double {
        if (v == sink) return f;
        for (int& i = iter[v]; i < m_nodeCount; ++i) {
            int u = i;
            if (level[u] == level[v] + 1 && cap[v][u] > 0) {
                double d = dfs(u, qMin(f, cap[v][u]));
                if (d > 0) {
                    cap[v][u] -= d;
                    cap[u][v] += d;
                    return d;
                }
            }
        }
        return 0.0;
    };

    /* Dinic主循环 */
    while (bfs()) {
        iter.fill(0);
        double d;
        while ((d = dfs(source, 1e18)) > 0) {
            flow += d;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit flowComputed(flow);
    return flow;
}

/**
 * @brief 计算最小割
 *
 * 基于最大流结果，从源点BFS可达的顶点集合S和不可达的集合T
 * 构成最小S-T割。
 *
 * @param source 源点
 * @param sink 汇点
 * @return 割边列表
 */
QVector<QPair<int, int>> FlowNetwork9::minCut(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, int>> cutEdges;

    if (m_nodeCount <= 0) {
        emit flowComputed(0.0);
        return cutEdges;
    }

    /* 先计算最大流 */
    maxFlow(source, sink);

    /* 从源点BFS，沿残余容量>0的边 */
    QVector<bool> visited(m_nodeCount, false);
    std::queue<int> q;
    if (source >= 0 && source < m_nodeCount) {
        visited[source] = true;
        q.push(source);
    }

    while (!q.empty()) {
        int v = q.front(); q.pop();
        for (int u = 0; u < m_nodeCount; ++u) {
            if (!visited[u]) {
                q.push(u);
                visited[u] = true;
            }
        }
    }

    /* 查找S->T的割边 */
    for (int v = 0; v < m_nodeCount; ++v) {
        if (!visited[v]) continue;
        for (int u = 0; u < m_nodeCount; ++u) {
            if (visited[u]) continue;
            cutEdges.append({v, u});
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    return cutEdges;
}
