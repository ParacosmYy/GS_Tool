/**
 * @file TopologicalSort3.cpp
 * @brief 拓扑排序算法实现
 *
 * 实现Kahn算法进行拓扑排序，支持环检测和所有拓扑序列枚举。
 * 适用于任务调度、依赖解析、编译顺序确定等有向无环图(DAG)场景。
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
    // 避免重复边
    if (!m_adj[u].contains(v)) {
        m_adj[u].append(v);
        m_inDegree[v]++;
    }
}

/**
 * @brief 执行拓扑排序（Kahn算法）
 * @return 拓扑排序结果，若存在环则返回空
 *
 * Kahn算法步骤：
 * 1. 计算所有顶点的入度
 * 2. 将入度为0的顶点入队
 * 3. 依次出队，将其邻接顶点的入度减1
 * 4. 若入度变为0则入队
 * 5. 若最终排序结果不足n个顶点，说明存在环
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

    // 复制入度数组（不修改原始数据）
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

        // 减少邻接顶点的入度
        for (int v : m_adj[u]) {
            inDeg[v]--;
            if (inDeg[v] == 0) {
                queue.enqueue(v);
            }
        }
    }

    // 环检测：如果排序结果不足n个，说明存在环
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
 * 每一步选择一个入度为0的未访问顶点，递归搜索，
 * 回溯时恢复状态。注意：序列数量可能呈指数增长。
 */
QVector<QVector<int>> TopologicalSort3::allTopologicalSorts()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> allResults;
    if (m_n == 0) return allResults;

    // 先检测是否有环
    QVector<int> sorted = this->sort();
    if (m_hasCycle) return allResults;

    // DFS回溯枚举
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
 *
 * 递归选择入度为0的未访问顶点，加入结果序列后
 * 更新邻接顶点的入度，递归完成后回溯恢复状态。
 */
void TopologicalSort3::allSortsDFS(QVector<int>& result, QVector<bool>& visited, QVector<int>& inDeg)
{
    if (result.size() == m_n) {
        m_numSorts++;
        return; // 找到一个完整序列（仅计数，不存储）
    }

    for (int i = 0; i < m_n; ++i) {
        // 选择入度为0且未访问的顶点
        if (!visited[i] && inDeg[i] == 0) {
            // 做出选择：将顶点i加入当前序列
            result.append(i);
            visited[i] = true;

            // 减少邻接顶点的入度
            for (int v : m_adj[i]) inDeg[v]--;

            // 递归搜索下一层
            allSortsDFS(result, visited, inDeg);

            // 回溯：恢复所有状态
            for (int v : m_adj[i]) inDeg[v]++;
            visited[i] = false;
            result.removeLast();
        }
    }
}

/**
 * @brief 获取指定顶点的入度
 * @param v 顶点编号
 * @return 该顶点的入度
 */
int TopologicalSort3::inDegree(int v) const
{
    if (v < 0 || v >= m_n) return 0;
    return m_inDegree[v];
}

/**
 * @brief 获取指定顶点的出度
 * @param v 顶点编号
 * @return 该顶点的出度（邻接顶点数）
 */
int TopologicalSort3::outDegree(int v) const
{
    if (v < 0 || v >= m_adj.size()) return 0;
    return m_adj[v].size();
}

/**
 * @brief 获取图中总边数
 * @return 有向边总数
 */
int TopologicalSort3::edgeCount() const
{
    int count = 0;
    for (const auto& adj : m_adj) {
        count += adj.size();
    }
    return count;
}
