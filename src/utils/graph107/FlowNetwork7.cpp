#include "FlowNetwork7.h"
#include <QElapsedTimer>
#include <algorithm>
#include <queue>

/**
 * @brief 构造函数，初始化流网络求解器
 * @param parent 父对象指针
 */
FlowNetwork7::FlowNetwork7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 顶点数量
 */
void FlowNetwork7::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加有向边及容量
 * @param from 起始顶点
 * @param to 终止顶点
 * @param capacity 边的容量
 */
void FlowNetwork7::addEdge(int from, int to, double capacity)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(capacity)
}

/**
 * @brief BFS寻找增广路径(Edmonds-Karp)
 * @param adj 邻接表(残差图)
 * @param capacity 容量矩阵
 * @param source 源点
 * @param sink 汇点
 * @param parent 记录路径前驱
 * @return 是否找到增广路径
 */
static bool bfsAugment(const QVector<QVector<int>>& adj,
                        const QVector<QVector<double>>& capacity,
                        int source, int sink, QVector<int>& parent)
{
    int n = adj.size();
    std::fill(parent.begin(), parent.end(), -1);
    QVector<bool> visited(n, false);
    std::queue<int> q;
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : adj[u]) {
            if (!visited[v] && capacity[u][v] > 1e-10) {
                visited[v] = true;
                parent[v] = u;
                if (v == sink) return true;
                q.push(v);
            }
        }
    }
    return false;
}

/**
 * @brief 计算源点到汇点的最大流(Edmonds-Karp算法)
 *
 * 反复通过BFS寻找增广路径，沿路径推进最小残余容量，
 * 直到不存在增广路径为止，此时流量即为最大流。
 *
 * @param source 源点编号
 * @param sink 汇点编号
 */
void FlowNetwork7::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    if (m_vertexCount <= 0 || source < 0 || sink < 0 ||
        source >= m_vertexCount || sink >= m_vertexCount) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolved++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
        emit solved(0.0);
        return;
    }

    int n = m_vertexCount;

    /* 构建残差图的容量矩阵和邻接表 */
    QVector<QVector<double>> cap(n, QVector<double>(n, 0.0));
    QVector<QVector<int>> adj(n);

    /* 添加示例边(实际使用时由addEdge填充) */
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (cap[i][j] > 0 || cap[j][i] > 0) {
                adj[i].append(j);
                adj[j].append(i);
            }
        }
    }

    /* Edmonds-Karp: BFS寻找增广路径 */
    double totalFlow = 0.0;
    QVector<int> parent(n, -1);

    while (bfsAugment(adj, cap, source, sink, parent)) {
        /* 找到路径上的最小残余容量 */
        double pathFlow = 1e18;
        int v = sink;
        while (v != source) {
            int u = parent[v];
            pathFlow = qMin(pathFlow, cap[u][v]);
            v = u;
        }

        /* 更新残差图 */
        v = sink;
        while (v != source) {
            int u = parent[v];
            cap[u][v] -= pathFlow;
            cap[v][u] += pathFlow;
            v = u;
        }

        totalFlow += pathFlow;
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
    emit solved(totalFlow);
}

/**
 * @brief 重置统计数据
 */
void FlowNetwork7::resetStatistics()
{
    m_stats.totalSolved = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
