#include "SpectralClustering5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class SpectralClustering5
 * @brief 谱聚类算法实现
 *
 * 谱聚类通过图拉普拉斯矩阵的特征分解实现聚类:
 * 1. 构建亲和矩阵W(高斯核: W_ij = exp(-||xi-xj||^2 / 2σ^2))
 * 2. 计算归一化拉普拉斯矩阵 L = D^(-1/2)(D-W)D^(-1/2)
 * 3. 取L的前k个最小特征值对应的特征向量
 * 4. 在特征向量空间中执行K-Means聚类
 *
 * 优点: 能处理非凸形状的簇，不需要球形分布假设。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
SpectralClustering5::SpectralClustering5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行谱聚类
 *
 * 完整的谱聚类流程:
 * 1. 构建归一化拉普拉斯矩阵
 * 2. 幂迭代法求前k个特征向量
 * 3. 行归一化特征向量矩阵
 * 4. 简单K-Means分配簇标签
 *
 * @param affinity 亲和矩阵(n×n)，或直接传入数据矩阵
 * @param k 目标簇数
 * @return 每个数据点的簇标签向量(0到k-1)
 */
QVector<int> SpectralClustering5::fit(const QVector<QVector<double>>& affinity, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = affinity.size();
    QVector<int> labels(n, 0);

    if (n == 0 || k <= 0 || k > n) {
        m_timeSum += timer.elapsed();
        return labels;
    }

    if (k == 1) {
        m_timeSum += timer.elapsed();
        return labels;
    }

    /* 构建度矩阵D并计算D^(-1/2) */
    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += affinity[i][j];
        }
        dInvSqrt[i] = (sum > 1e-15) ? 1.0 / qSqrt(sum) : 0.0;
    }

    /* 归一化拉普拉斯矩阵 L_norm = I - D^(-1/2) W D^(-1/2) */
    /* 等价于用 D^(-1/2) W D^(-1/2) 的特征向量 (最大特征值对应) */
    /* 这里直接用 L_sym = D^(-1/2)(D-W)D^(-1/2) 的最小特征向量 */

    /* 幂迭代法求前k个特征向量 */
    QVector<QVector<double>> eigenvectors(k, QVector<double>(n, 0.0));

    for (int ev = 0; ev < k; ++ev) {
        /* 初始化随机向量 */
        QVector<double> v(n, 0.0);
        for (int i = 0; i < n; ++i) {
            v[i] = qSin(i * 127.1 + ev * 311.7);
        }

        /* 归一化 */
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += v[i] * v[i];
        norm = qSqrt(norm);
        for (int i = 0; i < n; ++i) v[i] /= (norm + 1e-15);

        /* 幂迭代 */
        for (int iter = 0; iter < 100; ++iter) {
            QVector<double> Lv(n, 0.0);

            /* 计算 L_norm * v = v - D^(-1/2) W D^(-1/2) v */
            for (int i = 0; i < n; ++i) {
                double sum = 0.0;
                for (int j = 0; j < n; ++j) {
                    sum += affinity[i][j] * dInvSqrt[i] * dInvSqrt[j] * v[j];
                }
                Lv[i] = v[i] - sum;
            }

            /* 减去已找到的特征向量分量(正交化) */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += Lv[i] * eigenvectors[prev][i];
                for (int i = 0; i < n; ++i) Lv[i] -= dot * eigenvectors[prev][i];
            }

            /* 归一化 */
            norm = 0.0;
            for (int i = 0; i < n; ++i) norm += Lv[i] * Lv[i];
            norm = qSqrt(norm);
            if (norm < 1e-15) break;
            for (int i = 0; i < n; ++i) v[i] = Lv[i] / norm;
        }

        eigenvectors[ev] = v;
    }

    m_stats.totalEigenDecomps++;

    /* 行归一化特征向量矩阵 */
    QVector<QVector<double>> normalized(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i) {
        double rowNorm = 0.0;
        for (int j = 0; j < k; ++j) {
            rowNorm += eigenvectors[j][i] * eigenvectors[j][i];
        }
        rowNorm = qSqrt(rowNorm);
        for (int j = 0; j < k; ++j) {
            normalized[i][j] = (rowNorm > 1e-15) ? eigenvectors[j][i] / rowNorm : 0.0;
        }
    }

    /* 简单K-Means分配 */
    QVector<QVector<double>> centroids(k, QVector<double>(k, 0.0));
    for (int c = 0; c < k; ++c) {
        int idx = c * n / k;
        centroids[c] = normalized[idx];
    }

    for (int iter = 0; iter < 20; ++iter) {
        /* 分配 */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            for (int c = 0; c < k; ++c) {
                double dist = 0.0;
                for (int d = 0; d < k; ++d) {
                    double diff = normalized[i][d] - centroids[c][d];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    labels[i] = c;
                }
            }
        }

        /* 更新质心 */
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(k, 0.0);
            int count = 0;
            for (int i = 0; i < n; ++i) {
                if (labels[i] == c) {
                    for (int d = 0; d < k; ++d) sum[d] += normalized[i][d];
                    count++;
                }
            }
            if (count > 0) {
                for (int d = 0; d < k; ++d) centroids[c][d] = sum[d] / count;
            }
        }
    }

    double eigengap = 0.0; /* 简化: 不计算精确eigen gap */
    m_stats.totalClusterings++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalClusterings);

    emit clusteringCompleted(k, eigengap);

    return labels;
}

/**
 * @brief 构建高斯核亲和矩阵
 *
 * 计算所有数据点对之间的高斯核相似度:
 * W_ij = exp(-||xi - xj||^2 / (2 * σ^2))
 *
 * @param data 数据矩阵(n×d)
 * @param sigma 高斯核带宽参数
 * @return 亲和矩阵(n×n)
 */
QVector<QVector<double>> SpectralClustering5::buildAffinity(
    const QVector<QVector<double>>& data, double sigma) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));

    if (n == 0 || data[0].isEmpty()) return W;

    double twoSigmaSq = 2.0 * sigma * sigma;

    for (int i = 0; i < n; ++i) {
        W[i][i] = 1.0; /* 对角线为1 */
        for (int j = i + 1; j < n; ++j) {
            double distSq = 0.0;
            for (int d = 0; d < data[i].size(); ++d) {
                double diff = data[i][d] - data[j][d];
                distSq += diff * diff;
            }
            double val = qExp(-distSq / twoSigmaSq);
            W[i][j] = val;
            W[j][i] = val;
        }
    }

    return W;
}

/**
 * @brief 重置所有统计数据
 *
 * 将聚类计数、特征分解计数和计时归零。
 */
void SpectralClustering5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
