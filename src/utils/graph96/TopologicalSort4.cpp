#include "TopologicalSort4.h"
#include <QElapsedTimer>
#include <queue>

/**
 * @brief 构造函数，初始化拓扑排序器
 * @param parent 父QObject对象指针
 */
TopologicalSort4::TopologicalSort4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Kahn算法拓扑排序
 *
 * 基于入度计数的有向无环图拓扑排序：
 * 1. 计算所有顶点的入度
 * 2. 将入度为0的顶点加入队列
 * 3. 依次取出队首顶点，将其所有邻居入度减1
 * 4. 若邻居入度变为0则加入队列
 * 5. 若最终排序结果不包含所有顶点，说明存在环
 *
 * @param adjacency 邻接表，adjacency[i]为顶点i指向的顶点列表
 * @return 拓扑排序结果，若存在环则返回不完整序列
 */
QVector<int> TopologicalSort4::sort(const QVector<QVector<int>>& adjacency)
{
    QElapsedTimer timer;
    timer.start();

    const int n = adjacency.size();
    if (n == 0) return {};

    /// 计算入度数组
    QVector<int> inDegree(n, 0);
    for (int u = 0; u < n; ++u) {
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) {
                inDegree[v]++;
            }
        }
    }

    /// 初始化队列：收集所有入度为0的顶点
    std::queue<int> queue;
    for (int i = 0; i < n; ++i) {
        if (inDegree[i] == 0) {
            queue.push(i);
        }
    }

    /// Kahn算法主循环
    QVector<int> result;
    result.reserve(n);

    while (!queue.empty()) {
        int u = queue.front();
        queue.pop();
        result.append(u);

        /// 遍历u的所有邻居，减少入度
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) {
                --inDegree[v];
                if (inDegree[v] == 0) {
                    queue.push(v);
                }
            }
        }
    }

    /// 检测是否存在环
    bool hasCycleFlag = (result.size() != n);

    /// 更新统计信息
    m_stats.totalSorts++;
    if (hasCycleFlag) m_stats.totalCyclesDetected++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(n, hasCycleFlag);
    return result;
}

/**
 * @brief 检测有向图中是否存在环
 *
 * 使用Kahn算法的变体：执行拓扑排序后检查是否所有顶点
 * 都被处理。若有顶点未被处理，说明存在环。
 *
 * @param adjacency 邻接表
 * @return true如果存在环，false如果为DAG
 */
bool TopologicalSort4::hasCycle(const QVector<QVector<int>>& adjacency) const
{
    const int n = adjacency.size();
    if (n == 0) return false;

    /// 计算入度
    QVector<int> inDegree(n, 0);
    for (int u = 0; u < n; ++u) {
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) inDegree[v]++;
        }
    }

    /// 模拟Kahn算法统计处理顶点数
    std::queue<int> queue;
    for (int i = 0; i < n; ++i) {
        if (inDegree[i] == 0) queue.push(i);
    }

    int processed = 0;
    while (!queue.empty()) {
        int u = queue.front();
        queue.pop();
        ++processed;
        for (int v : adjacency[u]) {
            if (v >= 0 && v < n) {
                if (--inDegree[v] == 0) queue.push(v);
            }
        }
    }

    return processed != n;
}

/**
 * @brief 获取当前统计数据
 * @return 包含排序次数、环检测次数和平均耗时的Stats结构
 */
TopologicalSort4::Stats TopologicalSort4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void TopologicalSort4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
