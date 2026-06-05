/**
 * @file IndependentSet2.cpp
 * @brief 独立集2 — 最大独立集近似+图着色方法 实现
 *
 * 提供三种独立集求解方法：贪心近似、图着色近似和综合求解。
 * 贪心法按度数排序选取；着色法利用颜色类的天然独立集性质。
 * 所有方法都带 QElapsedTimer 计时和统计追踪。
 */

#include "utils/graph62/IndependentSet2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/**
 * @brief 构造函数，初始化空图
 * @param parent 父QObject
 */
IndependentSet2::IndependentSet2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的邻接关系
 *
 * 根据边列表构建邻接表，自环和重边会被自动忽略。
 *
 * @param n 顶点数
 * @param edges 边列表，每条边为 (u, v) 对
 */
void IndependentSet2::setGraph(int n, const QVector<QPair<int, int>>& edges)
{
    m_n = qMax(0, n);
    m_adj.assign(m_n, QVector<int>());
    m_solution.clear();

    for (const auto& edge : edges) {
        int u = edge.first;
        int v = edge.second;
        if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) continue;
        if (!m_adj[u].contains(v)) m_adj[u].append(v);
        if (!m_adj[v].contains(u)) m_adj[v].append(u);
    }
}

/**
 * @brief 综合求解最大独立集
 *
 * 分别运行贪心法和着色法，取较大者作为最终结果。
 * 使用 QElapsedTimer 计时并更新统计。
 *
 * @return 独立集中的顶点索引列表
 */
QVector<int> IndependentSet2::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) {
        m_solution.clear();
        return {};
    }

    /* 运行两种近似方法 */
    QVector<int> greedyResult = solveGreedy();
    QVector<int> coloringResult = solveColoring();

    /* 选择更大的结果 */
    if (greedyResult.size() >= coloringResult.size()) {
        m_solution = greedyResult;
    } else {
        m_solution = coloringResult;
    }

    /* 统计更新 */
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_stats.independentSetSize = m_solution.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(m_solution.size(), m_n);
    return m_solution;
}

/**
 * @brief 贪心法求解独立集
 *
 * 按顶点度数从小到大排序，依次将不与当前独立集冲突的顶点加入。
 * 度数小的顶点优先，因为它们与更少的顶点相邻，
 * 留下更多空间给后续选择。
 *
 * @return 贪心独立集
 */
QVector<int> IndependentSet2::solveGreedy()
{
    if (m_n == 0) return {};

    /* 按度数排序（低度数优先） */
    QVector<QPair<int, int>> degreeVertex;
    degreeVertex.reserve(m_n);
    for (int i = 0; i < m_n; ++i) {
        degreeVertex.append(qMakePair(m_adj[i].size(), i));
    }
    std::sort(degreeVertex.begin(), degreeVertex.end());

    QSet<int> selected;
    QSet<int> excluded;

    for (const auto& dv : degreeVertex) {
        int v = dv.second;
        if (excluded.contains(v)) continue;

        selected.insert(v);
        for (int u : m_adj[v]) {
            excluded.insert(u);
        }
    }

    return QVector<int>(selected.begin(), selected.end());
}

/**
 * @brief 图着色法求解独立集
 *
 * 使用贪心着色（按度数降序的 Welsh-Powell 策略），
 * 然后取最大颜色类作为独立集（同一颜色类的顶点不互相连接）。
 *
 * @return 着色法得到的独立集
 */
QVector<int> IndependentSet2::solveColoring()
{
    if (m_n == 0) return {};

    /* 按度数降序排列 */
    QVector<QPair<int, int>> degreeVertex;
    degreeVertex.reserve(m_n);
    for (int i = 0; i < m_n; ++i) {
        degreeVertex.append(qMakePair(m_adj[i].size(), i));
    }
    std::sort(degreeVertex.begin(), degreeVertex.end(),
              [](const QPair<int, int>& a, const QPair<int, int>& b) {
                  return a.first > b.first;
              });

    /* 贪心着色 */
    QVector<int> color(m_n, -1);
    int maxColor = -1;

    for (const auto& dv : degreeVertex) {
        int v = dv.second;
        QSet<int> usedColors;
        for (int u : m_adj[v]) {
            if (color[u] >= 0) {
                usedColors.insert(color[u]);
            }
        }
        /* 找最小可用颜色 */
        int c = 0;
        while (usedColors.contains(c)) c++;
        color[v] = c;
        maxColor = qMax(maxColor, c);
    }

    /* 统计每个颜色类的大小 */
    QVector<int> colorCount(maxColor + 1, 0);
    for (int i = 0; i < m_n; ++i) {
        if (color[i] >= 0) {
            colorCount[color[i]]++;
        }
    }

    /* 找最大颜色类 */
    int bestColor = 0;
    for (int c = 1; c <= maxColor; ++c) {
        if (colorCount[c] > colorCount[bestColor]) {
            bestColor = c;
        }
    }

    QVector<int> result;
    for (int i = 0; i < m_n; ++i) {
        if (color[i] == bestColor) {
            result.append(i);
        }
    }
    return result;
}

/**
 * @brief 验证给定集合是否为独立集
 *
 * 检查集合中是否存在任意两点相邻。
 *
 * @param set 待验证的顶点集合
 * @return true 表示是独立集，false 表示存在相邻顶点
 */
bool IndependentSet2::isIndependent(const QVector<int>& set) const
{
    QSet<int> setLookup(set.begin(), set.end());
    for (int v : set) {
        if (v < 0 || v >= m_n) return false;
        for (int u : m_adj[v]) {
            if (setLookup.contains(u) && u != v) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief 构造一个极大独立集（非最大）
 *
 * 使用简单的顺序扫描：遍历所有顶点，如果与已选集合
 * 不冲突则加入。结果保证是极大独立集。
 *
 * @return 极大独立集
 */
QVector<int> IndependentSet2::maximalIndependentSet() const
{
    if (m_n == 0) return {};

    QSet<int> selected;
    QSet<int> excluded;

    for (int v = 0; v < m_n; ++v) {
        if (excluded.contains(v)) continue;
        selected.insert(v);
        for (int u : m_adj[v]) {
            excluded.insert(u);
        }
    }

    return QVector<int>(selected.begin(), selected.end());
}

/**
 * @brief 重置所有统计数据
 */
void IndependentSet2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
