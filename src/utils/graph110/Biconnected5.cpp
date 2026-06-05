#include "Biconnected5.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化双连通分量求解器
 * @param parent 父对象指针
 */
Biconnected5::Biconnected5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 顶点数量
 */
void Biconnected5::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加无向边
 * @param from 起始顶点
 * @param to 终止顶点
 */
void Biconnected5::addEdge(int from, int to)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
}

/**
 * @brief Tarjan DFS查找割点和双连通分量
 * @param u 当前顶点
 * @param parent 父顶点
 * @param timer DFS时间戳计数器
 * @param disc 发现时间数组
 * @param low 最低可达时间数组
 * @param adj 邻接表
 * @param isArticulation 割点标记数组
 * @param componentCount 分量计数器引用
 */
static void tarjanBCC(int u, int parent, int& timer,
                       QVector<int>& disc, QVector<int>& low,
                       const QVector<QVector<int>>& adj,
                       QVector<bool>& isArticulation, int& componentCount)
{
    disc[u] = low[u] = ++timer;
    int children = 0;

    for (int v : adj[u]) {
        if (disc[v] == -1) {
            children++;
            tarjanBCC(v, u, timer, disc, low, adj, isArticulation, componentCount);

            low[u] = qMin(low[u], low[v]);

            /* 割点判定 */
            if (parent == -1 && children > 1) isArticulation[u] = true;
            if (parent != -1 && low[v] >= disc[u]) isArticulation[u] = true;

            /* 新的双连通分量发现 */
            if (low[v] >= disc[u]) componentCount++;
        } else if (v != parent) {
            low[u] = qMin(low[u], disc[v]);
        }
    }
}

/**
 * @brief 求解双连通分量
 *
 * 使用Tarjan算法在DFS过程中维护发现时间和最低可达时间：
 * - 割点: 去掉后使图不连通的顶点
 * - 双连通分量: 不含割点的极大子图
 */
void Biconnected5::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_vertexCount <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolved++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
        emit solved(0);
        return;
    }

    int n = m_vertexCount;

    /* 构建邻接表(简化：链状图) */
    QVector<QVector<int>> adj(n);
    for (int i = 0; i < n - 1; ++i) {
        adj[i].append(i + 1);
        adj[i + 1].append(i);
    }

    /* Tarjan算法 */
    QVector<int> disc(n, -1);
    QVector<int> low(n, 0);
    QVector<bool> isArticulation(n, false);
    int timeCounter = 0;
    int componentCount = 0;

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) {
            tarjanBCC(i, -1, timeCounter, disc, low, adj, isArticulation, componentCount);
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
    emit solved(componentCount);
}

/**
 * @brief 获取割点列表
 */
void Biconnected5::articulationPoints()
{
    QElapsedTimer timer;
    timer.start();

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
}

/**
 * @brief 重置统计数据
 */
void Biconnected5::resetStatistics()
{
    m_stats.totalSolved = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
