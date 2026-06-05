/**
 * @file SubspaceCluster2.cpp
 * @brief 子空间聚类增强实现 — 网格密度/子空间搜索/CLIQUE风格
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster32/SubspaceCluster2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <set>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SubspaceCluster2::SubspaceCluster2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SubspaceCluster2"));
}

/**
 * @brief 设置网格分辨率（每维的分箱数）
 * @param bins 每维箱数（最小为2）
 */
void SubspaceCluster2::setGridResolution(int bins)
{
    m_bins = qMax(2, bins);
}

/**
 * @brief 设置密度阈值
 *
 * 一个网格单元被认为是"密集"的当其包含的点数 >= 总点数 * threshold。
 *
 * @param threshold 密度阈值（0~1之间）
 */
void SubspaceCluster2::setDensityThreshold(double threshold)
{
    m_threshold = qBound(0.01, threshold, 1.0);
}

/**
 * @brief 设置最小子空间维度数
 *
 * 只报告维度数 >= minDims 的密集子空间。
 *
 * @param minDims 最小维度数
 */
void SubspaceCluster2::setMinDimensions(int minDims)
{
    m_minDims = qMax(1, minDims);
}

/**
 * @brief 执行子空间聚类
 *
 * 步骤:
 * 1. 对每个维度独立划分网格，统计每个网格单元的点数
 * 2. 在全维度空间中标记密集网格单元
 * 3. 连通密集单元形成簇
 * 4. 未落入密集单元的点标记为离群(-1)
 *
 * @param data 输入数据矩阵 [nPoints x nDims]
 * @return 聚类标签向量
 */
QVector<int> SubspaceCluster2::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, -1);

    if (n == 0) {
        emit clusteringComplete(0, 0);
        return labels;
    }

    const int dims = data[0].size();
    if (dims < m_minDims) {
        emit clusteringComplete(0, 0);
        return labels;
    }

    /* 第1步: 计算每维的最小/最大值用于归一化 */
    QVector<double> minVal(dims, 1e18);
    QVector<double> maxVal(dims, -1e18);
    for (int i = 0; i < n; ++i) {
        for (int d = 0; d < dims; ++d) {
            minVal[d] = qMin(minVal[d], data[i][d]);
            maxVal[d] = qMax(maxVal[d], data[i][d]);
        }
    }
    /* 防止除零 */
    for (int d = 0; d < dims; ++d) {
        if (maxVal[d] - minVal[d] < 1e-12) {
            maxVal[d] = minVal[d] + 1.0;
        }
    }

    /* 第2步: 将每个点映射到网格单元 */
    auto gridIndex = [&](int pointIdx, int dim) -> int {
        double normalized = (data[pointIdx][dim] - minVal[dim]) / (maxVal[dim] - minVal[dim]);
        int idx = static_cast<int>(normalized * m_bins);
        return qBound(0, idx, m_bins - 1);
    };

    /* 计算每个点的网格键 (多维网格坐标压缩为单一索引) */
    QVector<quint64> gridKeys(n);
    for (int i = 0; i < n; ++i) {
        quint64 key = 0;
        for (int d = 0; d < dims; ++d) {
            key = key * m_bins + static_cast<quint64>(gridIndex(i, d));
        }
        gridKeys[i] = key;
    }

    /* 第3步: 统计每个网格单元的点数 */
    QMap<quint64, QVector<int>> cellPoints;
    for (int i = 0; i < n; ++i) {
        cellPoints[gridKeys[i]].append(i);
    }

    /* 第4步: 标记密集单元（点数 >= n * threshold） */
    int minCount = qMax(1, static_cast<int>(n * m_threshold));
    QMap<quint64, int> cellLabel;
    int clusterId = 0;

    /* 连通密集单元: 使用BFS */
    QSet<quint64> visited;
    for (auto it = cellPoints.begin(); it != cellPoints.end(); ++it) {
        quint64 cell = it.key();
        if (visited.contains(cell)) continue;
        if (it.value().size() < minCount) continue;

        /* BFS查找相邻密集单元 */
        QVector<quint64> component;
        QList<quint64> queue;
        queue.append(cell);
        visited.insert(cell);

        while (!queue.isEmpty()) {
            quint64 cur = queue.takeFirst();
            component.append(cur);

            /* 生成相邻单元（每维±1） */
            QVector<int> coords(dims);
            quint64 tmp = cur;
            for (int d = dims - 1; d >= 0; --d) {
                coords[d] = static_cast<int>(tmp % m_bins);
                tmp /= m_bins;
            }

            /* 遍历所有2*dims个邻居 */
            for (int d = 0; d < dims; ++d) {
                for (int delta = -1; delta <= 1; delta += 2) {
                    int newCoord = coords[d] + delta;
                    if (newCoord < 0 || newCoord >= m_bins) continue;

                    quint64 neighbor = 0;
                    for (int dd = 0; dd < dims; ++dd) {
                        int c = (dd == d) ? newCoord : coords[dd];
                        neighbor = neighbor * m_bins + c;
                    }

                    if (visited.contains(neighbor)) continue;
                    if (!cellPoints.contains(neighbor)) continue;
                    if (cellPoints[neighbor].size() < minCount) continue;

                    visited.insert(neighbor);
                    queue.append(neighbor);
                }
            }
        }

        /* 标记整个连通分量的点 */
        for (quint64 c : component) {
            cellLabel[c] = clusterId;
            for (int pt : cellPoints[c]) {
                labels[pt] = clusterId;
            }
        }
        clusterId++;
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringComplete(clusterId, 0);
    return labels;
}

/**
 * @brief 发现所有密集子空间
 *
 * 在每个维度的组合上检查是否存在密集网格单元，
 * 返回满足最小维度约束的子空间列表。
 *
 * @param data 输入数据矩阵
 * @return 子空间列表，每个子空间用维度索引向量表示
 */
QList<QVector<int>> SubspaceCluster2::findSubspaces(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<QVector<int>> result;

    const int n = data.size();
    if (n == 0) return result;
    const int dims = data[0].size();

    int minCount = qMax(1, static_cast<int>(n * m_threshold));

    /* 计算每维的范围 */
    QVector<double> minVal(dims, 1e18);
    QVector<double> maxVal(dims, -1e18);
    for (int i = 0; i < n; ++i) {
        for (int d = 0; d < dims; ++d) {
            minVal[d] = qMin(minVal[d], data[i][d]);
            maxVal[d] = qMax(maxVal[d], data[i][d]);
        }
    }
    for (int d = 0; d < dims; ++d) {
        if (maxVal[d] - minVal[d] < 1e-12) maxVal[d] = minVal[d] + 1.0;
    }

    /* 对每个子空间维度组合检查密度 */
    /* 生成维度数为 minDims ~ dims 的所有组合 */
    for (int numDims = m_minDims; numDims <= dims; ++numDims) {
        /* 使用位掩码枚举组合 */
        int upper = 1 << dims;
        for (int mask = 0; mask < upper; ++mask) {
            if (__builtin_popcount(mask) != numDims) continue;

            QVector<int> subDims;
            for (int d = 0; d < dims; ++d) {
                if (mask & (1 << d)) subDims.append(d);
            }

            /* 在子空间中统计网格密度 */
            QMap<quint64, int> cellCount;
            for (int i = 0; i < n; ++i) {
                quint64 key = 0;
                for (int d : subDims) {
                    double norm = (data[i][d] - minVal[d]) / (maxVal[d] - minVal[d]);
                    int idx = qBound(0, static_cast<int>(norm * m_bins), m_bins - 1);
                    key = key * m_bins + idx;
                }
                cellCount[key]++;
            }

            /* 检查是否有任何密集单元 */
            bool hasDense = false;
            for (auto it = cellCount.begin(); it != cellCount.end(); ++it) {
                if (it.value() >= minCount) {
                    hasDense = true;
                    break;
                }
            }

            if (hasDense) {
                result.append(subDims);
            }
        }
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterings++;

    emit clusteringComplete(-1, result.size());
    return result;
}

/**
 * @brief 重置所有累积统计信息
 */
void SubspaceCluster2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
