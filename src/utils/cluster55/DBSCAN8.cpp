/**
 * @file DBSCAN8.cpp
 * @brief DBSCAN密度聚类实现 — 多距离度量密度聚类
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 DBSCAN（Density-Based Spatial Clustering of Applications with Noise）算法。
 * 支持多种距离度量（欧氏、曼哈顿、切比雪夫、余弦），
 * 根据密度可达性自动发现任意形状的聚类并标记噪声点。
 */

#include "utils/cluster55/DBSCAN8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 DBSCAN 参数
 * @param parent 父QObject对象
 */
DBSCAN8::DBSCAN8(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DBSCAN8"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置邻域半径 epsilon
 *
 * epsilon 决定了两个点被认为是"邻居"的最大距离。
 * 值越大，聚类越少越大；值越小，聚类越多越小。
 *
 * @param eps 邻域半径，必须 > 0
 */
void DBSCAN8::setEpsilon(double eps)
{
    m_eps = qMax(1e-6, eps);
}

/**
 * @brief 设置核心点的最小邻居数
 *
 * 一个点如果在其 epsilon 邻域内至少有 minPts 个点，
 * 则被认为是核心点。
 *
 * @param minPts 最小邻居数，必须 >= 1
 */
void DBSCAN8::setMinPoints(int minPts)
{
    m_minPts = qMax(1, minPts);
}

/**
 * @brief 设置距离度量类型
 *
 * 支持的距离度量：
 * - "euclidean": 欧氏距离（默认）
 * - "manhattan": 曼哈顿距离
 * - "chebyshev": 切比雪夫距离
 * - "cosine": 余弦距离
 *
 * @param metric 距离度量名称
 */
void DBSCAN8::setDistanceMetric(const QString& metric)
{
    m_metric = metric.toLower();
}

// ──────────────────────────────────────────────
// 核心聚类接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入数据执行 DBSCAN 聚类
 *
 * 算法流程：
 * 1. 对每个未访问点，查询其 epsilon 邻域
 * 2. 如果邻域内点数 >= minPts，创建新聚类并扩展
 * 3. 如果邻域内点数 < minPts，标记为噪声（可能后续被重新分配）
 * 4. 核心点的邻域内的点递归扩展到同一聚类
 *
 * @param points 输入数据点集合
 * @return 聚类标签数组，-1 表示噪声点，0-based 表示聚类编号
 */
QVector<int> DBSCAN8::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    if (n == 0) {
        m_numClusters = 0;
        m_noiseCount = 0;
        return {};
    }

    // 初始化标签：-2 表示未访问
    QVector<int> labels(n, -2);
    m_numClusters = 0;
    m_noiseCount = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] != -2) continue; // 已处理

        // 查询 epsilon 邻域
        QVector<int> neighbors = regionQuery(points, i);

        if (neighbors.size() < m_minPts) {
            // 标记为噪声（暂时）
            labels[i] = -1;
            m_noiseCount++;
        } else {
            // 创建新聚类
            int clusterId = m_numClusters;
            m_numClusters++;
            labels[i] = clusterId;

            // 扩展聚类：处理所有邻域点
            QVector<int> seedSet = neighbors;
            int seedIdx = 0;

            while (seedIdx < seedSet.size()) {
                int q = seedSet[seedIdx];
                seedIdx++;

                if (labels[q] == -1) {
                    // 之前标记为噪声，现在归入聚类
                    labels[q] = clusterId;
                    m_noiseCount--;
                }

                if (labels[q] != -2) continue; // 已处理

                labels[q] = clusterId;

                // 查询 q 的邻域
                QVector<int> qNeighbors = regionQuery(points, q);

                if (qNeighbors.size() >= m_minPts) {
                    // q 是核心点，将其邻域加入种子集
                    for (int nb : qNeighbors) {
                        if (labels[nb] == -2 || labels[nb] == -1) {
                            // 检查是否已在种子集中（简化：允许重复）
                            seedSet.append(nb);
                        }
                    }
                }
            }
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_numClusters, m_noiseCount);
    return labels;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含聚类次数、总点数和平均耗时的Stats结构
 */
DBSCAN8::Stats DBSCAN8::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void DBSCAN8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 邻域查询
// ──────────────────────────────────────────────

/**
 * @brief 查询指定点的 epsilon 邻域
 *
 * 遍历所有数据点，计算距离并收集在 epsilon 范围内的点。
 *
 * @param pts 所有数据点
 * @param idx 目标点索引
 * @return 邻域内点的索引列表（包括自身）
 */
QVector<int> DBSCAN8::regionQuery(const QVector<QVector<double>>& pts, int idx)
{
    QVector<int> neighbors;
    const int n = pts.size();

    for (int i = 0; i < n; ++i) {
        double dist = distance(pts[idx], pts[i]);
        if (dist <= m_eps) {
            neighbors.append(i);
        }
    }

    return neighbors;
}

// ──────────────────────────────────────────────
// 私有方法 — 距离计算
// ──────────────────────────────────────────────

/**
 * @brief 根据当前度量类型计算两个点之间的距离
 *
 * 支持的度量：
 * - euclidean: sqrt(sum((a_i - b_i)^2))
 * - manhattan: sum(|a_i - b_i|)
 * - chebyshev: max(|a_i - b_i|)
 * - cosine: 1 - (a.b / (||a|| * ||b||))
 *
 * @param a 第一个点
 * @param b 第二个点
 * @return 距离值
 */
double DBSCAN8::distance(const QVector<double>& a, const QVector<double>& b) const
{
    const int dim = qMin(a.size(), b.size());
    if (dim == 0) return 0.0;

    if (m_metric == QStringLiteral("manhattan")) {
        double sum = 0.0;
        for (int i = 0; i < dim; ++i) {
            sum += qAbs(a[i] - b[i]);
        }
        return sum;
    }

    if (m_metric == QStringLiteral("chebyshev")) {
        double maxVal = 0.0;
        for (int i = 0; i < dim; ++i) {
            maxVal = qMax(maxVal, qAbs(a[i] - b[i]));
        }
        return maxVal;
    }

    if (m_metric == QStringLiteral("cosine")) {
        double dotAB = 0.0, normA = 0.0, normB = 0.0;
        for (int i = 0; i < dim; ++i) {
            dotAB += a[i] * b[i];
            normA += a[i] * a[i];
            normB += b[i] * b[i];
        }
        double denom = qSqrt(normA) * qSqrt(normB);
        if (denom < 1e-15) return 1.0;
        return 1.0 - dotAB / denom;
    }

    // 默认欧氏距离
    double sumSq = 0.0;
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sumSq += diff * diff;
    }
    return qSqrt(sumSq);
}
