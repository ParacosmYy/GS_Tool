/**
 * @file EdgeColoring2.cpp
 * @brief 边着色算法实现 — 基于Vizing定理的图边着色
 *
 * 实现无向图的边着色算法:
 * - Vizing定理: 任何简单图的边色数为 Δ 或 Δ+1 (Δ为最大度数)
 * - 使用贪心策略+冲突消解实现近似最优着色
 * - 支持着色有效性验证
 * - 统计着色次数、处理边数、使用颜色数、平均耗时
 *
 * 算法流程:
 * 1. 计算每个顶点的度数
 * 2. 按最大度数确定颜色上界(Δ+1)
 * 3. 逐边贪心着色，优先选择不冲突的最小颜色
 * 4. 冲突时使用 Kempe链交换消解
 */

#include "utils/graph52/EdgeColoring2.h"

#include <QElapsedTimer>
#include <QtGlobal>

#include <algorithm>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 初始化边着色器
 * @param parent QObject父对象
 */
EdgeColoring2::EdgeColoring2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置图的拓扑结构
 * @param n 顶点数
 * @param edges 边列表，每对表示一条无向边的两个端点
 */
void EdgeColoring2::setGraph(int n, const QVector<QPair<int, int>>& edges)
{
    m_n = qMax(0, n);
    m_edges = edges;
}

/**
 * @brief 执行边着色 — 为每条边分配颜色
 *
 * 算法:
 * 1. 计算最大度数Δ，确定颜色范围[0, Δ]
 * 2. 维护每个顶点已使用的颜色集合
 * 3. 逐边贪心着色: 选择两端点都未使用的最小颜色
 * 4. 若无可用颜色，扩展颜色范围至Δ+1
 *
 * @return 每条边的颜色编号(从0开始)，-1表示着色失败
 */
QVector<int> EdgeColoring2::color()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;

    if (m_n <= 0 || m_edges.isEmpty()) {
        m_stats.totalColorings++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            (m_stats.totalColorings > 0) ? m_timeSum / m_stats.totalColorings : 0.0;
        return result;
    }

    const int edgeCount = m_edges.size();
    result.resize(edgeCount);
    result.fill(-1);

    /* 步骤1: 计算每个顶点的度数和最大度数 */
    QVector<int> degree(m_n, 0);
    for (int i = 0; i < edgeCount; ++i) {
        degree[m_edges[i].first]++;
        degree[m_edges[i].second]++;
    }

    int maxDegree = *std::max_element(degree.begin(), degree.end());
    if (maxDegree == 0) {
        /* 没有边 */
        m_stats.totalColorings++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            (m_stats.totalColorings > 0) ? m_timeSum / m_stats.totalColorings : 0.0;
        return result;
    }

    /* 步骤2: 颜色上限 = Δ + 1 (Vizing定理) */
    int maxColors = maxDegree + 1;

    /* 步骤3: 维护每个顶点已使用的颜色标记 */
    /* vertexUsed[v][c] = true 表示顶点v的某条邻接边使用了颜色c */
    QVector<QVector<bool>> vertexUsed(m_n, QVector<bool>(maxColors, false));

    /* 步骤4: 逐边贪心着色 */
    for (int i = 0; i < edgeCount; ++i) {
        int u = m_edges[i].first;
        int v = m_edges[i].second;

        /* 寻找两端点都未使用的最小颜色 */
        int chosenColor = -1;
        for (int c = 0; c < maxColors; ++c) {
            if (!vertexUsed[u][c] && !vertexUsed[v][c]) {
                chosenColor = c;
                break;
            }
        }

        if (chosenColor == -1) {
            /* 无可用颜色 — 尝试Kempe链交换 */
            chosenColor = kempeChainSwap(i, vertexUsed, result, maxColors);
        }

        if (chosenColor >= 0) {
            result[i] = chosenColor;
            vertexUsed[u][chosenColor] = true;
            vertexUsed[v][chosenColor] = true;
        }
    }

    /* 更新统计信息 */
    int usedColors = 0;
    if (result.isEmpty() == false) {
        int maxC = *std::max_element(result.begin(), result.end());
        usedColors = maxC + 1;
    }
    if (m_stats.bestColors == 0 || usedColors < m_stats.bestColors) {
        m_stats.bestColors = usedColors;
    }

    m_stats.totalColorings++;
    m_stats.totalEdgesProcessed += edgeCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalColorings > 0) ? m_timeSum / m_stats.totalColorings : 0.0;

    emit coloringComplete(usedColors);
    return result;
}

/**
 * @brief 获取使用的颜色数
 * @return 着色使用的颜色总数
 */
int EdgeColoring2::numColors() const
{
    if (m_edges.isEmpty()) return 0;

    /* 需要先执行着色才有意义 — 基于当前图计算理论最小值 */
    QVector<int> degree(m_n, 0);
    for (const auto& edge : m_edges) {
        degree[edge.first]++;
        degree[edge.second]++;
    }
    int maxDegree = *std::max_element(degree.begin(), degree.end());
    return maxDegree + 1;  /* Vizing上界 */
}

/**
 * @brief 验证着色方案的有效性
 *
 * 检查条件:
 * 1. 所有边都已着色(颜色 >= 0)
 * 2. 共享同一顶点的两条边颜色不同
 *
 * @param colors 待验证的颜色方案
 * @return 是否为有效着色
 */
bool EdgeColoring2::isValid(const QVector<int>& colors) const
{
    if (colors.size() != m_edges.size()) return false;

    /* 检查每条边都有颜色 */
    for (int c : colors) {
        if (c < 0) return false;
    }

    /* 检查共享顶点的边颜色不冲突 */
    for (int i = 0; i < m_edges.size(); ++i) {
        for (int j = i + 1; j < m_edges.size(); ++j) {
            /* 如果两条边共享一个顶点 */
            if (m_edges[i].first == m_edges[j].first ||
                m_edges[i].first == m_edges[j].second ||
                m_edges[i].second == m_edges[j].first ||
                m_edges[i].second == m_edges[j].second) {
                /* 颜色必须不同 */
                if (colors[i] == colors[j]) {
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * @brief 重置统计信息
 */
void EdgeColoring2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ===== 私有方法实现 ===== */

/**
 * @brief Kempe链交换 — 消解着色冲突
 *
 * 当边(u,v)无法找到可用颜色时:
 * 1. 找到u未使用的颜色cu和v未使用的颜色cv
 * 2. 从v开始沿cu-cv交替颜色的链交换
 * 3. 交换后释放u或v的一个颜色槽
 *
 * @param edgeIdx 当前边的索引
 * @param vertexUsed 每个顶点已使用的颜色标记
 * @param colors 已着色的结果
 * @param maxColors 颜色总数
 * @return 分配的颜色，-1表示失败
 */
int EdgeColoring2::kempeChainSwap(int edgeIdx,
                                   QVector<QVector<bool>>& vertexUsed,
                                   QVector<int>& colors,
                                   int maxColors)
{
    int u = m_edges[edgeIdx].first;
    int v = m_edges[edgeIdx].second;

    /* 找u未使用的最小颜色 */
    int cu = -1;
    for (int c = 0; c < maxColors; ++c) {
        if (!vertexUsed[u][c]) {
            cu = c;
            break;
        }
    }

    /* 找v未使用的最小颜色 */
    int cv = -1;
    for (int c = 0; c < maxColors; ++c) {
        if (!vertexUsed[v][c]) {
            cv = c;
            break;
        }
    }

    if (cu < 0 || cv < 0) return -1;

    /* 构建Kempe链: 从v出发，沿cu-cv交替的边 */
    /* 简化实现: 枚举v的邻接边，找颜色为cu的边 */
    for (int i = 0; i < m_edges.size(); ++i) {
        if (colors[i] < 0) continue;

        bool adjToV = (m_edges[i].first == v || m_edges[i].second == v);
        if (!adjToV) continue;

        if (colors[i] == cu) {
            /* 将颜色cu的边改为cv */
            int other = (m_edges[i].first == v) ? m_edges[i].second : m_edges[i].first;
            vertexUsed[v][cu] = false;
            vertexUsed[other][cu] = false;

            colors[i] = cv;
            vertexUsed[v][cv] = true;
            vertexUsed[other][cv] = true;
            break;
        }
    }

    /* 交换后再次检查cu是否可用 */
    if (!vertexUsed[u][cu] && !vertexUsed[v][cu]) {
        return cu;
    }

    /* 再次尝试cv */
    if (!vertexUsed[u][cv] && !vertexUsed[v][cv]) {
        return cv;
    }

    /* 仍然冲突 — 在Δ+1范围内找任意可用颜色 */
    for (int c = 0; c < maxColors; ++c) {
        if (!vertexUsed[u][c] && !vertexUsed[v][c]) {
            return c;
        }
    }

    return -1;  /* 理论上不应到达 */
}
