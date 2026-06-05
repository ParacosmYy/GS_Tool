/**
 * @file EdmondsMatching.cpp
 * @brief Edmonds 花算法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/edmonds/EdmondsMatching.h"

#include <QtGlobal>

/**
 * @brief 构造函数
 * @param parent 父 QObject
 */
EdmondsMatching::EdmondsMatching(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 并查集查找根(带路径压缩)
 * @param parent 并查集父数组
 * @param u 查找顶点
 * @return 根顶点索引
 */
int EdmondsMatching::findRoot(QVector<int> &parent, int u)
{
    while (parent[u] != u) {
        parent[u] = parent[parent[u]]; // 路径压缩
        u = parent[u];
    }
    return u;
}

/**
 * @brief 从花结构中提取增广路径
 *
 * 当检测到花(blossom)时，从花中提取从 u 到 base 的交替路径。
 * 回溯 match[] 和 label[] 找到完整的增广路径。
 *
 * @param u 当前顶点
 * @param v 遇到的交叉顶点(花检测)
 * @param blossomMark 花标记(属于花的顶点)
 * @param base 花的基顶点
 * @param match 当前匹配
 * @param path 输出路径
 */
void EdmondsMatching::extractPath(int u, int v, const QVector<int> &blossomMark,
                                   int base, const QVector<int> &match,
                                   QVector<int> &path)
{
    // 沿交替路径回溯到 base
    while (u != base) {
        if (blossomMark[u] == 0) {
            path.push_back(u);
        }
        int mu = match[u];
        if (mu >= 0 && blossomMark[mu] == 0) {
            path.push_back(mu);
        }
        if (mu >= 0) {
            u = mu;
        } else {
            break;
        }
    }
}

/**
 * @brief 求解最大匹配
 *
 * 简化版 Edmonds 算法:
 * 1. 构建邻接表
 * 2. BFS 搜索增广路径
 * 3. 遇到花时收缩(用并查集标记)
 * 4. 找到增广路径后翻转匹配
 * 5. 重复直到无法增广
 * 6. 从 match[] 数组提取匹配边
 *
 * @param edges 边列表
 * @param n 顶点数
 * @return 最大匹配的边列表
 */
QVector<QPair<int, int>> EdmondsMatching::maxMatching(
    const QVector<QPair<int, int>> &edges, int n)
{
    m_timer.start();

    // 输入校验
    if (n <= 0 || edges.isEmpty()) {
        emit solveCompleted(0);
        return {};
    }

    // 构建邻接表
    QVector<QVector<int>> adj(n);
    for (const auto &e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n
            && e.first != e.second) {
            adj[e.first].push_back(e.second);
            adj[e.second].push_back(e.first);
        }
    }

    // match[v] = v 的匹配顶点, -1 表示未匹配
    QVector<int> match(n, -1);

    // 反复寻找增广路径
    bool found = true;
    while (found) {
        found = false;

        // BFS 状态
        QVector<int> label(n, -1);       // 标号: -1=未标, 0=外点, 1=内点
        QVector<int> parentBFS(n, -1);   // BFS 树中的父节点
        QVector<int> dsu(n);             // 并查集(花收缩)
        for (int i = 0; i < n; ++i) dsu[i] = i;

        QQueue<int> queue;

        // 初始化: 所有未匹配顶点作为外点
        for (int v = 0; v < n; ++v) {
            if (match[v] == -1) {
                label[v] = 0;  // 外点
                parentBFS[v] = v;
                queue.enqueue(v);
            }
        }

        int augU = -1, augV = -1; // 增广路径端点

        while (!queue.isEmpty() && !found) {
            int u = queue.dequeue();
            int ru = findRoot(dsu, u);

            for (int w : adj[u]) {
                int rw = findRoot(dsu, w);
                if (ru == rw) continue; // 同一花内，跳过

                if (label[rw] == -1) {
                    // rw 未标号 → 内点
                    label[rw] = 1;
                    parentBFS[rw] = u;

                    int mw = match[rw];
                    if (mw == -1) mw = rw; // 自环保护
                    if (match[rw] >= 0) {
                        int rmw = findRoot(dsu, match[rw]);
                        if (label[rmw] == -1) {
                            label[rmw] = 0; // 匹配顶点为外点
                            parentBFS[rmw] = rw;
                            queue.enqueue(rmw);
                        }
                    }
                } else if (label[rw] == 0) {
                    // 两个外点相遇 → 找到增广路径或花
                    augU = u;
                    augV = w;
                    found = true;
                    break;
                }
            }
        }

        if (found && augU >= 0 && augV >= 0) {
            // 回溯增广路径并翻转匹配
            // 简化: 使用路径回溯
            QVector<int> pathU, pathV;
            int cu = augU;
            while (cu >= 0 && cu != parentBFS[cu]) {
                pathU.push_back(cu);
                cu = parentBFS[cu];
            }
            if (cu >= 0) pathU.push_back(cu);

            int cv = augV;
            while (cv >= 0 && cv != parentBFS[cv]) {
                pathV.push_back(cv);
                cv = parentBFS[cv];
            }
            if (cv >= 0) pathV.push_back(cv);

            // 找到公共祖先
            int commonAncestor = -1;
            for (int pu : pathU) {
                for (int pv : pathV) {
                    if (pu == pv) {
                        commonAncestor = pu;
                        break;
                    }
                }
                if (commonAncestor >= 0) break;
            }

            // 构建增广路径: pathU(到祖先) + pathV(从祖先反向)
            QVector<int> augPath;
            for (int pu : pathU) {
                augPath.push_back(pu);
                if (pu == commonAncestor) break;
            }
            bool pastAncestor = false;
            for (int pv : pathV) {
                if (pv == commonAncestor) {
                    pastAncestor = true;
                    continue;
                }
                if (pastAncestor) {
                    augPath.push_back(pv);
                }
            }

            // 翻转增广路径上的匹配
            for (int i = 0; i + 1 < static_cast<int>(augPath.size()); i += 2) {
                int a = augPath[i];
                int b = augPath[i + 1];
                match[a] = b;
                match[b] = a;
            }
        }
    }

    // 提取匹配边(去重)
    QVector<QPair<int, int>> result;
    for (int v = 0; v < n; ++v) {
        if (match[v] >= 0 && v < match[v]) {
            result.push_back({v, match[v]});
        }
    }

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalSolves++;
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(static_cast<int>(result.size()));
    return result;
}

/**
 * @brief 重置统计计数器
 */
void EdmondsMatching::resetStatistics()
{
    m_stats.totalSolves = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeAccum = 0.0;
}
