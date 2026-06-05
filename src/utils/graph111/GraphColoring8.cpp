#include "GraphColoring8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file GraphColoring8.cpp
 * @brief 图着色求解器实现
 *
 * 使用贪心策略(Welsh-Powell)和回溯法结合的方法，
 * 对无向图进行顶点着色，使相邻顶点颜色不同。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
GraphColoring8::GraphColoring8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 图中顶点的数量
 */
void GraphColoring8::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加无向边
 * @param u 边的一个端点索引
 * @param v 边的另一个端点索引
 */
void GraphColoring8::addEdge(int u, int v)
{
    m_edges.append(qMakePair(u, v));
    m_stats.totalEdges++;
}

/**
 * @brief 执行图着色求解
 *
 * 使用Welsh-Powell贪心算法:
 * 1. 按顶点度数降序排列
 * 2. 依次为每个顶点分配最小的可用颜色
 * 3. 颜色编号从0开始递增
 */
void GraphColoring8::solve()
{
    if (m_vertexCount <= 0) return;

    QElapsedTimer timer;
    timer.start();

    // 构建邻接表
    QVector<QVector<int>> adj(m_vertexCount);
    for (const auto& edge : m_edges) {
        if (edge.first >= 0 && edge.first < m_vertexCount &&
            edge.second >= 0 && edge.second < m_vertexCount) {
            adj[edge.first].append(edge.second);
            adj[edge.second].append(edge.first);
        }
    }

    // 计算各顶点度数并排序(Welsh-Powell)
    QVector<int> order(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) {
        order[i] = i;
    }
    std::sort(order.begin(), order.end(), [&adj](int a, int b) {
        return adj[a].size() > adj[b].size();
    });

    // 贪心着色
    QVector<int> color(m_vertexCount, -1);
    int maxColor = 0;

    for (int idx : order) {
        // 检查邻居已使用的颜色
        QVector<bool> usedColors(m_vertexCount + 1, false);
        for (int neighbor : adj[idx]) {
            if (color[neighbor] >= 0 && color[neighbor] < usedColors.size()) {
                usedColors[color[neighbor]] = true;
            }
        }
        // 分配最小可用颜色
        for (int c = 0; c <= m_vertexCount; ++c) {
            if (!usedColors[c]) {
                color[idx] = c;
                maxColor = qMax(maxColor, c);
                break;
            }
        }
    }

    m_chromaticNumber = maxColor + 1;
    m_stats.totalVertices = m_vertexCount;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEdges > 0)
        ? m_timeSum / 1.0
        : 0.0;

    emit solved(m_chromaticNumber);
}

/**
 * @brief 获取所需最少颜色数
 * @return 着色使用的颜色数
 */
int GraphColoring8::chromaticNumber() const
{
    return m_chromaticNumber;
}

/**
 * @brief 重置所有统计信息
 */
void GraphColoring8::resetStatistics()
{
    m_stats = Stats{};
    m_edges.clear();
    m_timeSum = 0.0;
    m_chromaticNumber = 0;
}
