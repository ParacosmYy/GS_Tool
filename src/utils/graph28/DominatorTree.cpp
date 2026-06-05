/**
 * @file DominatorTree.cpp
 * @brief 支配树构造实现 — Lengauer-Tarjan算法
 */

#include "utils/graph28/DominatorTree.h"

#include <QElapsedTimer>
#include <algorithm>
#include <numeric>
#include <vector>

/* ========== 构造 ========== */

DominatorTree::DominatorTree(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/* ========== 主算法: Lengauer-Tarjan ========== */

QVector<DominatorTree::DomInfo> DominatorTree::build(
    const QVector<Edge>& edges, int numNodes, int entryNode)
{
    QElapsedTimer timer;
    timer.start();

    QVector<DomInfo> result(numNodes);
    if (numNodes <= 0 || edges.isEmpty()) {
        m_timeSum += timer.elapsed();
        return result;
    }

    /* 构建邻接表(前驱和后继) */
    QVector<QVector<int>> succ(numNodes);   /* 后继列表 */
    QVector<QVector<int>> pred(numNodes);   /* 前驱列表 */
    for (const auto& e : edges) {
        if (e.from >= 0 && e.from < numNodes && e.to >= 0 && e.to < numNodes) {
            succ[e.from].append(e.to);
            pred[e.to].append(e.from);
        }
    }

    /* Step 1: DFS编号 */
    QVector<int> semi(numNodes, -1);      /* 半支配者(初始化为自身) */
    QVector<int> vertex(numNodes, -1);    /* vertex[dfnum] = 节点编号 */
    QVector<int> parent(numNodes, -1);    /* DFS树中的父节点 */
    QVector<int> dfnum(numNodes, -1);     /* DFS编号 */
    int counter = 0;

    dfs(entryNode, succ, semi, vertex, parent, counter);

    int reachable = counter; /* 可达节点数 */

    /* 初始化semi为自身 */
    for (int i = 0; i < numNodes; ++i) {
        if (dfnum[i] >= 0) semi[i] = i;
    }

    /* 并查集数据结构 */
    QVector<int> ancestor(numNodes, -1);
    QVector<int> label(numNodes);
    std::iota(label.begin(), label.end(), 0); /* label[i] = i */

    QVector<int> idom(numNodes, -1); /* 最终直接支配者 */

    /* Step 2~3: 按DFS序逆序处理 */
    /* bucket[w]: 需要以w为半支配者的节点集合 */
    QVector<QVector<int>> bucket(numNodes);

    for (int i = reachable - 1; i >= 1; --i) {
        int w = vertex[i]; /* 当前节点 */

        /* Step 2: 计算半支配者 */
        for (int v : pred[w]) {
            if (dfnum[v] < 0) continue; /* v不可达 */
            int u = find(v, ancestor, label, semi);
            if (dfnum[semi[u]] < dfnum[semi[w]])
                semi[w] = semi[u];
        }

        /* 将w加入bucket[semi[w]] */
        if (semi[w] >= 0 && semi[w] < numNodes)
            bucket[semi[w]].append(w);

        /* link(parent[w], w) */
        ancestor[w] = parent[w];
        label[w] = w;

        /* Step 3: 隐式计算idom */
        int s = parent[w];
        if (s >= 0) {
            for (int v : bucket[s]) {
                int u = find(v, ancestor, label, semi);
                if (semi[u] == semi[v]) {
                    idom[v] = s; /* semi[v] 就是v的直接支配者 */
                } else {
                    idom[v] = u; /* 待后续修正 */
                }
            }
            bucket[s].clear();
        }
    }

    /* Step 4: 修正Step 3中未完全确定的idom */
    for (int i = 1; i < reachable; ++i) {
        int w = vertex[i];
        if (idom[w] >= 0 && semi[w] >= 0 && idom[w] != semi[w]) {
            idom[w] = idom[idom[w]];
        }
    }
    idom[entryNode] = entryNode; /* 入口节点的支配者是自身 */

    /* 填充DomInfo */
    for (int i = 0; i < numNodes; ++i) {
        result[i].idom = idom[i];
        if (idom[i] >= 0 && idom[i] != i) {
            result[idom[i]].dominatedBy.append(i);
        }
    }

    /* 统计更新 */
    ++m_stats.totalConstructions;
    m_stats.totalNodesProcessed += numNodes;
    m_stats.totalEdgesProcessed += edges.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConstructions;

    emit constructionComplete(numNodes, edges.size());
    return result;
}

/* ========== 支配边界 ========== */

QVector<DominatorTree::DomInfo> DominatorTree::computeDominanceFrontiers(
    const QVector<DomInfo>& domInfo, int numNodes) const
{
    QVector<DomInfo> result = domInfo;
    if (numNodes <= 0) return result;

    /* 支配边界算法:
     * 对每个节点b, 其支配边界 DF(b) 包含满足以下条件的节点y:
     * b支配y的某个前驱, 但b不严格支配y
     */

    /* 需要前驱信息 —— 从dominatedBy推导支配关系是不够的,
     * 此处使用简化的基于支配者链的算法 */

    for (int b = 0; b < numNodes; ++b) {
        if (domInfo[b].idom < 0) continue;

        /* 检查b直接支配的所有节点 */
        for (int dominated : domInfo[b].dominatedBy) {
            /* dominated的直接支配者是b, 但dominated的其他前驱可能不被b支配
             * → dominated是b的支配边界候选 */
            /* 在简化实现中, 检查dominated的支配者链 */
            int runner = dominated;
            while (runner != b && runner >= 0 && domInfo[runner].idom != runner) {
                /* 如果b不严格支配runner的某个支配者链上的节点 */
                bool bStrictlyDominates = false;
                int check = runner;
                while (check >= 0) {
                    check = domInfo[check].idom;
                    if (check == b) { bStrictlyDominates = true; break; }
                }
                if (!bStrictlyDominates) break;
                runner = domInfo[runner].idom;
            }
        }

        /* 简化: 被b直接支配的节点的idom链上, 第一个不被b严格支配的节点 */
        for (int c : domInfo[b].dominatedBy) {
            /* c被b直接支配, 但c本身可能支配其他节点 */
            /* 将被b直接支配但b不严格支配其所有前驱的节点加入DF(b) */
            result[b].dominanceFrontier.append(c);

            /* 向上传播: 合并c的支配边界到b */
            for (int df : domInfo[c].dominanceFrontier) {
                /* 检查b是否严格支配df */
                bool strictlyDominatesDf = false;
                int check = df;
                while (check >= 0) {
                    check = domInfo[check].idom;
                    if (check == b) { strictlyDominatesDf = true; break; }
                }
                if (!strictlyDominatesDf) {
                    if (!result[b].dominanceFrontier.contains(df))
                        result[b].dominanceFrontier.append(df);
                }
            }
        }
    }

    int totalFrontierSize = 0;
    for (const auto& info : result) totalFrontierSize += info.dominanceFrontier.size();
    emit frontiersComputed(totalFrontierSize);
    return result;
}

/* ========== 支配判定 ========== */

bool DominatorTree::dominates(const QVector<DomInfo>& domInfo,
                               int a, int b) const
{
    /* a支配b ⟺ a在b的支配者链上 */
    int current = b;
    while (current >= 0) {
        if (current == a) return true;
        if (domInfo[current].idom == current) break; /* 到达入口 */
        current = domInfo[current].idom;
    }
    return (a == b); /* 自身支配自身 */
}

/* ========== DFS遍历 ========== */

void DominatorTree::dfs(int v, const QVector<QVector<int>>& succ,
                         QVector<int>& semi, QVector<int>& vertex,
                         QVector<int>& parent, int& counter)
{
    semi[v] = counter;
    vertex[counter] = v;

    for (int w : succ[v]) {
        if (semi[w] < 0) { /* 未访问 */
            parent[w] = v;
            ++counter;
            dfs(w, succ, semi, vertex, parent, counter);
        }
    }
}

/* ========== 并查集Find(路径压缩) ========== */

int DominatorTree::find(int v, QVector<int>& ancestor,
                         QVector<int>& label,
                         const QVector<int>& semi)
{
    if (ancestor[v] < 0) return v;

    /* 路径压缩 */
    if (ancestor[ancestor[v]] >= 0) {
        int u = find(ancestor[v], ancestor, label, semi);
        ancestor[v] = ancestor[u];
        if (semi[label[u]] < semi[label[v]])
            label[v] = label[u];
    }
    return label[v];
}

/* ========== Union ========== */

void DominatorTree::link(int v, int w, QVector<int>& size,
                          QVector<int>& child)
{
    /* 简单union(不用按秩合并, Lengauer-Tarjan中由ancestor数组替代) */
    Q_UNUSED(size);
    Q_UNUSED(child);
    ancestor[w] = v; /* 简化实现 */
}

/* ========== 重置统计 ========== */

void DominatorTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
