/**
 * @file MaxClique2.cpp
 * @brief 最大团搜索实现 - 基于着色上界与退化排序的分支限界
 *
 * 使用贪心图着色提供团大小的上界剪枝，
 * 配合退化排序(degeneracy ordering)优化搜索顺序，
 * 采用Bron-Kerbosch变体配合剪枝策略搜索最大团。
 */

#include "utils/graph51/MaxClique2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
MaxClique2::MaxClique2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置无向图
 * @param n 节点数量
 * @param edges 边列表(每条边为两个端点的索引对)
 */
void MaxClique2::setGraph(int n, const QVector<QPair<int, int>>& edges)
{
    m_n = n;
    m_adj.resize(n);
    for (int i = 0; i < n; ++i)
        m_adj[i].clear();

    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n) {
            m_adj[e.first].append(e.second);
            m_adj[e.second].append(e.first);
        }
    }

    /* 去重并排序邻接表 */
    for (int i = 0; i < n; ++i) {
        std::sort(m_adj[i].begin(), m_adj[i].end());
        m_adj[i].erase(std::unique(m_adj[i].begin(), m_adj[i].end()), m_adj[i].end());
    }
}

/**
 * @brief 贪心图着色(用于计算团上界)
 *
 * 按给定的顶点顺序进行贪心着色:
 * 对每个顶点，使用可用的最小颜色编号。
 * 所用颜色数即为色数上界，也是团大小的上界。
 *
 * @param order 顶点处理顺序
 * @return 每个顶点的颜色编号(从1开始)
 */
QVector<int> MaxClique2::greedyColoring(const QVector<int>& order) const
{
    QVector<int> color(m_n, 0);
    QVector<bool> used(m_n + 1, false);

    for (int v : order) {
        /* 标记邻居已使用的颜色 */
        for (int u : m_adj[v]) {
            if (color[u] > 0)
                used[color[u]] = true;
        }

        /* 找到最小可用颜色 */
        int c = 1;
        while (c <= m_n && used[c]) c++;
        color[v] = c;

        /* 清除标记 */
        for (int u : m_adj[v]) {
            if (color[u] > 0)
                used[color[u]] = false;
        }
    }

    return color;
}

/**
 * @brief 计算退化排序(degeneracy ordering)
 *
 * 不断移除当前度数最小的顶点，记录移除顺序。
 * 退化度(core number)等于此过程中最大度数。
 *
 * @return 退化排序后的顶点列表(从小度到大度)
 */
QVector<int> MaxClique2::degeneracyOrdering() const
{
    QVector<int> degree(m_n, 0);
    for (int i = 0; i < m_n; ++i)
        degree[i] = m_adj[i].size();

    QVector<bool> removed(m_n, false);
    QVector<int> order;
    order.reserve(m_n);

    for (int step = 0; step < m_n; ++step) {
        /* 找最小度数 */
        int minDeg = m_n + 1;
        int minV = -1;
        for (int i = 0; i < m_n; ++i) {
            if (!removed[i] && degree[i] < minDeg) {
                minDeg = degree[i];
                minV = i;
            }
        }
        if (minV < 0) break;

        order.append(minV);
        removed[minV] = true;

        /* 更新邻居度数 */
        for (int u : m_adj[minV]) {
            if (!removed[u]) degree[u]--;
        }
    }

    return order;
}

/**
 * @brief 查找最大团
 *
 * 使用分支限界法:
 * 1. 计算退化排序确定搜索顺序
 * 2. 贪心着色提供上界剪枝
 * 3. 从每个候选顶点出发扩展团
 *
 * @return 最大团的顶点列表
 */
QVector<int> MaxClique2::findMaximum()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> bestClique;

    if (m_n <= 0) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        emit searchComplete(0);
        return bestClique;
    }

    /* 计算退化排序(反转: 大度先处理) */
    QVector<int> degOrder = degeneracyOrdering();
    std::reverse(degOrder.begin(), degOrder.end());

    /* 贪心着色获取上界 */
    QVector<int> color = greedyColoring(degOrder);

    /* 构建逆映射: 顶点在排序中的位置 */
    QVector<int> position(m_n, -1);
    for (int i = 0; i < degOrder.size(); ++i)
        position[degOrder[i]] = i;

    /* 预计算邻居集合(使用位向量加速) */
    auto isNeighbor = [&](int u, int v) -> bool {
        return std::binary_search(m_adj[u].begin(), m_adj[u].end(), v);
    };

    /* 分支限界搜索 */
    /* current: 当前团, candidates: 候选顶点, colors: 候选颜色上界 */
    int bestSize = 0;

    /* 使用栈模拟递归 */
    struct State {
        QVector<int> current;      /**< 当前团 */
        QVector<int> candidates;   /**< 候选集 */
        QVector<int> candColors;   /**< 候选着色上界 */
    };

    /* 初始候选集: 按退化排序 */
    QVector<int> initCands = degOrder;
    QVector<int> initColors(initCands.size());
    for (int i = 0; i < initCands.size(); ++i)
        initColors[i] = color[initCands[i]];

    /* 逆序处理使栈弹出时按正序 */
    QVector<State> stack;
    stack.push_back({{}, initCands, initColors});

    while (!stack.isEmpty()) {
        State state = stack.takeLast();

        /* 剪枝: 当前团大小 + 候选集最大颜色上界 <= 已知最优 */
        int maxColorBound = 0;
        for (int c : state.candColors)
            maxColorBound = qMax(maxColorBound, c);

        if (state.current.size() + maxColorBound <= bestSize)
            continue;

        /* 从候选集中选择顶点扩展 */
        for (int i = state.candidates.size() - 1; i >= 0; --i) {
            int v = state.candidates[i];
            int vColor = state.candColors[i];

            /* 剪枝 */
            if (state.current.size() + vColor <= bestSize)
                continue;

            /* 构建新候选集: 与v相邻且在剩余候选中 */
            QVector<int> newCands;
            QVector<int> newColors;

            for (int j = 0; j < i; ++j) {
                int u = state.candidates[j];
                if (isNeighbor(v, u)) {
                    newCands.append(u);
                    newColors.append(state.candColors[j]);
                }
            }

            /* 扩展团 */
            state.current.append(v);

            if (state.current.size() > bestSize) {
                bestSize = state.current.size();
                bestClique = state.current;
            }

            /* 如果还有候选则继续搜索 */
            if (!newCands.isEmpty()) {
                /* 重新着色新候选集 */
                QVector<int> order(newCands.size());
                for (int j = 0; j < newCands.size(); ++j)
                    order[j] = newCands[j];

                QVector<int> recolor = greedyColoring(order);
                for (int j = 0; j < newCands.size(); ++j)
                    newColors[j] = recolor[newCands[j]];

                /* 按颜色排序(升序)以实现有效剪枝 */
                QVector<QPair<int, int>> paired;
                for (int j = 0; j < newCands.size(); ++j)
                    paired.append({newColors[j], newCands[j]});
                std::sort(paired.begin(), paired.end());

                QVector<int> sortedCands, sortedColors;
                for (auto& p : paired) {
                    sortedColors.append(p.first);
                    sortedCands.append(p.second);
                }

                stack.push_back({state.current, sortedCands, sortedColors});
            }

            state.current.removeLast();
        }
    }

    m_stats.totalSearches++;
    m_stats.totalVerticesProcessed += m_n;
    m_stats.bestSize = bestSize;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchComplete(bestSize);
    return bestClique;
}

/**
 * @brief 枚举所有极大团(Bron-Kerbosch算法)
 * @return 所有无冗余的极大团列表
 */
QList<QVector<int>> MaxClique2::enumerateMaximal()
{
    QElapsedTimer timer;
    timer.start();

    QList<QVector<int>> result;

    if (m_n <= 0) {
        m_timeSum += timer.elapsed();
        return result;
    }

    auto isNeighbor = [&](int u, int v) -> bool {
        return std::binary_search(m_adj[u].begin(), m_adj[u].end(), v);
    };

    /* Bron-Kerbosch with pivot */
    struct State { QVector<int> R, P, X; };
    QVector<State> stack;
    QVector<int> allVertices;
    for (int i = 0; i < m_n; ++i) allVertices.append(i);
    stack.push_back({{}, allVertices, {}});

    while (!stack.isEmpty()) {
        State st = stack.takeLast();

        if (st.P.isEmpty() && st.X.isEmpty()) {
            result.append(st.R);
            continue;
        }

        /* 选择枢轴: P中度数最大的顶点 */
        int pivot = -1;
        int maxDeg = -1;
        for (int v : st.P) {
            int deg = 0;
            for (int u : st.P)
                if (isNeighbor(v, u)) deg++;
            if (deg > maxDeg) { maxDeg = deg; pivot = v; }
        }
        for (int v : st.X) {
            int deg = 0;
            for (int u : st.P)
                if (isNeighbor(v, u)) deg++;
            if (deg > maxDeg) { maxDeg = deg; pivot = v; }
        }

        /* 候选 = P \ N(pivot) */
        QVector<int> candidates;
        for (int v : st.P) {
            if (pivot < 0 || !isNeighbor(pivot, v))
                candidates.append(v);
        }

        for (int v : candidates) {
            QVector<int> newR = st.R;
            newR.append(v);

            QVector<int> newP, newX;
            for (int u : st.P)
                if (isNeighbor(v, u)) newP.append(u);
            for (int u : st.X)
                if (isNeighbor(v, u)) newX.append(u);

            stack.push_back({newR, newP, newX});

            /* 将v从P移到X */
            st.P.removeOne(v);
            st.X.append(v);
        }
    }

    m_timeSum += timer.elapsed();
    return result;
}

/**
 * @brief 计算色数下界(基于贪心着色)
 * @return 团大小的下界估计
 */
int MaxClique2::chromaticLowerBound() const
{
    if (m_n <= 0) return 0;

    /* 找最大度数 */
    int maxDeg = 0;
    for (int i = 0; i < m_n; ++i)
        maxDeg = qMax(maxDeg, m_adj[i].size());

    return qMax(1, maxDeg + 1);
}

/**
 * @brief 重置所有统计数据
 */
void MaxClique2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
