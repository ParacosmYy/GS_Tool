#include "MinSpanningTree8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化最小生成树求解器
 * @param parent 父对象指针
 */
MinSpanningTree8::MinSpanningTree8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param count 顶点数量
 */
void MinSpanningTree8::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
    m_edges.clear();
}

/**
 * @brief 添加一条带权无向边
 * @param from 起始顶点
 * @param to 终止顶点
 * @param weight 边权重
 */
void MinSpanningTree8::addEdge(int from, int to, double weight)
{
    if (from >= 0 && to >= 0 && from != to) {
        m_edges.append({{from, to}, weight});
    }
}

/**
 * @brief 并查集查找(带路径压缩)
 * @param parent 并查集父节点数组
 * @param x 待查找的节点
 * @return 根节点
 */
static int findParent(QVector<int>& parent, int x)
{
    if (parent[x] != x) parent[x] = findParent(parent, x);
    return parent[x];
}

/**
 * @brief 执行最小生成树求解(Kruskal算法)
 *
 * 将所有边按权重升序排列，依次选取不会形成环的边，
 * 使用并查集检测连通性，最终构成最小生成树。
 */
void MinSpanningTree8::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_vertexCount <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolves++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solved(0.0);
        return;
    }

    /* 按权重升序排列边 */
    auto sortedEdges = m_edges;
    std::sort(sortedEdges.begin(), sortedEdges.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    /* 初始化并查集 */
    QVector<int> parent(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) parent[i] = i;

    /* Kruskal贪心选择 */
    m_totalWeight = 0.0;
    int edgeCount = 0;

    for (const auto& edge : sortedEdges) {
        int u = edge.first.first;
        int v = edge.first.second;
        double w = edge.second;

        int rootU = findParent(parent, u);
        int rootV = findParent(parent, v);

        if (rootU != rootV) {
            parent[rootU] = rootV;
            m_totalWeight += w;
            edgeCount++;
            if (edgeCount == m_vertexCount - 1) break;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalSolves++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solved(m_totalWeight);
}

/**
 * @brief 获取最小生成树的总权重
 * @return 总权重值
 */
double MinSpanningTree8::totalWeight() const
{
    return m_totalWeight;
}

/**
 * @brief 获取图的连通性检查
 * @return true表示图是连通的
 */
bool MinSpanningTree8::isConnected() const
{
    return m_edges.size() >= m_vertexCount - 1;
}

/**
 * @brief 获取图的边数量
 * @return 边数量
 */
int MinSpanningTree8::edgeCount() const
{
    return m_edges.size();
}

/**
 * @brief 计算图的平均边权重
 * @return 平均权重
 */
double MinSpanningTree8::averageEdgeWeight() const
{
    if (m_edges.isEmpty()) return 0.0;
    double sum = 0.0;
    for (const auto& e : m_edges) sum += e.second;
    return sum / m_edges.size();
}

/**
 * @brief 重置统计数据
 */
void MinSpanningTree8::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
