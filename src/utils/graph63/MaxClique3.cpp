/**
 * @file MaxClique3.cpp
 * @brief 最大团搜索算法实现
 *
 * 实现基于回溯+剪枝的最大团搜索算法，使用贪心着色作为上界
 * 进行分支限界剪枝。支持有向/无向图的最大团查找和团验证。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/graph63/MaxClique3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @class MaxClique3
 * @brief 最大团(MWC)搜索器，带回溯+着色剪枝
 *
 * 算法采用Bron-Kerbosch风格的回溯搜索，在每个分支节点
 * 使用贪心着色计算团大小的上界。若上界不超过当前最优解
 * 则剪枝，大幅减少搜索空间。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject指针
 */
MaxClique3::MaxClique3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的邻接关系
 * @param n 顶点数量
 * @param edges 边列表，每条边为一对顶点索引
 */
void MaxClique3::setGraph(int n, const QVector<QPair<int,int>>& edges)
{
    m_n = n;
    m_adj.assign(n, QVector<int>());
    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < n &&
            edge.second >= 0 && edge.second < n) {
            m_adj[edge.first].append(edge.second);
            m_adj[edge.second].append(edge.first);
        }
    }
    m_clique.clear();
}

/**
 * @brief 搜索最大团
 *
 * 从空团开始，逐步扩展候选顶点集。
 * 每个分支使用colorBound()计算着色上界进行剪枝。
 * 搜索完成后m_clique保存最大团的顶点集合。
 *
 * @return 最大团的顶点索引列表
 */
QVector<int> MaxClique3::solve()
{
    QElapsedTimer timer;
    timer.start();

    m_clique.clear();

    if (m_n <= 0) {
        m_stats.totalSolves++;
        m_stats.totalVertices += m_n;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
            ? m_timeSum / m_stats.totalSolves : 0.0;
        return m_clique;
    }

    /* 初始化：所有顶点都是候选 */
    QVector<int> current;
    QVector<int> candidates;
    candidates.reserve(m_n);
    for (int i = 0; i < m_n; ++i) {
        candidates.append(i);
    }

    /* 启动回溯搜索 */
    expand(current, candidates);

    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solved(m_clique.size());
    return m_clique;
}

/**
 * @brief 验证给定顶点集合是否构成团
 *
 * 检查集合中任意两个顶点之间是否都存在边。
 *
 * @param vertices 待验证的顶点列表
 * @return 若构成完全子图返回true
 */
bool MaxClique3::isClique(const QVector<int>& vertices) const
{
    for (int i = 0; i < vertices.size(); ++i) {
        for (int j = i + 1; j < vertices.size(); ++j) {
            int u = vertices[i];
            int v = vertices[j];
            bool found = false;
            for (int neighbor : m_adj[u]) {
                if (neighbor == v) { found = true; break; }
            }
            if (!found) return false;
        }
    }
    return true;
}

/**
 * @brief 重置统计数据
 */
void MaxClique3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 回溯扩展搜索最大团
 *
 * 对于候选集中的每个顶点v，尝试将其加入当前团，并
 * 将候选集更新为v的邻居中仍在候选集里的顶点。
 * 利用着色上界剪枝：如果当前团大小+着色上界<=已找到的最大团，
 * 则无需继续搜索此分支。
 *
 * @param current 当前团
 * @param candidates 候选顶点集
 */
void MaxClique3::expand(QVector<int>& current, QVector<int>& candidates)
{
    if (candidates.isEmpty()) {
        /* 无候选顶点，更新最优解 */
        if (current.size() > m_clique.size()) {
            m_clique = current;
        }
        return;
    }

    /* 着色上界剪枝 */
    int bound = colorBound(candidates);
    if (static_cast<int>(current.size()) + bound <= static_cast<int>(m_clique.size())) {
        return;
    }

    /* 按度数降序排列候选顶点（启发式排序） */
    QVector<int> sorted = candidates;
    std::sort(sorted.begin(), sorted.end(), [this](int a, int b) {
        return m_adj[a].size() > m_adj[b].size();
    });

    for (int i = 0; i < sorted.size(); ++i) {
        int v = sorted[i];

        /* 剪枝：即使把剩余所有候选都加入也无法超过当前最优 */
        if (static_cast<int>(current.size()) + static_cast<int>(sorted.size()) - i
            <= static_cast<int>(m_clique.size())) {
            break;
        }

        /* 将v加入当前团 */
        current.append(v);

        /* 计算新的候选集：v的邻居且在原候选集中且编号大于v */
        QVector<int> newCandidates;
        for (int c : candidates) {
            if (c == v) continue;
            bool isNeighbor = false;
            for (int neighbor : m_adj[v]) {
                if (neighbor == c) { isNeighbor = true; break; }
            }
            if (isNeighbor) newCandidates.append(c);
        }

        /* 递归搜索 */
        expand(current, newCandidates);

        /* 回溯：移除v */
        current.removeLast();
    }
}

/**
 * @brief 贪心着色计算团大小上界
 *
 * 对候选顶点集进行贪心着色，着色数即为团大小的上界。
 * 算法按顺序给每个顶点分配最小可用颜色号。
 *
 * @param candidates 候选顶点集
 * @return 着色数（团大小上界）
 */
int MaxClique3::colorBound(const QVector<int>& candidates) const
{
    if (candidates.isEmpty()) return 0;

    QVector<int> color(candidates.size(), 0);
    int maxColor = 0;

    for (int i = 0; i < candidates.size(); ++i) {
        QSet<int> usedColors;
        for (int j = 0; j < i; ++j) {
            /* 检查candidates[i]和candidates[j]是否相邻 */
            bool adjacent = false;
            for (int neighbor : m_adj[candidates[i]]) {
                if (neighbor == candidates[j]) { adjacent = true; break; }
            }
            if (adjacent) usedColors.insert(color[j]);
        }

        /* 分配最小可用颜色 */
        int c = 1;
        while (usedColors.contains(c)) c++;
        color[i] = c;
        maxColor = qMax(maxColor, c);
    }

    return maxColor;
}
