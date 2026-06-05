/**
 * @file MaxFlow.cpp
 * @brief Edmonds-Karp最大流算法实现
 */

#include "MaxFlow.h"
#include <QElapsedTimer>
#include <algorithm>
#include <limits>

MaxFlow::MaxFlow(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

double MaxFlow::compute(const QVector<QVector<double>>& capacity,
                        int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    int n = capacity.size();
    if (n == 0 || source < 0 || sink < 0 || source >= n || sink >= n
        || source == sink) {
        emit computationCompleted(0.0);
        return 0.0;
    }

    /* 初始化残余容量图 */
    QVector<QVector<double>> residual(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            residual[i][j] = capacity[i][j];

    double maxFlow = 0.0;
    QVector<int> parent(n);

    /* Edmonds-Karp: 反复BFS寻找增广路径 */
    while (bfs(residual, source, sink, parent)) {
        /* 找增广路径上的瓶颈容量 */
        double pathFlow = std::numeric_limits<double>::max();
        int v = sink;
        while (v != source) {
            int u = parent[v];
            pathFlow = std::min(pathFlow, residual[u][v]);
            v = u;
        }

        /* 更新残余容量 */
        v = sink;
        while (v != source) {
            int u = parent[v];
            residual[u][v] -= pathFlow;
            residual[v][u] += pathFlow;
            v = u;
        }

        maxFlow += pathFlow;
    }

    /* 计算实际流量矩阵: flow[i][j] = max(0, capacity[i][j] - residual[i][j]) */
    m_lastFlow.clear();
    m_lastFlow.resize(n);
    for (int i = 0; i < n; ++i) {
        m_lastFlow[i].resize(n);
        for (int j = 0; j < n; ++j) {
            double flow = capacity[i][j] - residual[i][j];
            m_lastFlow[i][j] = std::max(0.0, flow);
        }
    }

    /* 统计更新 */
    m_stats.totalComputed++;
    m_stats.totalNodes += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalComputed > 0) ? m_timeSum / m_stats.totalComputed : 0.0;

    emit computationCompleted(maxFlow);
    return maxFlow;
}

QVector<QVector<double>> MaxFlow::getFlow() const
{
    return m_lastFlow;
}

void MaxFlow::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_lastFlow.clear();
}

bool MaxFlow::bfs(const QVector<QVector<double>>& residual,
                  int source, int sink,
                  QVector<int>& parent) const
{
    int n = residual.size();
    QVector<bool> visited(n, false);
    parent.fill(-1);

    QQueue<int> queue;
    queue.enqueue(source);
    visited[source] = true;

    while (!queue.isEmpty()) {
        int u = queue.dequeue();

        for (int v = 0; v < n; ++v) {
            if (!visited[v] && residual[u][v] > 1e-12) {
                visited[v] = true;
                parent[v] = u;
                if (v == sink)
                    return true;
                queue.enqueue(v);
            }
        }
    }
    return false;
}
