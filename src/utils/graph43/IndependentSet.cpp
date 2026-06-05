/**
 * @file IndependentSet.cpp
 * @brief 独立集求解实现 — 最大独立集/贪心近似/分支定界/补图转换
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph43/IndependentSet.h"

#include <QElapsedTimer>

#include <algorithm>
#include <numeric>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
IndependentSet::IndependentSet(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置图的邻接关系
 *  @param n 顶点数
 *  @param edges 边列表(顶点对，0-indexed)
 */
void IndependentSet::setGraph(int n, const QVector<QPair<int,int>>& edges)
{
    m_n = n;
    m_adj.assign(n, QVector<int>());

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n &&
            e.second >= 0 && e.second < n &&
            e.first != e.second)
        {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
        }
    }

    /* 去重邻接表 */
    for (int i = 0; i < n; ++i) {
        std::sort(m_adj[i].begin(), m_adj[i].end());
        m_adj[i].erase(std::unique(m_adj[i].begin(), m_adj[i].end()),
                       m_adj[i].end());
    }
}

/** @brief 贪心最大独立集(按度数升序选取)
 *  每次选择度数最小且未被邻居排除的顶点加入独立集
 *  @return 独立集顶点列表
 */
QVector<int> IndependentSet::greedyMaximum()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (m_n == 0) return result;

    /* 按度数排序顶点索引 */
    std::vector<int> order(m_n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_adj[a].size() < m_adj[b].size();
    });

    QVector<bool> taken(m_n, false);
    for (int v : order) {
        if (taken[v]) continue;
        /* 检查是否与已选顶点相邻 */
        bool adjacent = false;
        for (int u : result) {
            for (int nb : m_adj[u]) {
                if (nb == v) { adjacent = true; break; }
            }
            if (adjacent) break;
        }
        if (!adjacent) {
            result.append(v);
            taken[v] = true;
            /* 标记所有邻居为不可选 */
            for (int nb : m_adj[v]) {
                taken[nb] = true;
            }
        }
    }

    m_stats.totalSearches++;
    m_stats.totalVerticesProcessed += m_n;
    m_stats.bestSize = qMax(m_stats.bestSize, result.size());
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalSearches > 0) ?
        m_timeSum / m_stats.totalSearches : 0.0;
    emit searchComplete(result.size());
    return result;
}

/** @brief 分支定界的上界估计
 *  使用当前可用顶点数的上界来剪枝
 *  @param available 可用顶点标记
 *  @return 独立集大小上界
 */
int IndependentSet::bound(QVector<bool>& available) const
{
    int count = 0;
    for (int i = 0; i < m_n; ++i) {
        if (available[i]) ++count;
    }
    return count;
}

/** @brief 分支定界递归搜索
 *  @param available 当前可用顶点集合
 *  @param current 当前独立集
 *  @param best 迄今最佳独立集
 *  @param depth 当前递归深度
 */
void IndependentSet::bbSearch(QVector<bool>& available,
                               QVector<int>& current,
                               QVector<int>& best, int depth)
{
    /* 剪枝: 剩余可用顶点加上当前大小无法超过已知最优 */
    int upper = current.size() + bound(available);
    if (upper <= static_cast<int>(best.size())) {
        return;
    }

    /* 找到第一个可用顶点 */
    int v = -1;
    int minDeg = m_n + 1;
    for (int i = 0; i < m_n; ++i) {
        if (available[i]) {
            /* 度数启发式: 选择度数最小的可用顶点 */
            int deg = 0;
            for (int nb : m_adj[i]) {
                if (available[nb]) ++deg;
            }
            if (deg < minDeg) {
                minDeg = deg;
                v = i;
            }
        }
    }

    if (v < 0) {
        /* 所有顶点已处理 */
        if (static_cast<int>(current.size()) > static_cast<int>(best.size())) {
            best = current;
        }
        return;
    }

    /* 分支1: 选取顶点v */
    QVector<bool> avail2 = available;
    avail2[v] = false;
    for (int nb : m_adj[v]) {
        avail2[nb] = false;
    }
    current.append(v);
    bbSearch(avail2, current, best, depth + 1);
    current.removeLast();

    /* 分支2: 不选取顶点v */
    available[v] = false;
    bbSearch(available, current, best, depth + 1);
    available[v] = true;
}

/** @brief 分支定界精确求解最大独立集 @return 最大独立集顶点列表 */
QVector<int> IndependentSet::branchAndBound()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (m_n == 0) {
        emit searchComplete(0);
        return result;
    }

    QVector<bool> available(m_n, true);
    QVector<int> current;
    bbSearch(available, current, result, 0);

    std::sort(result.begin(), result.end());
    m_stats.totalSearches++;
    m_stats.totalVerticesProcessed += m_n;
    m_stats.bestSize = qMax(m_stats.bestSize, static_cast<int>(result.size()));
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalSearches > 0) ?
        m_timeSum / m_stats.totalSearches : 0.0;
    emit searchComplete(result.size());
    return result;
}

/** @brief 通过补图最大团转换求解最大独立集
 *  在补图中寻找最大团等价于原图中的最大独立集
 *  @return 最大独立集顶点列表
 */
QVector<int> IndependentSet::complementClique()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (m_n == 0) {
        emit searchComplete(0);
        return result;
    }

    /* 构建补图邻接矩阵 */
    QVector<QVector<bool>> compAdj(m_n, QVector<bool>(m_n, false));
    for (int i = 0; i < m_n; ++i) {
        for (int j = i + 1; j < m_n; ++j) {
            compAdj[i][j] = true;
            compAdj[j][i] = true;
        }
    }
    /* 移除原图中已有的边 */
    for (int i = 0; i < m_n; ++i) {
        for (int nb : m_adj[i]) {
            if (nb > i) {
                compAdj[i][nb] = false;
                compAdj[nb][i] = false;
            }
        }
    }

    /* Bron-Kerbosch启发式求最大团 */
    QVector<int> R;     /* 当前团 */
    QVector<int> P;     /* 候选顶点 */
    QVector<int> X;     /* 已排除顶点 */
    for (int i = 0; i < m_n; ++i) P.append(i);

    /* 迭代版Bron-Kerbosch(带pivot) */
    QVector<QVector<int>> stack_R, stack_P, stack_X;
    stack_R.push_back(R);
    stack_P.push_back(P);
    stack_X.push_back(X);

    while (!stack_R.empty()) {
        R = stack_R.back(); stack_R.pop_back();
        P = stack_P.back(); stack_P.pop_back();
        X = stack_X.back(); stack_X.pop_back();

        if (P.isEmpty() && X.isEmpty()) {
            if (static_cast<int>(R.size()) > static_cast<int>(result.size())) {
                result = R;
            }
            continue;
        }

        /* 选择pivot(从P∪X中度数最大的) */
        QVector<int> PX = P;
        for (int x : X) PX.append(x);
        int pivot = -1;
        int maxDeg = -1;
        for (int u : PX) {
            int deg = 0;
            for (int j = 0; j < m_n; ++j) {
                if (compAdj[u][j]) ++deg;
            }
            if (deg > maxDeg) { maxDeg = deg; pivot = u; }
        }

        /* P \ N(pivot) */
        QVector<int> candidates;
        for (int v : P) {
            if (pivot < 0 || !(pivot >= 0 && pivot < m_n && v >= 0 && v < m_n && compAdj[pivot][v])) {
                candidates.append(v);
            }
        }

        for (int v : candidates) {
            QVector<int> newR = R;
            newR.append(v);
            QVector<int> newP, newX;
            for (int u : P) {
                if (compAdj[v][u]) newP.append(u);
            }
            for (int u : X) {
                if (compAdj[v][u]) newX.append(u);
            }
            stack_R.push_back(newR);
            stack_P.push_back(newP);
            stack_X.push_back(newX);

            /* 从P中移除v */
            P.removeOne(v);
            X.append(v);
        }
    }

    std::sort(result.begin(), result.end());
    m_stats.totalSearches++;
    m_stats.totalVerticesProcessed += m_n;
    m_stats.bestSize = qMax(m_stats.bestSize, static_cast<int>(result.size()));
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalSearches > 0) ?
        m_timeSum / m_stats.totalSearches : 0.0;
    emit searchComplete(result.size());
    return result;
}

/** @brief 检查给定顶点子集是否构成独立集
 *  @param vertices 待检查的顶点列表
 *  @return true如果任意两顶点之间没有边
 */
bool IndependentSet::isIndependent(const QVector<int>& vertices) const
{
    for (int i = 0; i < vertices.size(); ++i) {
        for (int j = i + 1; j < vertices.size(); ++j) {
            int u = vertices[i];
            int v = vertices[j];
            if (u >= 0 && u < m_n && v >= 0 && v < m_n) {
                for (int nb : m_adj[u]) {
                    if (nb == v) return false;
                }
            }
        }
    }
    return true;
}

/** @brief 重置所有统计计数器 */
void IndependentSet::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
