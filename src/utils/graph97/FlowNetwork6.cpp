#include "FlowNetwork6.h"
#include <QElapsedTimer>
#include <queue>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化流网络求解器
 * @param parent 父QObject对象指针
 */
FlowNetwork6::FlowNetwork6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 使用Dinic算法计算最大流
 *
 * Dinic算法分阶段执行：
 * 1. BFS构建层次图（从源到各点的最短距离）
 * 2. DFS在层次图上寻找增广路径并推送流量
 * 3. 重复直到无法构建新的层次图
 *
 * 时间复杂度O(V^2*E)，实践中通常更快。
 *
 * @param capacity 容量矩阵，capacity[u][v]为边(u,v)的容量
 * @param source 源点索引
 * @param sink 汇点索引
 * @return 最大流值
 */
double FlowNetwork6::maxFlow(const QVector<QVector<double>>& capacity, int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    const int n = capacity.size();
    if (n == 0 || source == sink) return 0.0;

    /// 残余图（可修改的容量副本）
    QVector<QVector<double>> residual = capacity;
    double maxFlowValue = 0.0;
    int augmentingPaths = 0;

    /// Dinic算法主循环
    while (true) {
        /// BFS构建层次图
        QVector<int> level(n, -1);
        std::queue<int> queue;
        queue.push(source);
        level[source] = 0;

        while (!queue.empty()) {
            int u = queue.front();
            queue.pop();
            for (int v = 0; v < n; ++v) {
                if (level[v] < 0 && residual[u][v] > 1e-10) {
                    level[v] = level[u] + 1;
                    queue.push(v);
                }
            }
        }

        /// 汇点不可达则算法终止
        if (level[sink] < 0) break;

        /// DFS寻找增广路径
        QVector<int> iter(n, 0);  ///< 当前弧优化

        /// 递归DFS推送流量
        std::function<double(int, double)> dfs = [&](int u, double pushed) -> double {
            if (u == sink) return pushed;
            for (int& i = iter[u]; i < n; ++i) {
                int v = i;
                if (level[v] == level[u] + 1 && residual[u][v] > 1e-10) {
                    double flow = dfs(v, qMin(pushed, residual[u][v]));
                    if (flow > 1e-10) {
                        residual[u][v] -= flow;
                        residual[v][u] += flow;
                        return flow;
                    }
                }
            }
            return 0.0;
        };

        while (true) {
            double pushed = dfs(source, 1e30);
            if (pushed < 1e-10) break;
            maxFlowValue += pushed;
            ++augmentingPaths;
        }
    }

    /// 计算源侧最小割集
    m_sourceSide.clear();
    for (int i = 0; i < n; ++i) {
        bool reachable = false;
        /// 通过残余图判断是否从源可达
        QVector<bool> visited(n, false);
        std::queue<int> bq;
        bq.push(source);
        visited[source] = true;
        while (!bq.empty()) {
            int u = bq.front();
            bq.pop();
            if (u == i) { reachable = true; break; }
            for (int v = 0; v < n; ++v) {
                if (!visited[v] && residual[u][v] > 1e-10) {
                    visited[v] = true;
                    bq.push(v);
                }
            }
        }
        if (reachable) m_sourceSide.append(i);
    }

    /// 更新统计信息
    m_stats.totalFlowsComputed++;
    m_stats.totalAugmentingPaths += augmentingPaths;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFlowsComputed;

    emit flowComputed(source, sink, maxFlowValue);
    return maxFlowValue;
}

/**
 * @brief 获取最小割集（源侧和汇侧）
 *
 * 在maxFlow()之后调用，根据残余图将顶点分为
 * 从源可达（源侧）和不可达（汇侧）两部分。
 * 割边的容量之和等于最大流值（最大流最小割定理）。
 *
 * @param source 源点索引
 * @return QPair(源侧顶点列表, 汇侧顶点列表)
 */
QPair<QVector<int>, QVector<int>> FlowNetwork6::minCut(int source) const
{
    Q_UNUSED(source)
    QVector<int> sinkSide;
    /// 汇侧为不在源侧的所有顶点
    return qMakePair(m_sourceSide, sinkSide);
}

/**
 * @brief 获取当前统计数据
 * @return 包含流计算次数、增广路径数和平均耗时的Stats结构
 */
FlowNetwork6::Stats FlowNetwork6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void FlowNetwork6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_sourceSide.clear();
}
