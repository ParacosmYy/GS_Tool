/**
 * @file ConnectedComponents.cpp
 * @brief 连通分量标注引擎实现 — 二值图像连通域分析
 */

#include "utils/connected/ConnectedComponents.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
ConnectedComponents::ConnectedComponents(QObject* parent)
    : QObject(parent)
    , m_connectivity(Connectivity::Eight)
    , m_componentCount(0)
    , m_timeSum(0.0)
{
}

/** @brief 设置连通类型 @param conn 连通类型 */
void ConnectedComponents::setConnectivity(Connectivity conn)
{
    m_connectivity = conn;
}

/** @brief 标注连通分量 @param binaryImage 二值图像 @return 标签图像 */
QVector<QVector<int>> ConnectedComponents::label(
    const QVector<QVector<int>>& binaryImage)
{
    QElapsedTimer timer;
    timer.start();

    int rows = binaryImage.size();
    m_componentCount = 0;
    m_componentSizes.clear();

    if (rows == 0) {
        ++m_stats.totalLabeled;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLabeled;
        emit labelingCompleted(0);
        return {};
    }

    int cols = binaryImage[0].size();
    QVector<QVector<int>> labels(rows, QVector<int>(cols, 0));

    /* 并查集数据结构 */
    int maxLabels = rows * cols / 2 + 1;
    QVector<int> parent(maxLabels);
    QVector<int> rank(maxLabels, 0);
    for (int i = 0; i < maxLabels; ++i) parent[i] = i;

    int nextLabel = 1;

    /* 第一遍扫描: 分配临时标签并合并 */
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (binaryImage[y][x] == 0) continue;

            QVector<int> neighbors;

            /* 检查已访问的邻居 */
            if (y > 0 && labels[y - 1][x] > 0) {
                neighbors.append(labels[y - 1][x]);
            }
            if (x > 0 && labels[y][x - 1] > 0) {
                neighbors.append(labels[y][x - 1]);
            }

            if (m_connectivity == Connectivity::Eight) {
                if (y > 0 && x > 0 && labels[y - 1][x - 1] > 0) {
                    neighbors.append(labels[y - 1][x - 1]);
                }
                if (y > 0 && x < cols - 1 && labels[y - 1][x + 1] > 0) {
                    neighbors.append(labels[y - 1][x + 1]);
                }
            }

            if (neighbors.isEmpty()) {
                /* 新标签 */
                labels[y][x] = nextLabel++;
            } else {
                /* 使用最小邻居标签 */
                int minLabel = neighbors[0];
                for (int nb : neighbors) {
                    minLabel = qMin(minLabel, nb);
                }
                labels[y][x] = minLabel;

                /* 合并所有邻居 */
                for (int nb : neighbors) {
                    unionSets(minLabel, nb, parent, rank);
                }
            }
        }
    }

    /* 第二遍扫描: 解析最终标签 */
    QMap<int, int> labelMap;
    int finalLabel = 0;
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (labels[y][x] == 0) continue;
            int root = findRoot(labels[y][x], parent);
            if (!labelMap.contains(root)) {
                labelMap[root] = ++finalLabel;
            }
            labels[y][x] = labelMap[root];
        }
    }

    /* 计算各连通分量大小 */
    m_componentCount = finalLabel;
    m_componentSizes.resize(finalLabel);
    m_componentSizes.fill(0);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (labels[y][x] > 0 && labels[y][x] <= finalLabel) {
                ++m_componentSizes[labels[y][x] - 1];
            }
        }
    }

    /* 更新统计 */
    ++m_stats.totalLabeled;
    m_stats.totalComponents += static_cast<quint64>(m_componentCount);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLabeled;

    emit labelingCompleted(m_componentCount);
    return labels;
}

/** @brief 并查集查找(路径压缩) @param x 元素 @param parent 父数组 @return 根 */
int ConnectedComponents::findRoot(int x, QVector<int>& parent) const
{
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];  /* 路径压缩 */
        x = parent[x];
    }
    return x;
}

/** @brief 并查集合并(按秩合并) @param a 元素a @param b 元素b @param parent 父数组 @param rank 秩数组 */
void ConnectedComponents::unionSets(int a, int b, QVector<int>& parent,
                                     QVector<int>& rank)
{
    int ra = findRoot(a, parent);
    int rb = findRoot(b, parent);
    if (ra == rb) return;

    if (rank[ra] < rank[rb]) {
        parent[ra] = rb;
    } else if (rank[ra] > rank[rb]) {
        parent[rb] = ra;
    } else {
        parent[rb] = ra;
        ++rank[ra];
    }
}

/** @brief 重置统计 */
void ConnectedComponents::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
