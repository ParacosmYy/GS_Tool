/**
 * @file MinSpanningTree6.cpp
 * @brief 最小生成树实现 — Kruskal算法 + 并查集 + 连通性检查
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Kruskal 最小生成树算法。
 * 使用并查集（Union-Find）数据结构高效检测环路，
 * 按边权从小到大排序后贪心选边构建最小生成树。
 */

#include "utils/graph71/MinSpanningTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空图
 * @param parent 父QObject对象
 */
MinSpanningTree6::MinSpanningTree6(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("MinSpanningTree6"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置顶点数
 * @param n 顶点数量
 */
void MinSpanningTree6::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_parent.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_parent[i] = i;
    }
}

/**
 * @brief 添加一条带权无向边
 * @param u 起始顶点（0-based）
 * @param v 终止顶点（0-based）
 * @param weight 边权重
 */
void MinSpanningTree6::addEdge(int u, int v, double weight)
{
    m_edges.append({weight, {u, v}});
}

// ──────────────────────────────────────────────
// Kruskal MST 求解
// ──────────────────────────────────────────────

/**
 * @brief 使用 Kruskal 算法求解最小生成树
 *
 * 算法步骤：
 * 1. 将所有边按权重升序排序
 * 2. 初始化并查集，每个顶点独立
 * 3. 依次考虑每条边：
 *    - 如果边的两个端点不在同一集合中，加入 MST
 *    - 合并两个集合
 * 4. 检查 MST 是否包含所有顶点（连通性）
 *
 * @return MST 的边列表，每条边为 (u, v)
 */
QVector<QPair<int, int>> MinSpanningTree6::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    // 步骤1：按权重排序
    std::sort(m_edges.begin(), m_edges.end());

    // 步骤2：初始化并查集
    m_parent.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_parent[i] = i;
    }

    // 步骤3：贪心选边
    QVector<QPair<int, int>> mst;
    m_totalWeight = 0.0;

    for (const auto& edge : m_edges) {
        int u = edge.second.first;
        int v = edge.second.second;
        double w = edge.first;

        if (unionSets(u, v)) {
            mst.append({u, v});
            m_totalWeight += w;

            if (mst.size() == m_n - 1) break;
        }
    }

    // 步骤4：连通性检查
    m_connected = (mst.size() == m_n - 1);

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalEdges += m_edges.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(mst.size(), m_totalWeight);
    return mst;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含求解次数、总边数和平均耗时的Stats结构
 */
MinSpanningTree6::Stats MinSpanningTree6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void MinSpanningTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 并查集操作
// ──────────────────────────────────────────────

/**
 * @brief 查找元素所属集合的代表（路径压缩）
 *
 * 使用路径压缩优化：在查找过程中将沿途所有节点
 * 直接连接到根节点，使后续查找更快。
 *
 * @param v 要查找的元素
 * @return 集合代表（根节点）
 */
int MinSpanningTree6::findSet(int v)
{
    if (v < 0 || v >= m_parent.size()) return v;
    if (m_parent[v] != v) {
        m_parent[v] = findSet(m_parent[v]); // 路径压缩
    }
    return m_parent[v];
}

/**
 * @brief 合并两个元素所在的集合（按秩合并简化）
 *
 * 如果两个元素已在同一集合中则不操作（检测环路）。
 *
 * @param a 第一个元素
 * @param b 第二个元素
 * @return true 成功合并（不在同一集合），false 已在同一集合
 */
bool MinSpanningTree6::unionSets(int a, int b)
{
    int rootA = findSet(a);
    int rootB = findSet(b);
    if (rootA == rootB) return false;

    m_parent[rootA] = rootB;
    return true;
}

// ──────────────────────────────────────────────
// 辅助方法 — 连通分量分析
// ──────────────────────────────────────────────

/**
 * @brief 统计当前图中的连通分量数
 *
 * 使用并查集计算连通分量数。
 * 两个顶点属于同一连通分量当且仅当它们的根相同。
 *
 * @return 连通分量数
 */
int MinSpanningTree6::countComponents() const
{
    if (m_n == 0) return 0;

    QSet<int> roots;
    for (int v = 0; v < m_n; ++v) {
        // 使用 const_cast 因为 findSet 会修改 m_parent（路径压缩）
        MinSpanningTree6* self = const_cast<MinSpanningTree6*>(this);
        roots.insert(self->findSet(v));
    }
    return roots.size();
}

/**
 * @brief 获取所有边的信息（用于调试和可视化）
 *
 * 返回所有已添加的边及其权重，按权重升序排列。
 *
 * @return 边列表，每条边包含 (权重, (u, v))
 */
QVector<QPair<double, QPair<int, int>>> MinSpanningTree6::edges() const
{
    QVector<QPair<double, QPair<int, int>>> sorted = m_edges;
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}
