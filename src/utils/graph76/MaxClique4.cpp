/**
 * @file MaxClique4.cpp
 * @brief 最大团搜索算法实现 — 基于颜色排序的分支限界法
 *
 * 使用贪心着色排序剪枝的回溯算法，高效搜索无向图中的最大团。
 * 支持逐步构建图结构，并提供团验证功能。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/graph76/MaxClique4.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
MaxClique4::MaxClique4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数，必须 >= 1
 */
void MaxClique4::setVertexCount(int n)
{
    m_n = qMax(1, n);
    m_adj.assign(m_n, QVector<int>());
}

/**
 * @brief 添加无向边
 * @param u 第一个顶点编号
 * @param v 第二个顶点编号
 */
void MaxClique4::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return;

    /* 检查是否已存在 */
    for (int x : m_adj[u]) {
        if (x == v) return;
    }
    m_adj[u].append(v);
    m_adj[v].append(u);
}

/**
 * @brief 求解最大团
 *
 * 使用回溯法 + 颜色排序剪枝策略:
 * 1. 对候选顶点贪心着色
 * 2. 利用颜色上界剪枝不可能产生更大团的分支
 * 3. 按颜色递减顺序扩展，优先搜索有希望的顶点
 *
 * @return 最大团的顶点集合
 */
QVector<int> MaxClique4::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) {
        m_cliqueSize = 0;
        return {};
    }

    QVector<int> best;
    QVector<int> current;
    QVector<int> candidates;
    candidates.reserve(m_n);
    for (int i = 0; i < m_n; ++i) {
        candidates.append(i);
    }

    m_cliqueSize = 0;
    expand(current, candidates, best);
    m_cliqueSize = best.size();

    /* 更新统计信息 */
    m_stats.totalSearches++;
    m_stats.totalVertices += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit solveCompleted(m_cliqueSize);
    return best;
}

/**
 * @brief 检查给定顶点集合是否构成团
 *
 * 验证集合中每对顶点之间是否都存在边。
 *
 * @param set 待验证的顶点集合
 * @return true如果构成团，false否则
 */
bool MaxClique4::isClique(const QVector<int>& set) const
{
    const int sz = set.size();
    for (int i = 0; i < sz; ++i) {
        for (int j = i + 1; j < sz; ++j) {
            int u = set[i];
            int v = set[j];
            bool found = false;
            for (int x : m_adj[u]) {
                if (x == v) { found = true; break; }
            }
            if (!found) return false;
        }
    }
    return true;
}

/**
 * @brief 重置所有统计数据
 */
void MaxClique4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 回溯搜索最大团
 *
 * 递归扩展当前团，使用颜色上界剪枝。
 *
 * @param current 当前已选顶点集合
 * @param candidates 候选顶点集合
 * @param best 当前最优解(输出参数)
 */
void MaxClique4::expand(QVector<int>& current, QVector<int>& candidates, QVector<int>& best)
{
    if (candidates.isEmpty()) {
        if (current.size() > best.size()) {
            best = current;
        }
        return;
    }

    /* 贪心着色获取上界 */
    QVector<int> colors = colorSort(candidates);

    /* 按颜色递减顺序扩展 */
    for (int i = candidates.size() - 1; i >= 0; --i) {
        /* 剪枝: 当前大小 + 颜色上界 <= 最优解 */
        if (static_cast<int>(current.size()) + colors[i] <= static_cast<int>(best.size())) {
            return;
        }

        int v = candidates[i];
        current.append(v);

        /* 构造新的候选集: 与v相邻的候选顶点 */
        QVector<int> newCands;
        for (int j = 0; j < i; ++j) {
            int u = candidates[j];
            /* 检查u是否与v相邻 */
            for (int neighbor : m_adj[v]) {
                if (neighbor == u) {
                    newCands.append(u);
                    break;
                }
            }
        }

        expand(current, newCands, best);
        current.removeLast();
    }
}

/**
 * @brief 贪心着色排序
 *
 * 对候选顶点按度数递减顺序贪心着色，
 * 返回每个顶点的颜色编号(用作团大小上界)。
 *
 * @param candidates 候选顶点集合
 * @return 对应每个顶点的颜色编号向量
 */
QVector<int> MaxClique4::colorSort(const QVector<int>& candidates)
{
    const int n = candidates.size();
    QVector<int> order = candidates;

    /* 按度数(在候选集中的邻居数)降序排序 */
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_adj[a].size() > m_adj[b].size();
    });

    QVector<int> colors(n, 0);
    QVector<bool> usedColor(n + 1, false);

    for (int i = 0; i < n; ++i) {
        /* 标记已使用颜色 */
        for (int& uc : (QVector<bool>&)usedColor) { uc = false; } /* 重置 */
        usedColor.assign(n + 1, false);

        for (int j = 0; j < i; ++j) {
            /* 检查order[i]和order[j]是否相邻 */
            bool adjacent = false;
            for (int neighbor : m_adj[order[i]]) {
                if (neighbor == order[j]) {
                    adjacent = true;
                    break;
                }
            }
            if (adjacent) {
                usedColor[colors[j]] = true;
            }
        }

        /* 分配最小可用颜色 */
        for (int c = 1; c <= n; ++c) {
            if (!usedColor[c]) {
                colors[i] = c;
                break;
            }
        }
    }

    /* 将排序后的颜色映射回原始候选顺序 */
    QVector<int> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = colors[i];
    }
    return result;
}
