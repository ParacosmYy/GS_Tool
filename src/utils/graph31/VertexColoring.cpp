/**
 * @file VertexColoring.cpp
 * @brief 图顶点着色实现 — 贪心/Welsh-Powell/DSATUR算法
 */

#include "utils/graph31/VertexColoring.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <queue>
#include <numeric>

/* ──────────────────── 构造/析构 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
VertexColoring::VertexColoring(QObject* parent)
    : QObject(parent)
    , m_numVertices(0)
{
}

/** @brief 析构函数 */
VertexColoring::~VertexColoring() = default;

/* ──────────────────── 图构建 ──────────────────── */

/** @brief 创建空图 @param numVertices 顶点数 */
void VertexColoring::createGraph(int numVertices)
{
    m_numVertices = numVertices;
    m_adj.assign(numVertices, QVector<int>());
}

/** @brief 添加无向边 @param u 顶点u @param v 顶点v */
void VertexColoring::addEdge(int u, int v)
{
    if (u < 0 || u >= m_numVertices || v < 0 || v >= m_numVertices) return;
    if (u == v) return;

    /* 避免重复边 */
    if (!m_adj[u].contains(v)) {
        m_adj[u].append(v);
        m_adj[v].append(u);
    }
}

/** @brief 从边列表构建图 @param numVertices 顶点数 @param edges 边列表 */
void VertexColoring::buildFromEdges(int numVertices,
                                    const QVector<QPair<int, int>>& edges)
{
    createGraph(numVertices);
    for (const auto& edge : edges) {
        addEdge(edge.first, edge.second);
    }
}

/** @brief 从邻接矩阵构建图 @param adjacencyMatrix 邻接矩阵 */
void VertexColoring::buildFromMatrix(const QVector<QVector<int>>& adjacencyMatrix)
{
    int n = adjacencyMatrix.size();
    createGraph(n);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (i < adjacencyMatrix.size() && j < adjacencyMatrix[i].size()) {
                if (adjacencyMatrix[i][j] != 0) {
                    addEdge(i, j);
                }
            }
        }
    }
}

/** @brief 清空图 */
void VertexColoring::clear()
{
    m_numVertices = 0;
    m_adj.clear();
}

/* ──────────────────── 着色算法 ──────────────────── */

/** @brief 执行顶点着色 @param algorithm 着色算法 @return 着色结果 */
VertexColoring::ColoringResult VertexColoring::color(Algorithm algorithm)
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    if (m_numVertices == 0) return result;

    switch (algorithm) {
    case Greedy: {
        QVector<int> order(m_numVertices);
        std::iota(order.begin(), order.end(), 0);
        result = greedyColor(order);
        break;
    }
    case WelshPowell: {
        QVector<int> order = welshPowellOrder();
        result = greedyColor(order);
        break;
    }
    case DSATUR:
        result = dsaturColor();
        break;
    }

    /* 验证着色合法性 */
    auto conflicts = validateColoring(result.colors);
    result.totalConflicts = conflicts.size();
    result.isValid = conflicts.isEmpty();

    /* 统计 */
    m_stats.totalColorings++;
    m_stats.totalVerticesProcessed += static_cast<quint64>(m_numVertices);
    m_stats.totalEdgesChecked += static_cast<quint64>(graphInfo().numEdges);
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalColorings, 1ULL));

    QString algoName;
    switch (algorithm) {
    case Greedy: algoName = tr("贪心"); break;
    case WelshPowell: algoName = tr("Welsh-Powell"); break;
    case DSATUR: algoName = tr("DSATUR"); break;
    }
    emit coloringCompleted(result.numColors, algoName);
    return result;
}

/** @brief 比较所有算法结果 @return 算法名→着色结果 */
QMap<QString, VertexColoring::ColoringResult> VertexColoring::compareAll()
{
    QMap<QString, ColoringResult> results;
    results[tr("贪心")] = color(Greedy);
    results[tr("Welsh-Powell")] = color(WelshPowell);
    results[tr("DSATUR")] = color(DSATUR);
    return results;
}

/* ──────────────────── 验证与分析 ──────────────────── */

/** @brief 验证着色合法性 @param colors 颜色分配 @return 冲突边列表 */
QVector<QPair<int, int>> VertexColoring::validateColoring(const QVector<int>& colors) const
{
    QVector<QPair<int, int>> conflicts;

    for (int u = 0; u < m_numVertices; ++u) {
        if (u >= colors.size()) continue;
        for (int v : m_adj[u]) {
            if (v > u && v < colors.size()) {
                if (colors[u] == colors[v] && colors[u] >= 0) {
                    conflicts.append({u, v});
                    emit conflictFound(u, v, colors[u]);
                }
            }
        }
    }

    return conflicts;
}

/** @brief 计算色数上下界 @return 色数界 */
VertexColoring::ChromaticBounds VertexColoring::chromaticBounds() const
{
    ChromaticBounds bounds;

    if (m_numVertices == 0) return bounds;

    int maxDeg = 0;
    for (int i = 0; i < m_numVertices; ++i) {
        int deg = degree(i);
        if (deg > maxDeg) maxDeg = deg;
    }

    /* 上界: Brooks定理 χ(G) ≤ Δ(G) + 1 (非完全图或奇环时 χ ≤ Δ) */
    bounds.upperBound = maxDeg + 1;

    /* 下界: 最大团大小(近似) */
    int cliqueSize = maxCliqueApprox();
    bounds.lowerBound = cliqueSize;

    /* 启发式估计 */
    double avgDeg = 0.0;
    for (int i = 0; i < m_numVertices; ++i) avgDeg += degree(i);
    avgDeg /= static_cast<double>(m_numVertices);
    bounds.heuristicEstimate = avgDeg / (2.0 * qLn(static_cast<double>(cliqueSize) + 1.0) + 1.0)
                             * static_cast<double>(maxDeg) / (avgDeg + 1.0);
    bounds.heuristicEstimate = qBound(static_cast<double>(bounds.lowerBound),
                                      bounds.heuristicEstimate,
                                      static_cast<double>(bounds.upperBound));

    return bounds;
}

/** @brief 获取图信息 @return 图统计 */
VertexColoring::GraphInfo VertexColoring::graphInfo() const
{
    GraphInfo info;
    info.numVertices = m_numVertices;

    int totalEdges = 0;
    int maxDeg = 0;
    for (int i = 0; i < m_numVertices; ++i) {
        int deg = m_adj[i].size();
        totalEdges += deg;
        if (deg > maxDeg) maxDeg = deg;
    }

    info.numEdges = totalEdges / 2; /* 无向图每条边计数两次 */
    info.maxDegree = maxDeg;
    info.avgDegree = m_numVertices > 0
        ? static_cast<double>(totalEdges) / static_cast<double>(m_numVertices)
        : 0.0;
    info.isConnected = countComponents() <= 1;

    return info;
}

/* ──────────────────── 私有着色方法 ──────────────────── */

/** @brief 贪心着色 @param order 顶点处理顺序 @return 着色结果 */
VertexColoring::ColoringResult VertexColoring::greedyColor(const QVector<int>& order)
{
    ColoringResult result;
    result.colors.fill(-1, m_numVertices);
    int iters = 0;

    for (int v : order) {
        /* 收集已使用的颜色 */
        QVector<bool> used(m_numVertices + 1, false);
        for (int neighbor : m_adj[v]) {
            if (neighbor < result.colors.size() && result.colors[neighbor] >= 0) {
                used[result.colors[neighbor]] = true;
            }
        }

        /* 分配最小可用颜色 */
        int color = 0;
        while (used[color]) ++color;
        result.colors[v] = color;
        ++iters;
    }

    /* 统计颜色数 */
    int maxColor = 0;
    for (int c : result.colors) {
        if (c > maxColor) maxColor = c;
    }
    result.numColors = maxColor + 1;
    result.iterations = iters;

    return result;
}

/** @brief Welsh-Powell排序(按度数降序) @return 顶点顺序 */
QVector<int> VertexColoring::welshPowellOrder() const
{
    QVector<int> order(m_numVertices);
    std::iota(order.begin(), order.end(), 0);

    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return degree(a) > degree(b);
    });

    return order;
}

/** @brief DSATUR着色 @return 着色结果 */
VertexColoring::ColoringResult VertexColoring::dsaturColor()
{
    ColoringResult result;
    result.colors.fill(-1, m_numVertices);

    /* 饱和度 = 邻居中已使用不同颜色数 */
    QVector<int> saturation(m_numVertices, 0);
    QVector<bool> colored(m_numVertices, false);
    int iters = 0;

    for (int step = 0; step < m_numVertices; ++step) {
        /* 选择饱和度最高(且未着色)的顶点 */
        int best = -1;
        int bestSat = -1;
        int bestDeg = -1;

        for (int v = 0; v < m_numVertices; ++v) {
            if (colored[v]) continue;
            if (saturation[v] > bestSat ||
                (saturation[v] == bestSat && degree(v) > bestDeg)) {
                bestSat = saturation[v];
                bestDeg = degree(v);
                best = v;
            }
        }

        if (best < 0) break;

        /* 贪心分配颜色 */
        QVector<bool> used(m_numVertices + 1, false);
        for (int neighbor : m_adj[best]) {
            if (colored[neighbor] && result.colors[neighbor] >= 0) {
                used[result.colors[neighbor]] = true;
            }
        }

        int color = 0;
        while (used[color]) ++color;
        result.colors[best] = color;
        colored[best] = true;

        /* 更新邻居饱和度 */
        for (int neighbor : m_adj[best]) {
            if (!colored[neighbor]) {
                /* 检查这是否为邻居的第一种该颜色 */
                bool colorExists = false;
                for (int nn : m_adj[neighbor]) {
                    if (colored[nn] && result.colors[nn] == color && nn != best) {
                        colorExists = true;
                        break;
                    }
                }
                if (!colorExists) {
                    saturation[neighbor]++;
                }
            }
        }

        ++iters;
    }

    int maxColor = 0;
    for (int c : result.colors) {
        if (c > maxColor) maxColor = c;
    }
    result.numColors = maxColor + 1;
    result.iterations = iters;

    return result;
}

/* ──────────────────── 辅助方法 ──────────────────── */

/** @brief 计算顶点度数 @param v 顶点 @return 度数 */
int VertexColoring::degree(int v) const
{
    if (v < 0 || v >= m_numVertices) return 0;
    return m_adj[v].size();
}

/** @brief 近似最大团(贪心) @return 团大小 */
int VertexColoring::maxCliqueApprox() const
{
    if (m_numVertices == 0) return 0;

    /* 贪心: 按度数降序尝试扩展团 */
    QVector<int> order(m_numVertices);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return degree(a) > degree(b);
    });

    int maxClique = 1;

    for (int start = 0; start < qMin(m_numVertices, 20); ++start) {
        int v = order[start];
        QVector<int> clique = {v};

        /* 尝试添加与当前团全连接的顶点 */
        for (int u = start + 1; u < m_numVertices; ++u) {
            int candidate = order[u];
            bool allConnected = true;
            for (int c : clique) {
                if (!m_adj[c].contains(candidate)) {
                    allConnected = false;
                    break;
                }
            }
            if (allConnected) {
                clique.append(candidate);
            }
        }

        if (clique.size() > maxClique) {
            maxClique = clique.size();
        }
    }

    return maxClique;
}

/** @brief BFS计算连通分量数 @return 分量数 */
int VertexColoring::countComponents() const
{
    if (m_numVertices == 0) return 0;

    QVector<bool> visited(m_numVertices, false);
    int components = 0;

    for (int start = 0; start < m_numVertices; ++start) {
        if (visited[start]) continue;

        ++components;
        std::queue<int> q;
        q.push(start);
        visited[start] = true;

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (int neighbor : m_adj[v]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
    }

    return components;
}

/* ──────────────────── 统计 ──────────────────── */

/** @brief 获取统计 @return 统计 */
VertexColoring::Stats VertexColoring::stats() const { return m_stats; }

/** @brief 重置统计 */
void VertexColoring::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
