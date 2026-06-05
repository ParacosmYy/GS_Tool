/**
 * @file GraphColoring5.cpp
 * @brief 图着色算法实现 (回溯法 + 色数搜索)
 *
 * 使用回溯法进行图的着色，并通过二分搜索确定最小色数(色数)。
 * 从最少颜色数开始递增尝试，直到找到可行的着色方案。
 * 使用邻接表存储图结构，支持动态添加边。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/graph73/GraphColoring5.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化图着色求解器
 * @param parent 父QObject指针
 */
GraphColoring5::GraphColoring5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量
 *
 * 调用此方法会重置图的所有边和邻接关系
 */
void GraphColoring5::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.resize(m_n);
    for (auto& row : m_adj) {
        row.clear();
    }
}

/**
 * @brief 添加一条无向边
 * @param u 第一个顶点索引 (0-based)
 * @param v 第二个顶点索引 (0-based)
 *
 * 如果u或v超出范围，该操作被忽略
 */
void GraphColoring5::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) {
        return;
    }

    /* 避免重复添加同一条边 */
    if (!m_adj[u].contains(v)) {
        m_adj[u].append(v);
        m_adj[v].append(u);
    }
}

/**
 * @brief 执行图着色，搜索最小色数
 *
 * 从1种颜色开始递增尝试，使用回溯法判断给定颜色数是否可行。
 * 找到第一个可行方案即为最优(最小色数)。
 *
 * @return 每个顶点的颜色编号 (0 ~ chromatic-1)，失败返回空向量
 */
QVector<int> GraphColoring5::color()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> colors(m_n, -1);

    if (m_n == 0) {
        m_chromatic = 0;
        m_valid = true;
        emit coloringCompleted(0, true);
        return colors;
    }

    /* 空图只需要1种颜色 */
    bool hasEdges = false;
    for (int i = 0; i < m_n && !hasEdges; ++i) {
        if (!m_adj[i].isEmpty()) hasEdges = true;
    }

    if (!hasEdges) {
        m_chromatic = 1;
        m_valid = true;
        for (int i = 0; i < m_n; ++i) {
            colors[i] = 0;
        }
        m_stats.totalColorings++;
        m_stats.totalVertices += m_n;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;
        emit coloringCompleted(1, true);
        return colors;
    }

    /* 计算最大度数，作为色数的上界 */
    int maxDegree = 0;
    for (int i = 0; i < m_n; ++i) {
        maxDegree = qMax(maxDegree, m_adj[i].size());
    }

    /* 从1种颜色到maxDegree+1种颜色逐步尝试 */
    m_valid = false;
    m_chromatic = maxDegree + 1;

    for (int numColors = 1; numColors <= maxDegree + 1; ++numColors) {
        QVector<int> trial(m_n, -1);
        if (tryColoring(numColors, trial)) {
            colors = trial;
            m_chromatic = numColors;
            m_valid = true;
            break;
        }
    }

    /* 更新统计 */
    m_stats.totalColorings++;
    m_stats.totalVertices += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalColorings;

    bool optimal = (m_chromatic <= maxDegree);
    emit coloringCompleted(m_chromatic, optimal);

    return colors;
}

/**
 * @brief 重置所有统计数据
 */
void GraphColoring5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 使用回溯法尝试用指定数量的颜色着色
 *
 * 递归地对每个顶点尝试分配每种可用颜色:
 * 1. 检查当前颜色是否与已着色的邻居冲突
 * 2. 如果无冲突，分配颜色并递归处理下一个顶点
 * 3. 如果所有颜色都冲突，回溯到上一个顶点
 *
 * @param maxColors 最大允许颜色数
 * @param colors 颜色分配数组 (输入输出参数)
 * @return 是否找到可行的着色方案
 */
bool GraphColoring5::tryColoring(int maxColors, QVector<int>& colors)
{
    /* 初始化所有顶点为未着色 */
    for (int i = 0; i < m_n; ++i) {
        colors[i] = -1;
    }

    /* 使用栈模拟递归回溯，避免声明额外的成员函数 */
    int vertex = 0;
    while (vertex >= 0 && vertex < m_n) {
        bool found = false;
        /* 从上一个尝试的颜色之后继续 (colors[vertex]初始为-1) */
        int startC = (colors[vertex] < 0) ? 0 : colors[vertex] + 1;

        for (int c = startC; c < maxColors; ++c) {
            if (isSafe(vertex, c, colors)) {
                colors[vertex] = c;
                vertex++;
                found = true;
                break;
            }
        }

        if (!found) {
            /* 回溯: 当前顶点无可行颜色 */
            colors[vertex] = -1;
            vertex--;
        }
    }

    return (vertex >= m_n);
}

/**
 * @brief 检查给顶点v分配颜色c是否安全(不冲突)
 * @param v 顶点索引
 * @param c 候选颜色
 * @param colors 当前颜色分配
 * @return 如果没有冲突返回true，否则false
 */
bool GraphColoring5::isSafe(int v, int c, const QVector<int>& colors) const
{
    for (int neighbor : m_adj[v]) {
        if (colors[neighbor] == c) {
            return false;
        }
    }
    return true;
}
