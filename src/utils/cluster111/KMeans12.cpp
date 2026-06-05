#include "KMeans12.h"
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化K-Means聚类引擎
 * @param parent 父对象指针
 */
KMeans12::KMeans12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void KMeans12::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算两点之间的欧氏距离平方
 * @param a 第一个点的特征向量
 * @param b 第二个点的特征向量
 * @return 欧氏距离平方
 */
static double distSq(const QVector<double>& a, const QVector<double>& b)
{
    const int dims = qMin(a.size(), b.size());
    double sum = 0.0;
    for (int d = 0; d < dims; ++d) {
        double diff = a[d] - b[d];
        sum += diff * diff;
    }
    return sum;
}

/**
 * @brief 使用K-Means++策略初始化聚类中心
 *
 * 第一个中心随机选取，后续中心按距离加权概率采样，
 * 保证初始中心分布均匀，加速收敛。
 *
 * @param dataPoints 输入数据点集合
 * @param k 聚类中心数量
 * @return 初始化后的聚类中心
 */
QVector<QVector<double>> KMeans12::initCentroidsKMeansPlusPlus(
    const QVector<QVector<double>>& dataPoints, int k)
{
    const int n = dataPoints.size();
    QVector<QVector<double>> centroids;

    if (n < 1 || k < 1) return centroids;
    k = qMin(k, n);

    /* 第一个中心：选取第一个数据点 */
    centroids.append(dataPoints[0]);

    QVector<double> minDists(n, 1e18);

    for (int c = 1; c < k; ++c) {
        /* 更新所有点到最近中心的距离 */
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = distSq(dataPoints[i], centroids.last());
            minDists[i] = qMin(minDists[i], d);
            totalDist += minDists[i];
        }

        /* 按距离加权概率选取下一个中心 */
        if (totalDist < 1e-12) {
            centroids.append(dataPoints[c % n]);
            continue;
        }

        double r = QRandomGenerator::global()->generateDouble() * totalDist;
        double cum = 0.0;
        int chosen = c % n;
        for (int i = 0; i < n; ++i) {
            cum += minDists[i];
            if (cum >= r) { chosen = i; break; }
        }
        centroids.append(dataPoints[chosen]);
    }
    return centroids;
}

/**
 * @brief 执行K-Means聚类（标准Lloyd's算法）
 *
 * 迭代流程：
 * 1. 使用K-Means++初始化聚类中心
 * 2. 分配步骤：将每个点分配到最近的中心
 * 3. 更新步骤：重新计算每个簇的中心
 * 4. 重复直到收敛或达到最大迭代次数
 *
 * @param dataPoints 输入数据点集合
 * @param k 聚类中心数量
 * @param maxIterations 最大迭代次数
 * @return 各数据点所属聚类标签
 */
QVector<int> KMeans12::fit(const QVector<QVector<double>>& dataPoints,
                            int k, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    QVector<int> labels(n, 0);

    if (n < 1 || k < 1) {
        emit clusterCompleted(0);
        return labels;
    }
    k = qMin(k, n);
    const int dim = dataPoints[0].size();

    /* K-Means++初始化 */
    auto centers = initCentroidsKMeansPlusPlus(dataPoints, k);
    if (centers.size() < k) {
        emit clusterCompleted(0);
        return labels;
    }

    int iterCount = 0;

    for (int iter = 0; iter < maxIterations; ++iter) {
        bool changed = false;

        /* 分配步骤：将每个点分配到最近中心 */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestCluster = 0;
            for (int c = 0; c < k; ++c) {
                double d = distSq(dataPoints[i], centers[c]);
                if (d < bestDist) {
                    bestDist = d;
                    bestCluster = c;
                }
            }
            if (labels[i] != bestCluster) {
                labels[i] = bestCluster;
                changed = true;
            }
        }

        if (!changed) break;
        iterCount = iter + 1;

        /* 更新步骤：重新计算每个簇的中心 */
        QVector<QVector<double>> newCenters(k, QVector<double>(dim, 0.0));
        QVector<int> counts(k, 0);
        for (int i = 0; i < n; ++i) {
            counts[labels[i]]++;
            for (int d = 0; d < dim; ++d) {
                newCenters[labels[i]][d] += dataPoints[i][d];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < dim; ++d) {
                    newCenters[c][d] /= counts[c];
                }
                centers[c] = newCenters[c];
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterOps;

    emit clusterCompleted(iterCount);
    return labels;
}

/**
 * @brief 计算轮廓系数评估聚类质量
 *
 * 轮廓系数衡量样本与其所属簇的紧密度和与其他簇的分离度。
 * s(i) = (b(i) - a(i)) / max(a(i), b(i))
 * 值域[-1, 1]，越大表示聚类效果越好。
 *
 * @param dataPoints 数据点集合
 * @param labels 聚类标签
 * @return 平均轮廓系数
 */
double KMeans12::computeSilhouette(const QVector<QVector<double>>& dataPoints,
                                    const QVector<int>& labels)
{
    const int n = dataPoints.size();
    if (n < 2 || n != labels.size()) return 0.0;

    int maxLabel = 0;
    for (int l : labels) maxLabel = qMax(maxLabel, l);
    if (maxLabel < 1) return 0.0;

    double totalSil = 0.0;
    for (int i = 0; i < n; ++i) {
        int myCluster = labels[i];

        /* 计算a(i)：同簇内平均距离 */
        double aSum = 0.0;
        int aCount = 0;
        for (int j = 0; j < n; ++j) {
            if (j != i && labels[j] == myCluster) {
                aSum += qSqrt(distSq(dataPoints[i], dataPoints[j]));
                ++aCount;
            }
        }
        double a = (aCount > 0) ? aSum / aCount : 0.0;

        /* 计算b(i)：最近其他簇的平均距离 */
        double b = 1e18;
        for (int c = 0; c <= maxLabel; ++c) {
            if (c == myCluster) continue;
            double cSum = 0.0;
            int cCount = 0;
            for (int j = 0; j < n; ++j) {
                if (labels[j] == c) {
                    cSum += qSqrt(distSq(dataPoints[i], dataPoints[j]));
                    ++cCount;
                }
            }
            if (cCount > 0) b = qMin(b, cSum / cCount);
        }
        if (b > 1e17) b = 0.0;

        double denom = qMax(a, b);
        totalSil += (denom > 0.0) ? (b - a) / denom : 0.0;
    }

    return totalSil / n;
}

/**
 * @brief Mini-Batch K-Means变体，适用于大规模数据
 *
 * 每次迭代随机选取一个小批次，对批次中的点分配到最近中心，
 * 使用流式均值更新：center = (1 - 1/counts[c]) * center + (1/counts[c]) * point
 *
 * @param dataPoints 输入数据点集合
 * @param k 聚类中心数量
 * @param batchSize 每批次样本数
 * @return 各数据点所属聚类标签
 */
QVector<int> KMeans12::fitMiniBatch(const QVector<QVector<double>>& dataPoints,
                                     int k, int batchSize)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    QVector<int> labels(n, 0);

    if (n < 1 || k < 1) {
        emit clusterCompleted(0);
        return labels;
    }
    k = qMin(k, n);
    batchSize = qMin(batchSize, n);
    const int dim = dataPoints[0].size();

    auto centers = initCentroidsKMeansPlusPlus(dataPoints, k);
    if (centers.size() < k) {
        emit clusterCompleted(0);
        return labels;
    }

    /* 每个中心的累计更新次数 */
    QVector<int> counts(k, 0);

    /* Mini-Batch迭代 */
    const int maxIter = 100;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 随机选取一个小批次 */
        QVector<int> batch;
        batch.reserve(batchSize);
        for (int b = 0; b < batchSize; ++b) {
            batch.append(QRandomGenerator::global()->bounded(n));
        }

        /* 对批次中的每个点，找到最近中心并流式更新 */
        for (int idx : batch) {
            double bestDist = 1e18;
            int bestCluster = 0;
            for (int c = 0; c < k; ++c) {
                double d = distSq(dataPoints[idx], centers[c]);
                if (d < bestDist) {
                    bestDist = d;
                    bestCluster = c;
                }
            }
            counts[bestCluster]++;
            double eta = 1.0 / counts[bestCluster];
            for (int d = 0; d < dim; ++d) {
                centers[bestCluster][d] = (1.0 - eta) * centers[bestCluster][d]
                                          + eta * dataPoints[idx][d];
            }
        }
    }

    /* 最终分配：将所有点分配到最终中心 */
    for (int i = 0; i < n; ++i) {
        double bestDist = 1e18;
        int bestCluster = 0;
        for (int c = 0; c < k; ++c) {
            double d = distSq(dataPoints[i], centers[c]);
            if (d < bestDist) {
                bestDist = d;
                bestCluster = c;
            }
        }
        labels[i] = bestCluster;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterOps;

    emit clusterCompleted(maxIter);
    return labels;
}
