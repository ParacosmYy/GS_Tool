/**
 * @file MaxClique.cpp
 * @brief 最大团搜索实现 — Bron-Kerbosch枢轴剪枝+k-团计数
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/graph38/MaxClique.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <functional>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
MaxClique::MaxClique(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图邻接表
 *
 * 构建内部邻接表和邻接集合(用于O(1)边查询)。
 *
 * @param adj 邻接表, adj[u]为u的邻居列表
 * @param n 节点数(编号0~n-1)
 */
void MaxClique::setGraph(const QVector<QVector<int>>& adj, int n)
{
    m_n = n;
    m_adj = adj;
    m_adjSet.resize(n);

    /* 构建邻接集合加速查询 */
    for (int u = 0; u < n; ++u) {
        m_adjSet[u].clear();
        if (u < adj.size()) {
            for (int v : adj[u]) {
                if (v >= 0 && v < n) {
                    m_adjSet[u].insert(v);
                }
            }
        }
    }

    m_bestClique.clear();
    m_maxSize = 0;
}

/**
 * @brief 找最大团
 *
 * 使用带枢轴选择的Bron-Kerbosch算法搜索最大团。
 * 枢轴策略: 选择P中邻居最多的节点作为枢轴以最大化剪枝。
 *
 * @return 最大团的节点列表(已排序)
 */
QVector<int> MaxClique::findMaximum()
{
    if (m_n == 0) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    m_bestClique.clear();
    m_maxSize = 0;
    int exploredNodes = 0;

    /* 初始化P为所有节点 */
    QVector<int> R, P, X;
    P.reserve(m_n);
    for (int i = 0; i < m_n; ++i) {
        P.append(i);
    }

    bronKerboschPivot(R, P, X);

    m_stats.totalSearches++;
    m_stats.totalCliquesFound++;
    m_stats.totalNodesExplored += exploredNodes;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    std::sort(m_bestClique.begin(), m_bestClique.end());
    emit searchCompleted(m_maxSize, exploredNodes);
    return m_bestClique;
}

/**
 * @brief 找所有极大团
 *
 * 枚举图中所有极大团(不可被更大团包含的团)。
 * 可限制返回数量防止指数爆炸。
 *
 * @param maxSize 最大返回团数(0表示全部返回)
 * @return 极大团列表
 */
QList<QVector<int>> MaxClique::findAllMaximal(int maxSize)
{
    if (m_n == 0) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    QList<QVector<int>> result;
    QVector<int> R, P, X;
    P.reserve(m_n);
    for (int i = 0; i < m_n; ++i) {
        P.append(i);
    }

    enumerateAll(R, P, X, result, maxSize);

    m_stats.totalSearches++;
    m_stats.totalCliquesFound += result.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    return result;
}

/**
 * @brief 判断给定节点集是否构成团
 *
 * 检查集合中每对节点是否都存在边相连。
 *
 * @param nodes 节点集合
 * @return true如果构成完全子图
 */
bool MaxClique::isClique(const QVector<int>& nodes) const
{
    int sz = nodes.size();
    for (int i = 0; i < sz; ++i) {
        for (int j = i + 1; j < sz; ++j) {
            int u = nodes[i];
            int v = nodes[j];
            if (u < 0 || u >= m_n || v < 0 || v >= m_n) {
                return false;
            }
            if (!m_adjSet[u].contains(v)) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief 统计k-团数量
 *
 * 使用递归回溯枚举所有大小为k的团并计数。
 * 节点按编号有序确保不重复计数。
 *
 * @param k 团大小
 * @return k-团数量
 */
long long MaxClique::countKCliques(int k) const
{
    if (k <= 0 || k > m_n) {
        return 0;
    }
    if (k == 1) {
        return m_n;
    }

    long long count = 0;
    QVector<int> current;
    current.reserve(k);

    /* 递归枚举有序k-团 */
    std::function<void(int, int)> enumerate = [&](int start, int depth) {
        if (depth == k) {
            ++count;
            return;
        }
        for (int v = start; v < m_n; ++v) {
            /* 检查v是否与current中所有节点相连 */
            bool connected = true;
            for (int u : current) {
                if (!m_adjSet[u].contains(v)) {
                    connected = false;
                    break;
                }
            }
            if (connected) {
                current.append(v);
                enumerate(v + 1, depth + 1);
                current.removeLast();
            }
        }
    };

    enumerate(0, 0);
    return count;
}

/**
 * @brief 计算图染色数下界(团数下界)
 *
 * 使用贪心着色给出染色数上界,这也是最大团大小的下界。
 *
 * @return 染色数上界(=最大团大小下界)
 */
int MaxClique::chromaticLowerBound() const
{
    if (m_n == 0) {
        return 0;
    }

    /* 贪心着色: 按度数降序排列 */
    QVector<int> order(m_n);
    for (int i = 0; i < m_n; ++i) {
        order[i] = i;
    }
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_adjSet[a].size() > m_adjSet[b].size();
    });

    QVector<int> color(m_n, -1);
    int maxColor = 0;

    for (int u : order) {
        /* 找出邻居已使用的颜色 */
        QSet<int> usedColors;
        for (int v : m_adjSet[u]) {
            if (color[v] >= 0) {
                usedColors.insert(color[v]);
            }
        }
        /* 分配最小可用颜色 */
        int c = 0;
        while (usedColors.contains(c)) {
            ++c;
        }
        color[u] = c;
        maxColor = qMax(maxColor, c + 1);
    }

    return maxColor;
}

/**
 * @brief Bron-Kerbosch递归(带枢轴剪枝)
 *
 * 核心递归过程:
 * 1. 选择枢轴u (P中邻居最多的节点)
 * 2. 只遍历P\N(u)中的候选节点(减少递归分支)
 * 3. 每次将候选加入R,更新P和X为交集
 *
 * @param R 当前团
 * @param P 候选集
 * @param X 已排除集
 */
void MaxClique::bronKerboschPivot(QVector<int>& R, QVector<int>& P, QVector<int>& X)
{
    if (P.isEmpty() && X.isEmpty()) {
        /* R是极大团 */
        if (R.size() > m_maxSize) {
            m_maxSize = R.size();
            m_bestClique = R;
        }
        m_stats.totalNodesExplored++;
        return;
    }

    /* 选择枢轴: P∪X中度数最大的节点 */
    int pivot = -1;
    int maxDeg = -1;
    for (int u : P) {
        if (u < m_adjSet.size() && m_adjSet[u].size() > maxDeg) {
            maxDeg = m_adjSet[u].size();
            pivot = u;
        }
    }
    for (int u : X) {
        if (u < m_adjSet.size() && m_adjSet[u].size() > maxDeg) {
            maxDeg = m_adjSet[u].size();
            pivot = u;
        }
    }

    /* P \ N(pivot): 不与枢轴相邻的候选 */
    QVector<int> candidates;
    for (int v : P) {
        if (pivot < 0 || !m_adjSet[pivot].contains(v)) {
            candidates.append(v);
        }
    }

    for (int v : candidates) {
        R.append(v);

        /* P∩N(v) */
        QVector<int> newP;
        for (int u : P) {
            if (m_adjSet[v].contains(u)) {
                newP.append(u);
            }
        }

        /* X∩N(v) */
        QVector<int> newX;
        for (int u : X) {
            if (m_adjSet[v].contains(u)) {
                newX.append(u);
            }
        }

        bronKerboschPivot(R, newP, newX);

        R.removeLast();
        P.removeOne(v);
        X.append(v);
    }
}

/**
 * @brief 枚举所有极大团的递归过程
 *
 * @param R 当前团
 * @param P 候选集
 * @param X 已排除集
 * @param result 结果列表
 * @param maxSize 最大返回数量
 */
void MaxClique::enumerateAll(QVector<int>& R, QVector<int>& P, QVector<int>& X,
                              QList<QVector<int>>& result, int maxSize)
{
    if (maxSize > 0 && result.size() >= maxSize) {
        return;
    }

    if (P.isEmpty() && X.isEmpty()) {
        QVector<int> clique = R;
        std::sort(clique.begin(), clique.end());
        result.append(clique);
        return;
    }

    if (P.isEmpty()) {
        return;
    }

    /* 无枢轴版本的Bron-Kerbosch用于完整枚举 */
    QVector<int> pCopy = P;
    for (int v : pCopy) {
        if (maxSize > 0 && result.size() >= maxSize) {
            break;
        }

        R.append(v);

        QVector<int> newP;
        for (int u : P) {
            if (m_adjSet[v].contains(u)) {
                newP.append(u);
            }
        }

        QVector<int> newX;
        for (int u : X) {
            if (m_adjSet[v].contains(u)) {
                newX.append(u);
            }
        }

        enumerateAll(R, newP, newX, result, maxSize);

        R.removeLast();
        P.removeOne(v);
        X.append(v);
    }
}

/**
 * @brief 重置统计信息
 */
void MaxClique::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
