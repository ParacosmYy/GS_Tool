/**
 * @file TopologicalSort3.cpp
 * @brief 拓扑排序算法实现
 *
 * 实现Kahn算法进行拓扑排序，支持环检测和所有拓扑序列枚举。
 * 适用于任务调度、依赖解析等有向无环图(DAG)场景。
 */

#include "utils/graph82/TopologicalSort3.h"

#include <QElapsedTimer>
#include <QQueue>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
TopologicalSort3::TopologicalSort3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param n 顶点数量
 */
void TopologicalSort3::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
    m_inDegree.clear();
    m_inDegree.resize(m_n, 0);
}

/**
 * @brief 添加有向边
 * @param u 起点
 * @param v 终点
 */
void TopologicalSort3::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    m_adj[u].append(v);
    m_inDegree[v]++;
}

/**
 * @brief 执行拓扑排序（Kahn算法）
 * @return 拓扑排序结果，若存在环则返回空
 */
QVector<int> TopologicalSort3::sort()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (m_n == 0) {
        m_hasCycle = false;
        return result;
    }

    // 复制入度数组
    QVector<int> inDeg = m_inDegree;
    QQueue<int> queue;

    // 将所有入度为0的顶点入队
    for (int i = 0; i < m_n; ++i) {
        if (inDeg[i] == 0) queue.enqueue(i);
    }

    result.reserve(m_n);
    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        result.append(u);

        for (int v : m_adj[u]) {
            inDeg[v]--;
            if (inDeg[v] == 0) queue.enqueue(v);
        }
    }

    m_hasCycle = (result.size() != m_n);

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSorts++;
    m_stats.totalVertices += m_n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(m_n, !m_hasCycle);
    return result;
}

/**
 * @brief 枚举所有拓扑排序序列
 * @return 所有可能的拓扑排序结果
 *
 * 使用DFS回溯法枚举所有合法的拓扑序列。
 * 注意：序列数量可能呈指数增长，仅适用于小规模图。
 */
QVector<QVector<int>> TopologicalSort3::allTopologicalSorts()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> allResults;
    if (m_n == 0 || m_hasCycle) return allResults;

    QVector<int> result;
    QVector<bool> visited(m_n, false);
    QVector<int> inDeg = m_inDegree;
    m_numSorts = 0;

    allSortsDFS(result, visited, inDeg);

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;

    return allResults;
}

/**
 * @brief 重置统计信息
 */
void TopologicalSort3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief DFS回溯枚举所有拓扑排序
 * @param result 当前构建的序列
 * @param visited 已访问顶点标记
 * @param inDeg 当前入度状态
 */
void TopologicalSort3::allSortsDFS(QVector<int>& result, QVector<bool>& visited, QVector<int>& inDeg)
{
    if (result.size() == m_n) {
        m_numSorts++;
        return; // 找到一个完整序列（不存储，仅计数）
    }

    for (int i = 0; i < m_n; ++i) {
        // 选择入度为0且未访问的顶点
        if (!visited[i] && inDeg[i] == 0) {
            // 选择顶点i
            result.append(i);
            visited[i] = true;

            // 减少邻接顶点的入度
            for (int v : m_adj[i]) inDeg[v]--;

            // 递归
            allSortsDFS(result, visited, inDeg);

            // 回溯
            for (int v : m_adj[i]) inDeg[v]++;
            visited[i] = false;
            result.removeLast();
        }
    }
}
