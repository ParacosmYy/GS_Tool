/**
 * @file IndependentSet3.cpp
 * @brief 最大独立集求解器实现 (分支限界法)
 *
 * 使用分支限界(Branch and Bound)算法求解图的最大独立集问题:
 * - 通过上界估计剪枝，减少搜索空间
 * - 按顶点度数排序优化搜索顺序
 * - 维护当前最优解并持续更新
 * 最大独立集是一个NP难问题，本实现适用于中小规模图。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/graph74/IndependentSet3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化最大独立集求解器
 * @param parent 父QObject指针
 */
IndependentSet3::IndependentSet3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量
 *
 * 调用此方法会重置所有边
 */
void IndependentSet3::setVertexCount(int n)
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
 */
void IndependentSet3::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) {
        return;
    }
    if (!m_adj[u].contains(v)) {
        m_adj[u].append(v);
        m_adj[v].append(u);
    }
}

/**
 * @brief 求解最大独立集
 *
 * 使用分支限界法:
 * 1. 按顶点顺序逐步决定是否加入独立集
 * 2. 如果当前顶点与已选顶点不相邻，则尝试加入
 * 3. 计算上界，如果当前解 + 上界 <= 最优解，则剪枝
 * 4. 最终返回找到的最大独立集
 *
 * @return 最大独立集中的顶点索引列表
 */
QVector<int> IndependentSet3::solve()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> bestSet;

    if (m_n == 0) {
        m_setSize = 0;
        emit solveCompleted(0);
        return bestSet;
    }

    /* 检查是否有边 */
    bool hasEdges = false;
    for (int i = 0; i < m_n && !hasEdges; ++i) {
        if (!m_adj[i].isEmpty()) hasEdges = true;
    }

    if (!hasEdges) {
        /* 无边图：所有顶点构成独立集 */
        for (int i = 0; i < m_n; ++i) {
            bestSet.append(i);
        }
        m_setSize = m_n;

        m_stats.totalSolves++;
        m_stats.totalVertices += m_n;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveCompleted(m_setSize);
        return bestSet;
    }

    /* 分支限界搜索 */
    QVector<int> current;
    QVector<bool> used(m_n, false);
    branchAndBound(current, bestSet, used, 0);

    m_setSize = bestSet.size();

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_setSize);
    return bestSet;
}

/**
 * @brief 验证给定集合是否为独立集
 *
 * 检查集合中任意两个顶点之间是否存在边
 *
 * @param set 待验证的顶点集合
 * @return 如果是独立集返回true，否则false
 */
bool IndependentSet3::isIndependent(const QVector<int>& set) const
{
    for (int i = 0; i < set.size(); ++i) {
        for (int j = i + 1; j < set.size(); ++j) {
            int u = set[i];
            int v = set[j];
            if (u >= 0 && u < m_n && v >= 0 && v < m_n) {
                if (m_adj[u].contains(v)) {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * @brief 重置所有统计数据
 */
void IndependentSet3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 分支限界递归搜索
 *
 * 对每个顶点，尝试两种选择:
 * 1. 将顶点加入当前独立集（如果不与已选顶点冲突）
 * 2. 跳过该顶点
 * 通过上界估计进行剪枝
 *
 * @param current 当前正在构建的独立集
 * @param best 迄今找到的最优独立集
 * @param used 标记与已选顶点相邻的顶点
 * @param idx 当前考虑的顶点索引
 */
void IndependentSet3::branchAndBound(QVector<int>& current, QVector<int>& best, QVector<bool>& used, int idx)
{
    /* 剪枝: 如果当前解 + 上界 <= 最优解，则不再搜索 */
    int ub = upperBound(current, used);
    if (static_cast<int>(current.size()) + ub <= static_cast<int>(best.size())) {
        return;
    }

    /* 已处理完所有顶点 */
    if (idx >= m_n) {
        if (static_cast<int>(current.size()) > static_cast<int>(best.size())) {
            best = current;
        }
        return;
    }

    /* 选择1: 跳过当前顶点 */
    branchAndBound(current, best, used, idx + 1);

    /* 选择2: 将当前顶点加入独立集（如果可行） */
    if (!used[idx]) {
        /* 标记当前顶点及其邻居为已使用 */
        current.append(idx);
        QVector<int> newlyUsed;
        used[idx] = true;
        for (int neighbor : m_adj[idx]) {
            if (!used[neighbor]) {
                used[neighbor] = true;
                newlyUsed.append(neighbor);
            }
        }

        /* 递归搜索 */
        branchAndBound(current, best, used, idx + 1);

        /* 回溯: 恢复状态 */
        current.removeLast();
        used[idx] = false;
        for (int v : newlyUsed) {
            used[v] = false;
        }
    }
}

/**
 * @brief 计算当前状态下的独立集大小上界
 *
 * 上界 = 当前可用（未使用且未被排除）的顶点数
 * 这是松弛的上界，用于加速剪枝
 *
 * @param current 当前独立集
 * @param used 已使用/已排除的顶点标记
 * @return 独立集大小的上界
 */
int IndependentSet3::upperBound(const QVector<int>& current, const QVector<bool>& used) const
{
    int remaining = 0;
    for (int i = 0; i < m_n; ++i) {
        if (!used[i]) {
            remaining++;
        }
    }
    return remaining;
}
