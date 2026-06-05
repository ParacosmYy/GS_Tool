/**
 * @file BipartiteMatch3.cpp
 * @brief 二分图匹配3实现 — 加权KM+DFS/BFS混合
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph55/BipartiteMatch3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>
#include <queue>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
BipartiteMatch3::BipartiteMatch3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BipartiteMatch3"));
}

/**
 * @brief 设置无权二分图边集合
 *
 * 构建邻接关系用于Hopcroft-Karp最大匹配算法。
 *
 * @param edges 边集合（左顶点, 右顶点）
 * @param leftN 左侧顶点数
 * @param rightN 右侧顶点数
 */
void BipartiteMatch3::setUnweighted(const QVector<QPair<int,int>>& edges,
                                     int leftN, int rightN)
{
    m_leftN = leftN;
    m_rightN = rightN;
    m_weightMatrix.assign(leftN, QVector<double>(rightN, 0.0));

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < leftN && e.second >= 0 && e.second < rightN) {
            m_weightMatrix[e.first][e.second] = 1.0;
        }
    }
}

/**
 * @brief 设置加权二分图边集合
 *
 * 构建权值矩阵用于Kuhn-Munkres最优匹配算法。
 *
 * @param edges 加权边集合 ((左顶点, 右顶点), 权值)
 * @param leftN 左侧顶点数
 * @param rightN 右侧顶点数
 */
void BipartiteMatch3::setWeighted(
    const QVector<QPair<QPair<int,int>,double>>& edges,
    int leftN, int rightN)
{
    m_leftN = leftN;
    m_rightN = rightN;
    m_weightMatrix.assign(leftN, QVector<double>(rightN, 0.0));

    for (const auto& e : edges) {
        int u = e.first.first;
        int v = e.first.second;
        if (u >= 0 && u < leftN && v >= 0 && v < rightN) {
            m_weightMatrix[u][v] = e.second;
        }
    }
}

/**
 * @brief 求最大匹配（无权）
 *
 * 使用Hopcroft-Karp算法，时间复杂度O(E*sqrt(V))。
 * DFS与BFS混合搜索增广路径。
 *
 * @return 匹配边集合
 */
QVector<QPair<int,int>> BipartiteMatch3::maximumMatching()
{
    QElapsedTimer timer;
    timer.start();

    m_matching = hopcroftKarp();
    m_totalWeight = 0.0;
    for (const auto& p : m_matching) {
        m_totalWeight += m_weightMatrix[p.first][p.second];
    }

    m_stats.totalMatches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingFound(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 求最大权匹配
 *
 * 使用Kuhn-Munkres算法求权值和最大的完美匹配。
 *
 * @return 最大权匹配边集合
 */
QVector<QPair<int,int>> BipartiteMatch3::maximumWeightedMatching()
{
    QElapsedTimer timer;
    timer.start();

    m_matching = kuhnMunkres();
    m_totalWeight = 0.0;
    for (const auto& p : m_matching) {
        m_totalWeight += m_weightMatrix[p.first][p.second];
    }

    m_stats.totalMatches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingFound(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 求最小权匹配
 *
 * 将权值取反后求最大权匹配，再恢复原权值。
 *
 * @return 最小权匹配边集合
 */
QVector<QPair<int,int>> BipartiteMatch3::minimumWeightedMatching()
{
    QElapsedTimer timer;
    timer.start();

    /* 取反权值 */
    double maxW = 0.0;
    for (int i = 0; i < m_leftN; ++i)
        for (int j = 0; j < m_rightN; ++j)
            maxW = qMax(maxW, m_weightMatrix[i][j]);

    auto saved = m_weightMatrix;
    for (int i = 0; i < m_leftN; ++i)
        for (int j = 0; j < m_rightN; ++j)
            m_weightMatrix[i][j] = maxW - m_weightMatrix[i][j];

    m_matching = kuhnMunkres();

    /* 恢复原始权值 */
    m_weightMatrix = saved;
    m_totalWeight = 0.0;
    for (const auto& p : m_matching) {
        m_totalWeight += m_weightMatrix[p.first][p.second];
    }

    m_stats.totalMatches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingFound(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 判断当前匹配是否为完美匹配
 * @return 完美匹配返回true
 */
bool BipartiteMatch3::isPerfect() const
{
    return m_matching.size() == qMin(m_leftN, m_rightN);
}

/**
 * @brief Hopcroft-Karp最大匹配算法
 *
 * BFS构建分层图，DFS搜索增广路径。
 * 每轮至少使匹配数增加1。
 *
 * @return 最大匹配边集合
 */
QVector<QPair<int,int>> BipartiteMatch3::hopcroftKarp()
{
    int n = m_leftN, m = m_rightN;
    QVector<int> pairU(n, -1), pairV(m, -1);
    QVector<int> dist(n, 0);
    int augmentations = 0;

    /* 构建邻接表 */
    QVector<QVector<int>> adj(n);
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < m; ++v) {
            if (m_weightMatrix[u][v] > 0.0) {
                adj[u].append(v);
            }
        }
    }

    /* BFS分层 */
    auto bfs = [&]() -> bool {
        std::queue<int> q;
        for (int u = 0; u < n; ++u) {
            if (pairU[u] == -1) { dist[u] = 0; q.push(u); }
            else { dist[u] = -1; }
        }
        bool found = false;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                int pu = pairV[v];
                if (pu != -1 && dist[pu] == -1) {
                    dist[pu] = dist[u] + 1;
                    q.push(pu);
                } else if (pu == -1) {
                    found = true;
                }
            }
        }
        return found;
    };

    /* DFS搜索增广路径 */
    std::function<bool(int)> dfs = [&](int u) -> bool {
        for (int v : adj[u]) {
            int pu = pairV[v];
            if (pu == -1 || (dist[pu] == dist[u] + 1 && dfs(pu))) {
                pairU[u] = v;
                pairV[v] = u;
                return true;
            }
        }
        dist[u] = -1;
        return false;
    };

    while (bfs()) {
        for (int u = 0; u < n; ++u) {
            if (pairU[u] == -1 && dfs(u)) {
                augmentations++;
            }
        }
    }

    m_stats.totalAugmentations += augmentations;
    m_stats.leftSize = n;

    QVector<QPair<int,int>> result;
    for (int u = 0; u < n; ++u) {
        if (pairU[u] != -1) {
            result.append({u, pairU[u]});
        }
    }
    return result;
}

/**
 * @brief Kuhn-Munkres最优匹配算法
 *
 * 通过顶标和相等子图求解最优分配。
 * 使用DFS搜索增广路径，O(n^3)复杂度。
 *
 * @return 最优匹配边集合
 */
QVector<QPair<int,int>> BipartiteMatch3::kuhnMunkres()
{
    int n = qMax(m_leftN, m_rightN);

    /* 扩展为方阵 */
    QVector<QVector<double>> w(n, QVector<double>(n, 0.0));
    for (int i = 0; i < m_leftN; ++i)
        for (int j = 0; j < m_rightN; ++j)
            w[i][j] = m_weightMatrix[i][j];

    QVector<double> lx(n, 0.0), ly(n, 0.0);
    QVector<int> matchX(n, -1), matchY(n, -1);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            lx[i] = qMax(lx[i], w[i][j]);
        }
    }

    for (int u = 0; u < n; ++u) {
        QVector<double> slack(n, 1e18);
        QVector<bool> visX(n, false), visY(n, false);

        bool found = false;
        int matchU = -1, matchV = -1;

        while (!found) {
            double delta = 1e18;
            visX.assign(n, false);
            visY.assign(n, false);

            std::function<bool(int)> dfs = [&](int x) -> bool {
                visX[x] = true;
                for (int y = 0; y < n; ++y) {
                    if (visY[y]) continue;
                    double t = lx[x] + ly[y] - w[x][y];
                    if (qFabs(t) < 1e-9) {
                        visY[y] = true;
                        if (matchY[y] == -1 || dfs(matchY[y])) {
                            matchX[x] = y;
                            matchY[y] = x;
                            return true;
                        }
                    } else {
                        slack[y] = qMin(slack[y], t);
                    }
                }
                return false;
            };

            if (dfs(u)) { found = true; break; }

            for (int y = 0; y < n; ++y) {
                if (!visY[y]) delta = qMin(delta, slack[y]);
            }

            for (int i = 0; i < n; ++i) {
                if (visX[i]) lx[i] -= delta;
                if (visY[i]) ly[i] += delta;
            }
        }
    }

    QVector<QPair<int,int>> result;
    for (int i = 0; i < m_leftN; ++i) {
        if (matchX[i] >= 0 && matchX[i] < m_rightN) {
            result.append({i, matchX[i]});
        }
    }
    return result;
}

/**
 * @brief 重置所有统计数据
 */
void BipartiteMatch3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
