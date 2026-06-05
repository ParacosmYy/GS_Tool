#include "DBSCAN16.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- 内部状态存储 ---- */
static int g_noiseCount = 0;

/**
 * @brief 构造函数，初始化DBSCAN聚类引擎
 * @param parent 父对象指针
 */
DBSCAN16::DBSCAN16(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DBSCAN16::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    g_noiseCount = 0;
}

/**
 * @brief 执行DBSCAN聚类
 *
 * 经典密度聚类流程：
 * 1. 对每个未访问点执行邻域查询
 * 2. 若邻域点数 >= minPoints 则标记为核心点，创建新簇
 * 3. 递归扩展核心点的邻域，将密度可达的点归入同一簇
 * 4. 不属于任何簇的点标记为噪声（标签-1）
 *
 * @param dataPoints 输入数据点集合
 * @param epsilon 邻域半径
 * @param minPoints 核心点最小邻居数
 * @return 各点聚类标签（-1表示噪声）
 */
QVector<int> DBSCAN16::fit(const QVector<QVector<double>>& dataPoints,
                            double epsilon, int minPoints)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    QVector<int> labels(n, -1);
    g_noiseCount = 0;

    if (n == 0) {
        emit clusteringCompleted(0);
        return labels;
    }

    /* 访问标记：0=未访问, 1=已访问 */
    QVector<int> visited(n, 0);
    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (visited[i]) continue;
        visited[i] = 1;

        /* 查询epsilon邻域 */
        QVector<int> neighbors = regionQuery(i, dataPoints, epsilon);

        if (neighbors.size() < minPoints) {
            /* 标记为噪声（后续可能被其他簇吸收） */
            labels[i] = -1;
        } else {
            /* 创建新簇并扩展 */
            labels[i] = clusterId;
            /* 使用队列进行广度优先扩展 */
            QVector<int> seeds = neighbors;
            int idx = 0;
            while (idx < seeds.size()) {
                int q = seeds[idx++];
                if (q < 0 || q >= n) continue;

                if (!visited[q]) {
                    visited[q] = 1;
                    QVector<int> qNeighbors = regionQuery(q, dataPoints, epsilon);
                    if (qNeighbors.size() >= minPoints) {
                        /* 将新邻域点加入待扩展列表 */
                        for (int nn : qNeighbors) {
                            if (!seeds.contains(nn)) {
                                seeds.append(nn);
                            }
                        }
                    }
                }

                /* 若q尚未归属任何簇，则归入当前簇 */
                if (labels[q] < 0) {
                    labels[q] = clusterId;
                }
            }
            clusterId++;
        }
    }

    /* 统计噪声点数量 */
    g_noiseCount = 0;
    for (int i = 0; i < n; ++i) {
        if (labels[i] < 0) g_noiseCount++;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterOps;

    emit clusteringCompleted(clusterId);
    return labels;
}

/**
 * @brief 使用k-distance图自动估计epsilon参数
 *
 * 对每个点计算到第k近邻的距离，排序后绘制k-distance图，
 * 在拐点处（最大曲率）选取epsilon值。
 * 拐点检测：找到排序后距离序列中斜率变化最大的位置。
 *
 * @param dataPoints 数据点集合
 * @param k k近邻数（通常等于minPoints）
 * @return 建议的epsilon值
 */
double DBSCAN16::estimateEpsilon(const QVector<QVector<double>>& dataPoints, int k)
{
    const int n = dataPoints.size();
    if (n == 0) return 0.0;
    k = qMax(1, qMin(k, n - 1));

    /* 计算每个点的k-distance */
    QVector<double> kDistances;
    for (int i = 0; i < n; ++i) {
        /* 计算到所有其他点的距离 */
        QVector<double> dists;
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double dist = 0.0;
            const int dim = qMin(dataPoints[i].size(), dataPoints[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = dataPoints[i][d] - dataPoints[j][d];
                dist += diff * diff;
            }
            dists.append(qSqrt(dist));
        }
        std::sort(dists.begin(), dists.end());
        if (k - 1 < dists.size()) {
            kDistances.append(dists[k - 1]);
        }
    }

    if (kDistances.isEmpty()) return 0.0;

    /* 排序后寻找拐点（最大二阶差分） */
    std::sort(kDistances.begin(), kDistances.end());

    double maxCurvature = 0.0;
    double bestEpsilon = kDistances.last();

    for (int i = 1; i < kDistances.size() - 1; ++i) {
        double d1 = kDistances[i] - kDistances[i - 1];
        double d2 = kDistances[i + 1] - kDistances[i];
        double curvature = qAbs(d2 - d1);
        if (curvature > maxCurvature) {
            maxCurvature = curvature;
            bestEpsilon = kDistances[i];
        }
    }

    return bestEpsilon;
}

/**
 * @brief 查询指定点的epsilon邻域内所有点
 *
 * 计算中心点到所有数据点的欧氏距离，
 * 返回距离小于epsilon的点索引集合。
 *
 * @param pointIndex 中心点索引
 * @param dataPoints 数据点集合
 * @param epsilon 邻域半径
 * @return 邻域内的点索引集合
 */
QVector<int> DBSCAN16::regionQuery(int pointIndex,
                                    const QVector<QVector<double>>& dataPoints,
                                    double epsilon)
{
    QVector<int> neighbors;
    const int n = dataPoints.size();
    if (pointIndex < 0 || pointIndex >= n) return neighbors;

    const auto& center = dataPoints[pointIndex];
    const int dim = center.size();
    const double eps2 = epsilon * epsilon;

    for (int i = 0; i < n; ++i) {
        if (i == pointIndex) continue;
        const int dMin = qMin(dim, dataPoints[i].size());
        double dist2 = 0.0;
        for (int d = 0; d < dMin; ++d) {
            double diff = center[d] - dataPoints[i][d];
            dist2 += diff * diff;
        }
        if (dist2 <= eps2) {
            neighbors.append(i);
        }
    }
    return neighbors;
}

/**
 * @brief 获取聚类结果中的噪声点数量
 * @return 噪声点数量
 */
int DBSCAN16::noiseCount() const
{
    return g_noiseCount;
}
