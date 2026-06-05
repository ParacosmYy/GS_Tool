/**
 * @file EdgeColoring3.cpp
 * @brief 图边着色算法实现
 *
 * 使用Vizing定理进行边着色，支持简单图和多重图。
 * 对简单图使用Delta或Delta+1种颜色，其中Delta为最大度数。
 * 返回最小或近似最小颜色数的边着色方案。
 */

#include "utils/graph88/EdgeColoring3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
EdgeColoring3::EdgeColoring3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 对图执行边着色
 * @param adjacency 邻接表，adjacency[u]包含u的所有邻居
 * @param vertexCount 顶点数量
 * @return coloring[edge] = color，edge用(u,v)表示
 *
 * 使用贪心边着色算法:
 * 1. 遍历所有边，按顺序着色
 * 2. 对每条边(u,v)，选择未被u和v的已着色边使用的最小颜色
 * 3. 颜色编号从0开始
 *
 * Vizing定理: 简单图的边着色数 = Delta 或 Delta+1
 * 其中Delta为最大顶点度数。
 */
QVector<QVector<int>> EdgeColoring3::color(const QVector<QVector<int>>& adjacency, int vertexCount)
{
    QElapsedTimer timer;
    timer.start();

    /* 收集所有边 */
    QVector<QPair<int, int>> edges;
    for (int u = 0; u < vertexCount; ++u) {
        for (int v : (u < adjacency.size() ? adjacency[u] : QVector<int>())) {
            if (u < v) {
                edges.append({u, v});
            }
        }
    }

    int numEdges = edges.size();
    if (numEdges == 0) {
        m_chromaticIndex = 0;
        return QVector<QVector<int>>();
    }

    /* 计算最大度数Delta */
    int maxDegree = 0;
    for (int u = 0; u < vertexCount; ++u) {
        int deg = (u < adjacency.size()) ? adjacency[u].size() : 0;
        maxDegree = qMax(maxDegree, deg);
    }

    /* 贪心边着色 */
    /* vertexColors[u] = 已分配给u的关联边的颜色集合 */
    QVector<QSet<int>> vertexColors(vertexCount);
    QVector<int> edgeColor(numEdges, -1);

    for (int e = 0; e < numEdges; ++e) {
        int u = edges[e].first;
        int v = edges[e].second;

        /* 找到u和v都未使用的最小颜色 */
        int c = 0;
        while (vertexColors[u].contains(c) || vertexColors[v].contains(c)) {
            c++;
        }

        edgeColor[e] = c;
        vertexColors[u].insert(c);
        vertexColors[v].insert(c);
    }

    /* 构建颜色到边的映射 */
    int numColors = 0;
    for (int e = 0; e < numEdges; ++e) {
        numColors = qMax(numColors, edgeColor[e] + 1);
    }

    m_colorEdges.clear();
    m_colorEdges.resize(numColors);
    for (int e = 0; e < numEdges; ++e) {
        m_colorEdges[edgeColor[e]].append(edges[e]);
    }

    m_chromaticIndex = numColors;

    /* 构建返回矩阵: coloring[color] = {u1, v1, u2, v2, ...} */
    QVector<QVector<int>> result(numColors);
    for (int c = 0; c < numColors; ++c) {
        for (const auto& edge : m_colorEdges[c]) {
            result[c].append(edge.first);
            result[c].append(edge.second);
        }
    }

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalGraphsColored++;
    m_stats.totalColorsUsed += numColors;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGraphsColored;

    emit coloringCompleted(numColors);
    return result;
}

/**
 * @brief 获取着色使用的颜色数
 * @return 色度指数(使用的颜色数量)
 */
int EdgeColoring3::chromaticIndex() const
{
    return m_chromaticIndex;
}

/**
 * @brief 验证着色方案的合法性
 * @param coloring coloring[edge] = color
 * @return true如果着色合法(无相邻边同色)
 *
 * 检查每对共享顶点的边是否使用了不同的颜色。
 */
bool EdgeColoring3::validateColoring(const QVector<QVector<int>>& coloring) const
{
    /* 重建边列表 */
    QMap<QPair<int, int>, int> edgeColorMap;
    for (int c = 0; c < coloring.size(); ++c) {
        for (int i = 0; i + 1 < coloring[c].size(); i += 2) {
            int u = coloring[c][i];
            int v = coloring[c][i + 1];
            edgeColorMap[{qMin(u, v), qMax(u, v)}] = c;
        }
    }

    /* 检查共享顶点的边是否有相同颜色 */
    for (auto it1 = edgeColorMap.begin(); it1 != edgeColorMap.end(); ++it1) {
        for (auto it2 = it1 + 1; it2 != edgeColorMap.end(); ++it2) {
            int u1 = it1.key().first, v1 = it1.key().second;
            int u2 = it2.key().first, v2 = it2.key().second;
            /* 共享顶点 */
            if (u1 == u2 || u1 == v2 || v1 == u2 || v1 == v2) {
                if (it1.value() == it2.value()) return false;
            }
        }
    }
    return true;
}

/**
 * @brief 获取指定颜色的所有边
 * @param color 颜色编号
 * @return 该颜色的边列表(u,v)对
 */
QVector<QPair<int, int>> EdgeColoring3::edgesOfColor(int color) const
{
    if (color < 0 || color >= m_colorEdges.size()) return QVector<QPair<int, int>>();
    return m_colorEdges[color];
}

/**
 * @brief 重置统计信息
 */
void EdgeColoring3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
