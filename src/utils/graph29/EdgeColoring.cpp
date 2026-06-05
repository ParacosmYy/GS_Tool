/**
 * @file EdgeColoring.cpp
 * @brief 边着色引擎实现 — Vizing定理近似 + 匹配着色
 */

#include "utils/graph29/EdgeColoring.h"

#include <QElapsedTimer>
#include <QSet>

#include <algorithm>
#include <queue>

// 构造 / 析构

EdgeColoring::EdgeColoring(QObject* parent)
    : QObject(parent)
{
}

EdgeColoring::~EdgeColoring() = default;

// Vizing贪心边着色

EdgeColoring::ColoringResult EdgeColoring::colorVizing(
    const QVector<QPair<int, int>>& edges, int numVertices)
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    result.vertexCount = numVertices;
    result.edgeCount = edges.size();

    if (edges.isEmpty() || numVertices <= 0) {
        result.success = true;
        result.chromaticIndex = 0;
        return result;
    }

    int maxDeg = computeMaxDegree(edges, numVertices);
    result.maxDegree = maxDeg;
    int numColors = maxDeg + 1; /* Vizing上界 */

    /* 跟踪每个顶点已使用的颜色 */
    QVector<QSet<int>> vertexColors(numVertices);

    /* 跟踪每个顶点在每种颜色下相邻的边 */
    /* freeColor[v]: 顶点v的可用颜色 */
    auto getFreeColor = [&](int v) -> int {
        for (int c = 0; c < numColors; ++c) {
            if (!vertexColors[v].contains(c)) return c;
        }
        return numColors; /* 不应到达 */
    };

    /* Misra & Gries边着色算法(简化版) */
    /* 逐边着色, 使用颜色交替路径 */
    for (const auto& edge : edges) {
        int u = edge.first;
        int v = edge.second;

        int freeU = getFreeColor(u);
        int freeV = getFreeColor(v);

        if (freeU == freeV) {
            /* 两端有相同的空闲颜色, 直接着色 */
            result.edgeColors[edge] = freeU;
            vertexColors[u].insert(freeU);
            vertexColors[v].insert(freeU);
        } else {
            /* 构建fan: 从u出发的交替路径 */
            /* 简化: 尝试从u进行颜色交替 */
            bool colored = false;

            /* 尝试在u的空闲色和v的空闲色之间交替 */
            /* 构建最大fan: {v=v0, v1, v2, ...} */
            QVector<int> fan;
            fan.append(v);

            /* 扩展fan: 找u的邻居w, 使得edge(u,w)的颜色==freeV */
            for (const auto& e2 : edges) {
                int w = -1;
                if (e2.first == u && e2.second != v) w = e2.second;
                if (e2.second == u && e2.first != v) w = e2.first;
                if (w < 0) continue;

                auto it = result.edgeColors.find(
                    qMakePair(qMin(u, w), qMax(u, w)));
                if (it != result.edgeColors.end() && it.value() == freeV) {
                    fan.append(w);
                }
            }

            /* 反转颜色交替路径: freeV <-> freeU */
            /* 从fan末端开始反转 */
            for (int i = fan.size() - 1; i >= 0; --i) {
                int fi = fan[i];
                auto edgeKey = qMakePair(qMin(u, fi), qMax(u, fi));

                if (i == 0) {
                    /* 原始边(u,v) */
                    result.edgeColors[edge] = freeU;
                    vertexColors[u].insert(freeU);
                    vertexColors[v].insert(freeU);
                } else {
                    /* 将edge(u, fan[i])的颜色改为freeU */
                    int oldColor = result.edgeColors.value(edgeKey, -1);
                    if (oldColor >= 0) {
                        vertexColors[u].remove(oldColor);
                        vertexColors[fi].remove(oldColor);
                    }
                    result.edgeColors[edgeKey] = freeU;
                    vertexColors[u].insert(freeU);
                    vertexColors[fi].insert(freeU);
                }
            }
            colored = true;
        }
    }

    /* 统计实际使用的颜色数 */
    QSet<int> usedColors;
    for (auto it = result.edgeColors.constBegin();
         it != result.edgeColors.constEnd(); ++it) {
        usedColors.insert(it.value());
    }
    result.chromaticIndex = usedColors.size();
    result.isOptimal = (result.chromaticIndex == maxDeg);
    result.success = true;

    m_stats.totalColorings++;
    m_stats.totalEdgesProcessed += edges.size();
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalColorings);

    emit coloringCompleted(result.chromaticIndex, edges.size());
    return result;
}

// 匹配分解边着色

EdgeColoring::ColoringResult EdgeColoring::colorByMatchings(
    const QVector<QPair<int, int>>& edges, int numVertices)
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    result.vertexCount = numVertices;
    result.edgeCount = edges.size();

    if (edges.isEmpty()) {
        result.success = true;
        return result;
    }

    int maxDeg = computeMaxDegree(edges, numVertices);
    result.maxDegree = maxDeg;

    /* 逐颜色提取最大匹配 */
    QVector<QPair<int, int>> remaining = edges;
    int color = 0;

    while (!remaining.isEmpty()) {
        /* 当前颜色: 找一组不相交的边(匹配) */
        QVector<QPair<int, int>> matching;
        QSet<int> usedVertices;

        /* 贪心匹配: 按度数排序后贪心选择 */
        auto adj = buildAdjacencyList(remaining, numVertices);

        /* 按顶点度数升序处理边(贪心近似) */
        QVector<QPair<int, int>> sortedRemaining = remaining;
        std::sort(sortedRemaining.begin(), sortedRemaining.end(),
            [&](const QPair<int, int>& a, const QPair<int, int>& b) {
                int degA = adj[a.first].size() + adj[a.second].size();
                int degB = adj[b.first].size() + adj[b.second].size();
                return degA < degB;
            });

        for (const auto& edge : sortedRemaining) {
            if (!usedVertices.contains(edge.first) &&
                !usedVertices.contains(edge.second)) {
                matching.append(edge);
                usedVertices.insert(edge.first);
                usedVertices.insert(edge.second);
            }
        }

        /* 将匹配中的边着当前色 */
        for (const auto& edge : matching) {
            auto key = qMakePair(qMin(edge.first, edge.second),
                                 qMax(edge.first, edge.second));
            result.edgeColors[key] = color;
        }

        /* 从剩余边中移除匹配 */
        QSet<QPair<int, int>> matchSet;
        for (const auto& e : matching) {
            matchSet.insert(qMakePair(qMin(e.first, e.second),
                                      qMax(e.first, e.second)));
        }

        QVector<QPair<int, int>> newRemaining;
        for (const auto& e : remaining) {
            auto key = qMakePair(qMin(e.first, e.second),
                                 qMax(e.first, e.second));
            if (!matchSet.contains(key)) {
                newRemaining.append(e);
            }
        }
        remaining = newRemaining;
        color++;
    }

    result.chromaticIndex = color;
    result.isOptimal = (color == maxDeg);
    result.success = true;

    m_stats.totalColorings++;
    m_stats.totalEdgesProcessed += edges.size();
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalColorings);

    emit coloringCompleted(result.chromaticIndex, edges.size());
    return result;
}

// 二分图最优边着色

EdgeColoring::ColoringResult EdgeColoring::colorBipartite(
    const QVector<QPair<int, int>>& edges,
    int numVertices, int leftSize)
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    result.vertexCount = numVertices;
    result.edgeCount = edges.size();

    if (edges.isEmpty()) {
        result.success = true;
        return result;
    }

    int maxDeg = computeMaxDegree(edges, numVertices);
    result.maxDegree = maxDeg;

    /* 二分图: Delta着色总是最优 */
    /* 使用增广路径方法 */
    QVector<QSet<int>> vertexColors(numVertices);

    /* 将每条边编号 */
    for (int i = 0; i < edges.size(); ++i) {
        int u = edges[i].first;
        int v = edges[i].second;
        auto key = qMakePair(qMin(u, v), qMax(u, v));

        /* 找两端都未使用的颜色 */
        int chosenColor = -1;
        for (int c = 0; c < maxDeg; ++c) {
            if (!vertexColors[u].contains(c) &&
                !vertexColors[v].contains(c)) {
                chosenColor = c;
                break;
            }
        }

        if (chosenColor >= 0) {
            /* 两端都有空闲颜色 */
            result.edgeColors[key] = chosenColor;
            vertexColors[u].insert(chosenColor);
            vertexColors[v].insert(chosenColor);
        } else {
            /* 需要颜色交替路径 */
            int freeU = -1, freeV = -1;
            for (int c = 0; c < maxDeg; ++c) {
                if (!vertexColors[u].contains(c)) { freeU = c; break; }
            }
            for (int c = 0; c < maxDeg; ++c) {
                if (!vertexColors[v].contains(c)) { freeV = c; break; }
            }

            /* 在二分图中构建颜色freeU/freeV的交替路径 */
            /* 从v出发, 交替查找freeU/freeV颜色的边 */
            QMap<int, int> augmentPath;
            augmentPath[v] = -1;
            std::queue<int> bfs;
            bfs.push(v);

            int pathEnd = -1;
            while (!bfs.empty() && pathEnd < 0) {
                int curr = bfs.front();
                bfs.pop();

                /* 当前查找的颜色(交替) */
                bool isLeft = (curr < leftSize);
                int searchColor = isLeft ? freeV : freeU;

                for (const auto& e : edges) {
                    int neighbor = -1;
                    if (e.first == curr) neighbor = e.second;
                    else if (e.second == curr) neighbor = e.first;
                    if (neighbor < 0 || augmentPath.contains(neighbor)) continue;

                    auto eKey = qMakePair(qMin(curr, neighbor),
                                          qMax(curr, neighbor));
                    if (result.edgeColors.value(eKey, -1) == searchColor) {
                        augmentPath[neighbor] = curr;

                        /* 检查neighbor是否是路径终点 */
                        int otherFree = isLeft ? freeU : freeV;
                        if (!vertexColors[neighbor].contains(otherFree)) {
                            pathEnd = neighbor;
                            break;
                        }
                        bfs.push(neighbor);
                    }
                }
            }

            /* 沿交替路径反转颜色 */
            if (pathEnd >= 0) {
                int curr = pathEnd;
                while (curr != v) {
                    int prev = augmentPath[curr];
                    auto eKey = qMakePair(qMin(curr, prev),
                                          qMax(curr, prev));

                    bool isCurrLeft = (curr < leftSize);
                    int oldColor = result.edgeColors.value(eKey, 0);
                    int newColor = isCurrLeft ? freeU : freeV;

                    vertexColors[curr].remove(oldColor);
                    vertexColors[prev].remove(oldColor);
                    result.edgeColors[eKey] = newColor;
                    vertexColors[curr].insert(newColor);
                    vertexColors[prev].insert(newColor);

                    /* 交换freeU/freeV用于下一轮 */
                    std::swap(freeU, freeV);
                    curr = prev;
                }
            }

            /* 现在u和v有共同的空闲颜色 */
            int commonFree = -1;
            for (int c = 0; c < maxDeg; ++c) {
                if (!vertexColors[u].contains(c) &&
                    !vertexColors[v].contains(c)) {
                    commonFree = c;
                    break;
                }
            }

            if (commonFree >= 0) {
                result.edgeColors[key] = commonFree;
                vertexColors[u].insert(commonFree);
                vertexColors[v].insert(commonFree);
            }
        }
    }

    QSet<int> usedColors;
    for (auto it = result.edgeColors.constBegin();
         it != result.edgeColors.constEnd(); ++it) {
        usedColors.insert(it.value());
    }
    result.chromaticIndex = usedColors.size();
    result.isOptimal = (result.chromaticIndex <= maxDeg);
    result.success = true;

    m_stats.totalColorings++;
    m_stats.totalEdgesProcessed += edges.size();
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs =
        m_timeSum / static_cast<double>(m_stats.totalColorings);

    emit coloringCompleted(result.chromaticIndex, edges.size());
    return result;
}

// 验证

bool EdgeColoring::verifyColoring(
    const QVector<QPair<int, int>>& edges,
    const QMap<QPair<int, int>, int>& edgeColors) const
{
    /* 检查每对相邻边是否有相同颜色 */
    for (int i = 0; i < edges.size(); ++i) {
        auto keyI = qMakePair(qMin(edges[i].first, edges[i].second),
                              qMax(edges[i].first, edges[i].second));
        if (!edgeColors.contains(keyI)) return false;

        for (int j = i + 1; j < edges.size(); ++j) {
            /* 检查边i和边j是否共享顶点 */
            if (edges[i].first == edges[j].first ||
                edges[i].first == edges[j].second ||
                edges[i].second == edges[j].first ||
                edges[i].second == edges[j].second) {
                auto keyJ = qMakePair(qMin(edges[j].first, edges[j].second),
                                      qMax(edges[j].first, edges[j].second));
                if (edgeColors.value(keyI, -1) == edgeColors.value(keyJ, -2)) {
                    return false;
                }
            }
        }
    }
    return true;
}

int EdgeColoring::computeMaxDegree(
    const QVector<QPair<int, int>>& edges, int numVertices)
{
    QVector<int> degree(numVertices, 0);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < numVertices) degree[e.first]++;
        if (e.second >= 0 && e.second < numVertices) degree[e.second]++;
    }
    int maxDeg = 0;
    for (int d : degree) maxDeg = qMax(maxDeg, d);
    return maxDeg;
}

// 统计

EdgeColoring::Stats EdgeColoring::stats() const
{
    return m_stats;
}

void EdgeColoring::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// 内部实现

QVector<QVector<int>> EdgeColoring::buildAdjacencyList(
    const QVector<QPair<int, int>>& edges, int numVertices)
{
    QVector<QVector<int>> adj(numVertices);
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < numVertices &&
            e.second >= 0 && e.second < numVertices) {
            adj[e.first].append(e.second);
            adj[e.second].append(e.first);
        }
    }
    return adj;
}

QVector<QPair<int, int>> EdgeColoring::findMaxMatching(
    const QVector<QVector<int>>& adj,
    const QSet<int>& leftVertices,
    int numVertices) const
{
    QVector<int> matchTo(numVertices, -1);
    QVector<QPair<int, int>> matching;

    for (int u : leftVertices) {
        QVector<bool> visited(numVertices, false);
        /* 简化DFS增广 */
        std::queue<int> q;
        q.push(u);
        visited[u] = true;
        int augmentEnd = -1;

        while (!q.empty() && augmentEnd < 0) {
            int curr = q.front();
            q.pop();

            for (int v : adj[curr]) {
                if (visited[v]) continue;
                visited[v] = true;

                if (matchTo[v] < 0) {
                    /* 找到增广路径终点 */
                    augmentEnd = v;
                    matchTo[v] = curr;
                    matchTo[curr] = v;
                    matching.append({qMin(curr, v), qMax(curr, v)});
                    break;
                } else {
                    q.push(matchTo[v]);
                }
            }
        }
    }

    return matching;
}
