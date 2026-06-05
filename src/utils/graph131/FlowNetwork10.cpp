#include "FlowNetwork10.h"
#include <QElapsedTimer>
#include <queue>
#include <algorithm>

/**
 * @brief 构造函数，初始化流网络引擎v10
 * @param parent 父对象指针
 */
FlowNetwork10::FlowNetwork10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void FlowNetwork10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Edmonds-Karp算法求最大流（BFS增广）
 *
 * Ford-Fulkerson的BFS实现，每次寻找最短增广路径。
 * 时间复杂度O(VE^2)。
 *
 * @param capacityMatrix 容量矩阵
 * @param source 源点索引
 * @param sink 汇点索引
 * @return 最大流值
 */
double FlowNetwork10::edmondsKarp(const QVector<QVector<double>>& capacityMatrix,
                                   int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    const int n = capacityMatrix.size();
    double maxFlowVal = 0.0;

    if (n == 0 || source < 0 || sink < 0 || source >= n || sink >= n || source == sink) {
        emit flowCompleted(0.0);
        return 0.0;
    }

    /* 残余图 */
    QVector<QVector<double>> residual = capacityMatrix;

    /* BFS寻找增广路径 */
    auto bfs = [&]() -> double {
        QVector<int> parent(n, -1);
        parent[source] = source;
        std::queue<int> q;
        q.push(source);

        while (!q.empty() && parent[sink] < 0) {
            int u = q.front(); q.pop();
            for (int v = 0; v < n; ++v) {
                if (parent[v] < 0 && u < residual.size() && v < residual[u].size()
                    && residual[u][v] > 0) {
                    parent[v] = u;
                    q.push(v);
                }
            }
        }

        if (parent[sink] < 0) return 0.0;

        /* 计算瓶颈容量 */
        double pathFlow = 1e18;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            pathFlow = qMin(pathFlow, residual[u][v]);
        }

        /* 更新残余图 */
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            residual[u][v] -= pathFlow;
            residual[v][u] += pathFlow;
        }

        return pathFlow;
    };

    double pathFlow;
    while ((pathFlow = bfs()) > 0) {
        maxFlowVal += pathFlow;
    }

    /* 保存流量矩阵 */
    m_flowMatrix = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < qMin(n, capacityMatrix[u].size()); ++v) {
            if (capacityMatrix[u][v] > 0) {
                m_flowMatrix[u][v] = capacityMatrix[u][v] - residual[u][v];
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFlowOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFlowOps;

    emit flowCompleted(maxFlowVal);
    return maxFlowVal;
}

/**
 * @brief Dinic算法求最大流（分层图+阻塞流）
 *
 * BFS构建层次图，DFS寻找阻塞流。
 * 时间复杂度O(V^2 E)，比Edmonds-Karp更快。
 *
 * @param capacityMatrix 容量矩阵
 * @param source 源点索引
 * @param sink 汇点索引
 * @return 最大流值
 */
double FlowNetwork10::dinic(const QVector<QVector<double>>& capacityMatrix,
                             int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    const int n = capacityMatrix.size();
    double maxFlowVal = 0.0;

    if (n == 0 || source < 0 || sink < 0 || source >= n || sink >= n || source == sink) {
        emit flowCompleted(0.0);
        return 0.0;
    }

    QVector<QVector<double>> residual = capacityMatrix;
    QVector<int> level(n);
    QVector<int> iter(n);

    /* BFS构建层次图 */
    auto bfs = [&]() -> bool {
        level.fill(-1);
        level[source] = 0;
        std::queue<int> q;
        q.push(source);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v = 0; v < n; ++v) {
                if (level[v] < 0 && u < residual.size() && v < residual[u].size()
                    && residual[u][v] > 0) {
                    level[v] = level[u] + 1;
                    q.push(v);
                }
            }
        }
        return level[sink] >= 0;
    };

    /* DFS寻找阻塞流 */
    std::function<double(int, double)> dfs = [&](int u, double f) -> double {
        if (u == sink) return f;
        for (int& i = iter[u]; i < n; ++i) {
            int v = i;
            if (level[v] == level[u] + 1 && u < residual.size() && v < residual[u].size()
                && residual[u][v] > 0) {
                double d = dfs(v, qMin(f, residual[u][v]));
                if (d > 0) {
                    residual[u][v] -= d;
                    residual[v][u] += d;
                    return d;
                }
            }
        }
        return 0.0;
    };

    while (bfs()) {
        iter.fill(0);
        double d;
        while ((d = dfs(source, 1e18)) > 0) {
            maxFlowVal += d;
        }
    }

    /* 保存流量矩阵 */
    m_flowMatrix = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < qMin(n, capacityMatrix[u].size()); ++v) {
            if (capacityMatrix[u][v] > 0) {
                m_flowMatrix[u][v] = capacityMatrix[u][v] - residual[u][v];
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFlowOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFlowOps;

    emit flowCompleted(maxFlowVal);
    return maxFlowVal;
}

/**
 * @brief 求最小割（与最大流对偶）
 *
 * 计算最大流后，从源点BFS可达的顶点集S与不可达的顶点集T。
 * S到T的所有边构成最小割。
 *
 * @param capacityMatrix 容量矩阵
 * @param source 源点索引
 * @param sink 汇点索引
 * @return 割集顶点划分 (S侧, T侧)
 */
QPair<QVector<int>, QVector<int>> FlowNetwork10::minCut(
    const QVector<QVector<double>>& capacityMatrix, int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    QPair<QVector<int>, QVector<int>> result;

    const int n = capacityMatrix.size();
    if (n == 0) {
        emit flowCompleted(0.0);
        return result;
    }

    /* 先计算最大流 */
    dinic(capacityMatrix, source, sink);

    /* 从源点BFS沿残余容量>0的边 */
    QVector<bool> reachable(n, false);
    std::queue<int> q;
    if (source >= 0 && source < n) {
        reachable[source] = true;
        q.push(source);
    }

    /* 使用dinic后的残余图（局部重建） */
    QVector<QVector<double>> residual = capacityMatrix;
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < qMin(n, m_flowMatrix[u].size()); ++v) {
            if (capacityMatrix[u][v] > 0) {
                residual[u][v] = capacityMatrix[u][v] - m_flowMatrix[u][v];
            }
        }
    }

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v = 0; v < n; ++v) {
            if (!reachable[v] && u < residual.size() && v < residual[u].size()
                && residual[u][v] > 0) {
                reachable[v] = true;
                q.push(v);
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (reachable[i]) result.first.append(i);
        else result.second.append(i);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    return result;
}

/**
 * @brief 获取各边的实际流量
 * @return 流量矩阵
 */
QVector<QVector<double>> FlowNetwork10::getFlowMatrix() const
{
    return m_flowMatrix;
}
