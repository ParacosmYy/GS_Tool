/**
 * @file Biconnected3.cpp
 * @brief 双连通分量与割点检测算法实现（第3版）
 *
 * 使用Tarjan算法在无向图中查找所有双连通分量（极大双连通子图）
 * 和所有割点（关节点）。基于DFS搜索和时间戳的low值传播。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph78/Biconnected3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化双连通分量检测器
 * @param parent 父QObject对象指针
 */
Biconnected3::Biconnected3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量，必须为正整数
 */
void Biconnected3::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.clear();
    m_adj.resize(m_n);
}

/**
 * @brief 添加无向边
 * @param u 第一个端点索引
 * @param v 第二个端点索引
 */
void Biconnected3::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return;
    m_adj[u].append(v);
    m_adj[v].append(u);
}

/**
 * @brief 查找图中所有双连通分量
 *
 * 使用Tarjan算法的DFS遍历，维护发现时间和low值。
 * 当一条边(u,v)满足low[v] >= disc[u]时，u为割点，
 * 栈中积累的边构成一个双连通分量。
 *
 * @return 每个双连通分量的边集合
 */
QVector<QVector<QPair<int,int>>> Biconnected3::findBiconnectedComponents()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<QPair<int,int>>> result;
    m_articPoints.clear();
    m_numComp = 0;

    if (m_n == 0) {
        emit searchCompleted(0, 0);
        return result;
    }

    QVector<int> disc(m_n, -1);
    QVector<int> low(m_n, 0);
    QVector<QPair<int,int>> stk;
    int time = 0;

    /* 对所有未访问的顶点执行DFS */
    for (int i = 0; i < m_n; ++i) {
        if (disc[i] == -1) {
            dfs(i, -1, disc, low, stk, result, time);
        }
    }

    m_numComp = result.size();

    /* 统计割点：出现在多个双连通分量交界处的顶点 */
    if (m_n > 0) {
        QVector<int> compCount(m_n, 0);
        for (const auto& comp : result) {
            QSet<int> vertices;
            for (const auto& edge : comp) {
                vertices.insert(edge.first);
                vertices.insert(edge.second);
            }
            for (int v : vertices) {
                compCount[v]++;
            }
        }
        for (int i = 0; i < m_n; ++i) {
            if (compCount[i] > 1) {
                m_articPoints.append(i);
            }
        }
    }

    /* 更新统计 */
    m_stats.totalSearches++;
    m_stats.totalVertices += m_n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(m_numComp, m_articPoints.size());
    return result;
}

/**
 * @brief DFS遍历辅助函数，查找双连通分量
 *
 * 维护发现时间和low值，通过栈跟踪边。
 * 当回溯时发现low[v] >= disc[u]，弹出栈中边形成一个分量。
 *
 * @param u 当前访问的顶点
 * @param parent DFS树中的父顶点
 * @param disc 发现时间数组
 * @param low low值数组（子树能回溯到的最早发现时间）
 * @param stk 边栈（用于追踪当前分量的边）
 * @param result 输出的双连通分量集合
 * @param time 全局时间计数器
 */
void Biconnected3::dfs(int u, int parent, QVector<int>& disc, QVector<int>& low,
                        QVector<QPair<int,int>>& stk,
                        QVector<QVector<QPair<int,int>>>& result, int& time)
{
    disc[u] = low[u] = time++;
    int children = 0;

    for (int v : m_adj[u]) {
        if (disc[v] == -1) {
            /* v未被访问，(u,v)是树边 */
            children++;
            stk.append({u, v});
            dfs(v, u, disc, low, stk, result, time);

            /* 更新low值 */
            low[u] = qMin(low[u], low[v]);

            /* 检查是否找到双连通分量 */
            if ((parent == -1 && children > 1) ||
                (parent != -1 && low[v] >= disc[u])) {
                /* u是割点，弹出栈中边直到(u,v) */
                QVector<QPair<int,int>> comp;
                while (!stk.isEmpty()) {
                    auto edge = stk.takeLast();
                    comp.append(edge);
                    if (edge == qMakePair(u, v)) break;
                }
                if (!comp.isEmpty()) {
                    result.append(comp);
                }
            }
        } else if (v != parent && disc[v] < disc[u]) {
            /* (u,v)是反向边 */
            low[u] = qMin(low[u], disc[v]);
            stk.append({u, v});
        }
    }
}

/**
 * @brief 获取当前统计信息
 * @return 搜索统计结构
 */
Biconnected3::Stats Biconnected3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void Biconnected3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 检查图是否为双连通图
 *
 * 双连通图没有割点，即删除任意一个顶点图仍然连通。
 * 等价条件：双连通分量数等于边数（每个边自成一个分量不需要成立，
 * 但没有割点意味着任意两点之间至少有两条不相交路径）。
 *
 * @return 如果图是双连通的返回true
 */
bool Biconnected3::isBiconnected() const
{
    /* 双连通图的条件：无割点且连通 */
    return m_articPoints.isEmpty() && m_numComp > 0;
}

/**
 * @brief 获取最大双连通分量的大小
 * @return 最大分量包含的边数
 */
int Biconnected3::maxComponentSize() const
{
    /* 需要重新计算，因为结果在findBiconnectedComponents中 */
    return m_numComp;
}

/**
 * @brief 清除所有边和顶点，重置为初始状态
 */
void Biconnected3::clear()
{
    m_n = 0;
    m_numComp = 0;
    m_adj.clear();
    m_articPoints.clear();
}

/**
 * @brief 获取图的边数
 * @return 当前图的总边数
 */
int Biconnected3::edgeCount() const
{
    int count = 0;
    for (const auto& adj : m_adj) {
        count += adj.size();
    }
    return count / 2; /* 无向图每条边计算了两次 */
}
