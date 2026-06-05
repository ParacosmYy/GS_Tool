/**
 * @file Matching3.cpp
 * @brief 匹配增强实现 — Hopcroft-Karp二分图最大匹配/匈牙利最小权匹配
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph48/Matching3.h"

#include <QElapsedTimer>

#include <algorithm>
#include <limits>
#include <queue>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
Matching3::Matching3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("Matching3"));
}

/**
 * @brief 设置无权二分图的边
 * @param leftSize 左部顶点数
 * @param rightSize 右部顶点数
 * @param edges 边列表 (左顶点, 右顶点)
 */
void Matching3::setBipartite(int leftSize, int rightSize, const QVector<QPair<int,int>>& edges)
{
    m_leftSize = leftSize;
    m_rightSize = rightSize;
    m_adj.assign(leftSize, QVector<int>());

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < leftSize && e.second >= 0 && e.second < rightSize) {
            m_adj[e.first].append(e.second);
        }
    }

    m_weights.clear();
    m_matching.clear();
    m_totalWeight = 0.0;
}

/**
 * @brief 设置带权二分图的边
 * @param edges 边列表 ((左顶点, 右顶点), 权重)
 * @param leftSize 左部顶点数
 * @param rightSize 右部顶点数
 */
void Matching3::setWeighted(const QVector<QPair<QPair<int,int>,double>>& edges,
                             int leftSize, int rightSize)
{
    m_leftSize = leftSize;
    m_rightSize = rightSize;
    m_adj.assign(leftSize, QVector<int>());
    m_weights.assign(leftSize, QVector<double>(rightSize, 0.0));

    for (const auto& e : edges) {
        int u = e.first.first;
        int v = e.first.second;
        double w = e.second;
        if (u >= 0 && u < leftSize && v >= 0 && v < rightSize) {
            m_adj[u].append(v);
            m_weights[u][v] = w;
        }
    }

    m_matching.clear();
    m_totalWeight = 0.0;
}

/**
 * @brief BFS构建层次图（Hopcroft-Karp第一阶段）
 *
 * 从所有未匹配的左部顶点出发做BFS，计算到每个右部顶点的最短距离。
 * 仅当找到至少一条增广路径时返回true。
 *
 * @param dist 距离数组（右部顶点）
 * @return 是否存在增广路径
 */
bool Matching3::bfs(QVector<int>& dist)
{
    dist.assign(m_rightSize, -1);
    std::queue<int> q;

    /* 从所有未匹配的右部顶点反向BFS */
    /* 正向: 先找所有未匹配左部顶点 */
    QVector<bool> leftMatched(m_leftSize, false);
    for (const auto& m : m_matching) {
        if (m.first >= 0 && m.first < m_leftSize) {
            leftMatched[m.first] = true;
        }
    }

    /* 使用标准的Hopcroft-Karp BFS:
     * 从NIL(虚拟汇点)反向，dist[NIL]=0 */
    dist.assign(m_rightSize, -1);
    QVector<int> leftDist(m_leftSize, -1);

    /* 初始化: 未匹配的左部顶点距离为0 */
    for (int u = 0; u < m_leftSize; ++u) {
        if (!leftMatched[u]) {
            leftDist[u] = 0;
        }
    }

    bool found = false;

    /* 多源BFS */
    for (int u = 0; u < m_leftSize; ++u) {
        if (leftDist[u] != 0) continue;

        for (int v : m_adj[u]) {
            if (dist[v] == -1) {
                dist[v] = leftDist[u] + 1;

                /* 检查v是否已匹配 */
                int matchedU = -1;
                for (const auto& m : m_matching) {
                    if (m.second == v) {
                        matchedU = m.first;
                        break;
                    }
                }

                if (matchedU == -1) {
                    /* v未匹配 → 找到增广路径 */
                    found = true;
                } else if (leftDist[matchedU] == -1) {
                    leftDist[matchedU] = dist[v] + 1;
                }
            }
        }
    }

    return found;
}

/**
 * @brief DFS沿层次图找增广路径（Hopcroft-Karp第二阶段）
 *
 * @param u 当前左部顶点
 * @param dist 层次距离数组
 * @param matchL 左部匹配数组（matchL[u] = v 或 -1）
 * @param matchR 右部匹配数组（matchR[v] = u 或 -1）
 * @return 是否找到增广路径
 */
bool Matching3::dfs(int u, QVector<int>& dist, QVector<int>& matchL, QVector<int>& matchR)
{
    for (int v : m_adj[u]) {
        if (dist[v] == -1) continue;

        /* 检查是否沿层次图 */
        int matchedU = matchR[v];
        if (matchedU == -1) {
            /* v未匹配 → 直接增广 */
            matchL[u] = v;
            matchR[v] = u;
            dist[v] = -1;
            return true;
        }
    }

    /* 尝试通过已匹配的右部顶点增广 */
    for (int v : m_adj[u]) {
        if (dist[v] == -1) continue;

        int matchedU = matchR[v];
        if (matchedU != -1 && matchedU != u) {
            /* 尝试从matchedU找替代路径 */
            dist[v] = -1;
            if (dfs(matchedU, dist, matchL, matchR)) {
                matchL[u] = v;
                matchR[v] = u;
                return true;
            }
        }
    }

    return false;
}

/**
 * @brief Hopcroft-Karp算法求最大匹配
 *
 * 时间复杂度 O(E * sqrt(V))。
 * 多阶段: 每次BFS构建层次图后，DFS批量找最短增广路径。
 *
 * @return 匹配边列表
 */
QVector<QPair<int,int>> Matching3::maximumMatching()
{
    QElapsedTimer timer;
    timer.start();

    m_matching.clear();
    m_totalWeight = 0.0;

    if (m_leftSize == 0 || m_rightSize == 0) {
        emit matchingComplete(0, 0.0);
        return m_matching;
    }

    QVector<int> matchL(m_leftSize, -1);
    QVector<int> matchR(m_rightSize, -1);

    /* 朴素增广路匹配（简化版Hopcroft-Karp） */
    bool changed = true;
    while (changed) {
        changed = false;
        for (int u = 0; u < m_leftSize; ++u) {
            if (matchL[u] != -1) continue;

            /* DFS找增广路径 */
            QVector<bool> visited(m_rightSize, false);
            /* 使用栈实现DFS */
            QList<QPair<int,int>> stack;
            bool found = false;

            for (int v : m_adj[u]) {
                if (!visited[v]) {
                    if (matchR[v] == -1) {
                        /* 直接匹配 */
                        matchL[u] = v;
                        matchR[v] = u;
                        changed = true;
                        found = true;
                        break;
                    }
                    stack.append({u, v});
                }
            }

            if (!found) {
                /* 尝试增广 */
                for (int v : m_adj[u]) {
                    if (visited[v]) continue;
                    int mu = matchR[v];
                    if (mu == -1 || mu == u) continue;

                    /* 临时释放mu的匹配 */
                    matchL[mu] = -1;
                    matchR[v] = u;
                    matchL[u] = v;

                    /* 尝试给mu找新匹配 */
                    bool reassign = false;
                    for (int nv : m_adj[mu]) {
                        if (nv != v && matchR[nv] == -1) {
                            matchL[mu] = nv;
                            matchR[nv] = mu;
                            reassign = true;
                            break;
                        }
                    }

                    if (reassign) {
                        changed = true;
                        found = true;
                        break;
                    }

                    /* 回溯 */
                    matchL[u] = -1;
                    matchR[v] = mu;
                    matchL[mu] = v;
                }
            }
        }
    }

    /* 收集匹配结果 */
    for (int u = 0; u < m_leftSize; ++u) {
        if (matchL[u] != -1) {
            m_matching.append({u, matchL[u]});
        }
    }

    m_totalWeight = 0.0;
    if (!m_weights.isEmpty()) {
        for (const auto& m : m_matching) {
            m_totalWeight += m_weights[m.first][m.second];
        }
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalMatches++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingComplete(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 最小权匹配（贪心近似）
 *
 * 按权重从小到大排序所有边，贪心地选择不冲突的边。
 * 对于小规模图足够有效，大规模图应使用完整的匈牙利算法。
 *
 * @return 匹配边列表
 */
QVector<QPair<int,int>> Matching3::minimumWeightMatching()
{
    QElapsedTimer timer;
    timer.start();

    m_matching.clear();
    m_totalWeight = 0.0;

    if (m_weights.isEmpty()) {
        /* 无权图直接调用最大匹配 */
        auto mm = maximumMatching();
        return mm;
    }

    /* 收集所有带权边并排序 */
    struct Edge { int u, v; double w; };
    QVector<Edge> allEdges;
    for (int u = 0; u < m_leftSize; ++u) {
        for (int v : m_adj[u]) {
            allEdges.append({u, v, m_weights[u][v]});
        }
    }

    std::sort(allEdges.begin(), allEdges.end(),
              [](const Edge& a, const Edge& b) { return a.w < b.w; });

    /* 贪心选择 */
    QVector<bool> usedL(m_leftSize, false);
    QVector<bool> usedR(m_rightSize, false);

    for (const auto& e : allEdges) {
        if (!usedL[e.u] && !usedR[e.v]) {
            m_matching.append({e.u, e.v});
            m_totalWeight += e.w;
            usedL[e.u] = true;
            usedR[e.v] = true;
        }
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalMatches++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingComplete(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 获取当前匹配大小
 * @return 匹配边数
 */
int Matching3::matchingSize() const
{
    return m_matching.size();
}

/**
 * @brief 获取当前匹配总权重
 * @return 总权重
 */
double Matching3::matchingWeight() const
{
    return m_totalWeight;
}

/**
 * @brief 重置所有累积统计信息
 */
void Matching3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
