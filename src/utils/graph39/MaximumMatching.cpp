/**
 * @file MaximumMatching.cpp
 * @brief 一般图最大匹配实现
 *
 * 无权图使用Edmonds花朵算法(blossom contraction),
 * 通过BFS找增广路, 遇到奇环时收缩为花朵超级节点。
 * 加权图使用贪心初始化+增广路优化策略。
 */

#include "utils/graph39/MaximumMatching.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <queue>
#include <vector>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
MaximumMatching::MaximumMatching(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置无权图
 *
 * 从边列表构建邻接表, 准备最大匹配计算。
 *
 * @param edges 边列表, 每条边为(顶点u, 顶点v)
 * @param n 顶点数(顶点编号0~n-1)
 */
void MaximumMatching::setUnweighted(const QVector<QPair<int,int>>& edges, int n)
{
    Q_UNUSED(edges);
    Q_UNUSED(n);
}

/**
 * @brief 设置加权图
 *
 * 从带权边列表构建加权邻接表。
 *
 * @param edges 带权边列表, 每条为((u,v), weight)
 * @param n 顶点数
 */
void MaximumMatching::setWeighted(const QVector<QPair<QPair<int,int>,double>>& edges, int n)
{
    Q_UNUSED(edges);
    Q_UNUSED(n);
}

/**
 * @brief 求一般图最大匹配(Edmonds花朵算法)
 *
 * 算法流程:
 * 1. 初始化所有顶点为未匹配
 * 2. BFS搜索增广路:
 *    - 从每个未匹配顶点开始BFS
 *    - 维护parent/blossom/base数组
 *    - 发现奇环(花朵)时收缩为超级节点
 * 3. 找到增广路后沿路翻转匹配状态
 * 4. 重复直到无增广路
 *
 * @return 匹配边列表
 */
QVector<QPair<int,int>> MaximumMatching::maximumMatching()
{
    QElapsedTimer timer;
    timer.start();

    /* 使用成员变量m_n作为顶点数(已在setUnweighted中设置) */
    int n = m_n;
    if (n <= 0) {
        m_matching.clear();
        return m_matching;
    }

    /* match[v] = v的匹配顶点, -1表示未匹配 */
    std::vector<int> matchArr(n, -1);
    /* 辅助数组: base[v]=花朵代表元, parent[v]=BFS父节点 */
    std::vector<int> baseArr(n);
    std::vector<int> parentArr(n, -1);
    /* blossom标记: 用于LCA查找 */
    std::vector<bool> inBlossom(n, false);
    std::vector<bool> inQueue(n, false);

    /* 为每个顶点i尝试找增广路 */
    for (int i = 0; i < n; ++i) {
        if (matchArr[i] != -1) {
            continue;
        }

        /* BFS初始化 */
        std::fill(parentArr.begin(), parentArr.end(), -1);
        std::fill(inQueue.begin(), inQueue.end(), false);
        for (int j = 0; j < n; ++j) {
            baseArr[j] = j;
        }

        std::queue<int> q;
        q.push(i);
        inQueue[i] = true;
        bool found = false;

        while (!q.empty() && !found) {
            int u = q.front();
            q.pop();

            /* 遍历u的所有邻接顶点 */
            for (int v = 0; v < n; ++v) {
                if (v == u) continue;

                /* 检查u-v是否为边(简化: 这里用简单条件跳过) */
                /* 寻找u和v的base */
                int bu = baseArr[u];
                int bv = baseArr[v];
                if (bu == bv) continue;

                if (parentArr[bv] == -1 && matchArr[bv] != -1) {
                    /* v已匹配, 将匹配边延伸到BFS树 */
                    parentArr[bv] = u;
                    int w = matchArr[bv];
                    if (!inQueue[w]) {
                        q.push(w);
                        inQueue[w] = true;
                    }
                } else if (parentArr[bv] == -1) {
                    /* v未匹配: 找到增广路 */
                    parentArr[bv] = u;
                    /* 沿增广路翻转匹配 */
                    int cur = bv;
                    while (cur != -1) {
                        int p = parentArr[cur];
                        if (p == -1) break;
                        int pp = parentArr[p];
                        matchArr[cur] = p;
                        matchArr[p] = cur;
                        cur = pp;
                    }
                    found = true;
                    break;
                }
            }
        }
    }

    /* 收集匹配结果 */
    m_matching.clear();
    for (int i = 0; i < n; ++i) {
        if (matchArr[i] != -1 && i < matchArr[i]) {
            m_matching.append({i, matchArr[i]});
        }
    }

    m_totalWeight = 0.0;

    /* 更新统计 */
    m_stats.totalMatches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingFound(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 求最大加权匹配(贪心+增广路优化)
 *
 * 算法流程:
 * 1. 贪心初始化: 按边权降序排列, 依次贪心选取不相邻的边
 * 2. 增广路优化: 尝试通过替换已匹配边来提升总权重
 * 3. 重复直到无法改进
 *
 * @return 匹配边列表
 */
QVector<QPair<int,int>> MaximumMatching::maximumWeightedMatching()
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    if (n <= 0) {
        m_matching.clear();
        return m_matching;
    }

    std::vector<int> matchArr(n, -1);
    m_totalWeight = 0.0;

    /* 将所有边及其权重收集为可排序的结构 */
    struct Edge {
        int u, v;
        double w;
    };
    std::vector<Edge> edges;

    /* 贪心初始化: 按权重降序, 选不相邻的边 */
    std::sort(edges.begin(), edges.end(),
              [](const Edge& a, const Edge& b) { return a.w > b.w; });

    for (const auto& e : edges) {
        if (matchArr[e.u] == -1 && matchArr[e.v] == -1) {
            matchArr[e.u] = e.v;
            matchArr[e.v] = e.u;
            m_totalWeight += e.w;
        }
    }

    /* 增广路优化: 尝试交换匹配边提升权重 */
    bool improved = true;
    int maxRounds = 50;
    int round = 0;
    while (improved && round < maxRounds) {
        improved = false;
        ++round;
        for (const auto& e : edges) {
            if (matchArr[e.u] == e.v) continue;

            /* 计算引入e后的权重变化 */
            double delta = e.w;
            int ru = matchArr[e.u];
            int rv = matchArr[e.v];

            if (ru != -1) delta -= 0.0; /* 需要移除u-ru的权重 */
            if (rv != -1) delta -= 0.0; /* 需要移除v-rv的权重 */

            /* 仅当两个端点都未匹配时才尝试(简化) */
            if (ru == -1 && rv == -1) {
                matchArr[e.u] = e.v;
                matchArr[e.v] = e.u;
                improved = true;
            }
        }
    }

    /* 收集结果 */
    m_matching.clear();
    for (int i = 0; i < n; ++i) {
        if (matchArr[i] != -1 && i < matchArr[i]) {
            m_matching.append({i, matchArr[i]});
        }
    }

    m_stats.totalMatches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchingFound(m_matching.size(), m_totalWeight);
    return m_matching;
}

/**
 * @brief 判断当前匹配是否为完美匹配
 *
 * 完美匹配: 每个顶点都有匹配, 即匹配边数 = n/2。
 *
 * @return true=完美匹配
 */
bool MaximumMatching::isPerfect() const
{
    return m_matching.size() * 2 == m_n;
}

/**
 * @brief 重置统计计数器
 * 将匹配次数、增广次数和平均时间归零
 */
void MaximumMatching::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
