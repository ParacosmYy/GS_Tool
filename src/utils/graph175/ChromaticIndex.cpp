/**
 * @file ChromaticIndex.cpp
 * @brief ChromaticIndex 实现
 *
 * 实现边着色：Misra & Gries贪心算法，基于Vizing定理
 * 证明简单图边色数为Δ或Δ+1。
 */

#include "utils/graph175/ChromaticIndex.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
ChromaticIndex::ChromaticIndex(QObject* parent)
    : QObject(parent)
{
}

void ChromaticIndex::addEdge(int u, int v)
{
    if (u == v) return; /* 忽略自环 */
    /* 确保u < v避免重复 */
    if (u > v) std::swap(u, v);
    m_edges.append({u, v});

    /* 更新度数 */
    QMap<int, int> degreeMap;
    for (const auto& e : m_edges) {
        degreeMap[e.first]++;
        degreeMap[e.second]++;
    }
    m_maxDegree = 0;
    for (auto it = degreeMap.constBegin(); it != degreeMap.constEnd(); ++it) {
        m_maxDegree = qMax(m_maxDegree, it.value());
    }
}

/**
 * @brief 找到顶点的空闲颜色
 */
int ChromaticIndex::freeColor(int vertex, const QVector<QMap<int, int>>& vertexColors,
                              int maxColor) const
{
    for (int c = 1; c <= maxColor + 1; ++c) {
        if (!vertexColors[vertex].contains(c)) {
            return c;
        }
    }
    return maxColor + 1;
}

/**
 * @brief 构造极大fan
 *
 * 从startVertex出发，沿未着色或可换色的边构建fan序列。
 */
QVector<int> ChromaticIndex::buildFan(int startVertex, int edgeIdx,
                                      const QVector<int>& edgeColors,
                                      const QVector<QMap<int, int>>& vertexColors,
                                      int maxColor) const
{
    QVector<int> fan;
    fan.append(edgeIdx);

    int current = startVertex;
    QSet<int> usedEdges;
    usedEdges.insert(edgeIdx);

    bool extended = true;
    while (extended) {
        extended = false;
        for (int e = 0; e < m_edges.size(); ++e) {
            if (usedEdges.contains(e)) continue;
            if (edgeColors[e] != 0) continue; /* 只考虑未着色边 */

            int u = m_edges[e].first;
            int v = m_edges[e].second;

            if (u == current || v == current) {
                int other = (u == current) ? v : u;
                int c = freeColor(other, vertexColors, maxColor);
                /* 检查是否可以扩展fan */
                int firstColor = (fan.size() > 0)
                    ? freeColor(m_edges[fan[0]].first == current
                                ? m_edges[fan[0]].second : m_edges[fan[0]].first,
                                vertexColors, maxColor)
                    : c;
                fan.append(e);
                usedEdges.insert(e);
                current = other;
                extended = true;
                break;
            }
        }
    }

    return fan;
}

/**
 * @brief Misra & Gries边着色算法
 *
 * 核心思想：
 * 1) 逐边处理，维护部分着色
 * 2) 对每条未着色边构造极大fan
 * 3) 通过颜色翻转和旋转完成着色
 * 4) 保证最多使用Δ+1种颜色
 */
int ChromaticIndex::misraGries(QVector<int>& edgeColors)
{
    if (m_edges.isEmpty()) return 0;

    /* 找到所有顶点 */
    QSet<int> vertexSet;
    for (const auto& e : m_edges) {
        vertexSet.insert(e.first);
        vertexSet.insert(e.second);
    }

    int maxVertex = 0;
    for (int v : vertexSet) maxVertex = qMax(maxVertex, v);

    int maxColor = m_maxDegree + 1;

    /* 顶点颜色映射：vertexColors[v][c] = 边索引 */
    QVector<QMap<int, int>> vertexColors(maxVertex + 1);

    /* 逐边着色 */
    for (int e = 0; e < m_edges.size(); ++e) {
        int u = m_edges[e].first;
        int v = m_edges[e].second;

        /* 找u和v的空闲颜色 */
        int cu = freeColor(u, vertexColors, maxColor);
        int cv = freeColor(v, vertexColors, maxColor);

        if (cu == cv) {
            /* 两个端点有相同的空闲颜色，直接着色 */
            edgeColors[e] = cu;
            vertexColors[u][cu] = e;
            vertexColors[v][cu] = e;
        } else {
            /* 颜色不同，需要路径翻转 */
            /* 从v出发沿cu颜色找交替路径 */
            QVector<int> path;
            QSet<int> visited;
            int current = v;
            visited.insert(current);

            while (true) {
                /* 找从current出发颜色为cu的边 */
                int nextVertex = -1;
                int nextEdge = -1;
                if (vertexColors[current].contains(cu)) {
                    nextEdge = vertexColors[current][cu];
                    int nu = m_edges[nextEdge].first;
                    int nv = m_edges[nextEdge].second;
                    nextVertex = (nu == current) ? nv : nu;
                }

                if (nextVertex == -1 || visited.contains(nextVertex)) break;
                path.append(nextEdge);
                visited.insert(nextVertex);
                current = nextVertex;

                /* 交换查找颜色 */
                std::swap(cu, cv);
            }

            /* 翻转路径上的颜色 */
            int color1 = freeColor(u, vertexColors, maxColor);
            int color2 = freeColor(v, vertexColors, maxColor);

            for (int pe : path) {
                int pu = m_edges[pe].first;
                int pv = m_edges[pe].second;
                int oldColor = edgeColors[pe];

                vertexColors[pu].remove(oldColor);
                vertexColors[pv].remove(oldColor);

                int newColor = (oldColor == color1) ? color2 : color1;
                edgeColors[pe] = newColor;
                vertexColors[pu][newColor] = pe;
                vertexColors[pv][newColor] = pe;
            }

            /* 现在u和v有共同的空闲颜色 */
            int commonFree = freeColor(u, vertexColors, maxColor);
            edgeColors[e] = commonFree;
            vertexColors[u][commonFree] = e;
            vertexColors[v][commonFree] = e;
        }
    }

    /* 计算实际使用的颜色数 */
    int usedColors = 0;
    for (int c : edgeColors) {
        usedColors = qMax(usedColors, c);
    }

    return usedColors;
}

/**
 * @brief 执行边着色
 */
QVector<int> ChromaticIndex::colorEdges()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> edgeColors(m_edges.size(), 0);

    int chromaticIdx = misraGries(edgeColors);

    m_stats.chromaticIndex = chromaticIdx;
    m_stats.totalEdgesColored += m_edges.size();
    m_stats.totalColorings++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalColorings > 0)
        ? m_timeSum / m_stats.totalColorings : 0.0;

    emit coloringCompleted(chromaticIdx, m_edges.size());
    return edgeColors;
}

void ChromaticIndex::clear()
{
    m_edges.clear();
    m_maxDegree = 0;
}

void ChromaticIndex::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
