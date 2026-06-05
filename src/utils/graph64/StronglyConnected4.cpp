/**
 * @file StronglyConnected4.cpp
 * @brief 强连通分量(SCC)搜索算法实现
 *
 * 实现Tarjan强连通分量算法，用于在有向图中查找所有
 * 最大强连通子图。同时支持缩图(DAG)生成和强连通性检测。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/graph64/StronglyConnected4.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @class StronglyConnected4
 * @brief Tarjan强连通分量搜索器
 *
 * Tarjan算法基于DFS，使用disc(发现时间)和low(可回溯到的
 * 最早祖先)数组识别强连通分量。当low[u]==disc[u]时，
 * 从栈中弹出到u的所有节点构成一个SCC。时间复杂度O(V+E)。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject指针
 */
StronglyConnected4::StronglyConnected4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置有向图的邻接关系
 * @param n 顶点数量
 * @param edges 有向边列表，每条边为(起点, 终点)对
 */
void StronglyConnected4::setGraph(int n, const QVector<QPair<int,int>>& edges)
{
    m_n = n;
    m_adj.assign(n, QVector<int>());
    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < n &&
            edge.second >= 0 && edge.second < n) {
            m_adj[edge.first].append(edge.second);
        }
    }
    m_sccs.clear();
}

/**
 * @brief 查找所有强连通分量
 *
 * 执行Tarjan算法遍历所有顶点，对未访问的顶点执行DFS。
 * 每次DFS维护一个递归栈，当发现low[u]==disc[u]时
 * 从栈中弹出一个完整的SCC。
 *
 * @return 所有强连通分量的列表，每个SCC为顶点索引向量
 */
QVector<QVector<int>> StronglyConnected4::findSCCs()
{
    QElapsedTimer timer;
    timer.start();

    m_sccs.clear();

    if (m_n <= 0) {
        m_stats.totalSearches++;
        m_stats.totalVertices += m_n;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalSearches > 0)
            ? m_timeSum / m_stats.totalSearches : 0.0;
        return m_sccs;
    }

    /* Tarjan算法所需的数据结构 */
    QVector<int> disc(m_n, -1);    /* 发现时间，-1表示未访问 */
    QVector<int> low(m_n, 0);      /* low值：可回溯的最早祖先 */
    QVector<bool> onStack(m_n, false);
    QVector<int> stack;
    int timerCounter = 0;

    /* 对每个未访问顶点启动DFS */
    for (int i = 0; i < m_n; ++i) {
        if (disc[i] == -1) {
            tarjanSCC(i, disc, low, onStack, stack, timerCounter);
        }
    }

    m_stats.totalSearches++;
    m_stats.totalVertices += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSearches > 0)
        ? m_timeSum / m_stats.totalSearches : 0.0;

    emit searchCompleted(m_sccs.size());
    return m_sccs;
}

/**
 * @brief 获取缩图(DAG)表示
 *
 * 将每个SCC缩为一个超节点，生成SCC之间的DAG拓扑。
 * 返回值数组中，索引为顶点编号，值为所属SCC的编号。
 *
 * @return 每个顶点所属的SCC编号（0~SCC数-1）
 */
QVector<int> StronglyConnected4::condensationDAG() const
{
    QVector<int> compMap(m_n, -1);
    for (int i = 0; i < m_sccs.size(); ++i) {
        for (int v : m_sccs[i]) {
            compMap[v] = i;
        }
    }
    return compMap;
}

/**
 * @brief 检查图是否为强连通图（整个图只有一个SCC）
 * @return 若所有顶点属于同一个SCC返回true
 */
bool StronglyConnected4::isStronglyConnected() const
{
    return m_sccs.size() == 1 && m_n > 0;
}

/**
 * @brief 重置统计数据
 */
void StronglyConnected4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief Tarjan算法的DFS递归函数
 *
 * 对顶点u执行DFS，维护disc和low数组。
 * 当low[u]==disc[u]时，从栈中弹出到u的所有顶点
 * 构成一个强连通分量。
 *
 * @param u 当前访问的顶点
 * @param disc 发现时间数组
 * @param low low值数组
 * @param onStack 是否在栈上的标记数组
 * @param stack 递归栈
 * @param timer 全局时间计数器
 */
void StronglyConnected4::tarjanSCC(int u, QVector<int>& disc, QVector<int>& low,
                                    QVector<bool>& onStack, QVector<int>& stack, int& timer)
{
    disc[u] = low[u] = timer++;
    stack.append(u);
    onStack[u] = true;

    /* 遍历u的所有邻居 */
    for (int v : m_adj[u]) {
        if (disc[v] == -1) {
            /* v未访问：递归访问 */
            tarjanSCC(v, disc, low, onStack, stack, timer);
            low[u] = qMin(low[u], low[v]);
        } else if (onStack[v]) {
            /* v在栈中：找到回边或横叉边 */
            low[u] = qMin(low[u], disc[v]);
        }
    }

    /* 若u是SCC的根节点，弹出栈中所有属于此SCC的顶点 */
    if (low[u] == disc[u]) {
        QVector<int> scc;
        int v;
        do {
            v = stack.takeLast();
            onStack[v] = false;
            scc.append(v);
        } while (v != u);

        m_sccs.append(scc);
    }
}

/**
 * @brief 获取缩图中的所有跨SCC边
 *
 * 在找到所有SCC之后，扫描原图中的每条边，
 * 若边的两个端点属于不同的SCC，则该边是缩图DAG中
 * 的一条跨分量边。用于分析SCC之间的依赖关系。
 *
 * @return 跨SCC的边列表，每条边为(源SCC, 目标SCC)对
 */
QVector<QPair<int,int>> StronglyConnected4::crossComponentEdges() const
{
    QVector<QPair<int,int>> crossEdges;
    QVector<int> compMap = condensationDAG();

    /* 遍历所有顶点的邻接表 */
    for (int u = 0; u < m_n; ++u) {
        int compU = compMap[u];
        if (compU < 0) continue;
        for (int v : m_adj[u]) {
            int compV = compMap[v];
            if (compV < 0 || compU == compV) continue;
            /* 检查是否已存在相同的跨分量边 */
            bool exists = false;
            for (const auto& edge : crossEdges) {
                if (edge.first == compU && edge.second == compV) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                crossEdges.append({compU, compV});
            }
        }
    }
    return crossEdges;
}

/**
 * @brief 获取指定SCC的顶点列表
 * @param index SCC索引（0~numSCCs-1）
 * @return 该SCC中的顶点列表，越界返回空列表
 */
QVector<int> StronglyConnected4::getComponent(int index) const
{
    if (index >= 0 && index < m_sccs.size()) {
        return m_sccs[index];
    }
    return QVector<int>();
}

/**
 * @brief 获取最大的强连通分量（顶点数最多）
 * @return 最大SCC的顶点列表
 */
QVector<int> StronglyConnected4::largestComponent() const
{
    QVector<int> largest;
    for (const auto& scc : m_sccs) {
        if (scc.size() > largest.size()) {
            largest = scc;
        }
    }
    return largest;
}
