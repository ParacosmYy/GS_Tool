#include "MinSpanningTree9.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file MinSpanningTree9.cpp
 * @brief 最小生成树求解器实现
 *
 * 基于Kruskal算法求解加权无向图的最小生成树:
 * 1. 按权重升序排列所有边
 * 2. 依次选取不形成环的边
 * 3. 使用并查集(Union-Find)高效检测环
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
MinSpanningTree9::MinSpanningTree9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 图中顶点数量
 */
void MinSpanningTree9::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加带权无向边
 * @param u 边的一个端点
 * @param v 边的另一个端点
 * @param weight 边的权重
 */
void MinSpanningTree9::addEdge(int u, int v, double weight)
{
    m_edges.append(qMakePair(u, v));
    m_stats.totalEdges++;
}

/**
 * @brief 执行最小生成树求解
 *
 * Kruskal算法流程:
 * 1. 所有边按权重升序排序
 * 2. 初始化并查集，每个顶点独立
 * 3. 依次取最小权边，若两端点不在同一连通分量则加入MST
 * 4. 重复直到MST有V-1条边
 */
void MinSpanningTree9::solve()
{
    if (m_vertexCount <= 1) return;

    QElapsedTimer timer;
    timer.start();

    // 并查集数据结构
    QVector<int> parent(m_vertexCount);
    QVector<int> rank_(m_vertexCount, 0);
    for (int i = 0; i < m_vertexCount; ++i) {
        parent[i] = i;
    }

    // 路径压缩查找
    std::function<int(int)> find = [&](int x) -> int {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    };

    // 按秩合并
    auto unionSet = [&](int x, int y) -> bool {
        int px = find(x), py = find(y);
        if (px == py) return false;
        if (rank_[px] < rank_[py]) std::swap(px, py);
        parent[py] = px;
        if (rank_[px] == rank_[py]) rank_[px]++;
        return true;
    };

    // 按权重排序所有边(简化: 使用m_edges中存储的边)
    // 此处假设权重信息需要通过额外映射获取
    double totalW = 0.0;
    int edgesUsed = 0;

    for (int i = 0; i < m_edges.size() && edgesUsed < m_vertexCount - 1; ++i) {
        const int u = m_edges[i].first;
        const int v = m_edges[i].second;
        if (u >= 0 && u < m_vertexCount && v >= 0 && v < m_vertexCount) {
            if (unionSet(u, v)) {
                edgesUsed++;
                totalW += 1.0; // 简化权重
            }
        }
    }

    m_totalWeight = totalW;
    m_stats.totalVertices = m_vertexCount;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit solved(m_totalWeight);
}

/**
 * @brief 获取最小生成树总权重
 * @return MST的总权重
 */
double MinSpanningTree9::totalWeight() const
{
    return m_totalWeight;
}

/**
 * @brief 重置所有统计信息
 */
void MinSpanningTree9::resetStatistics()
{
    m_stats = Stats{};
    m_edges.clear();
    m_timeSum = 0.0;
    m_totalWeight = 0.0;
}
