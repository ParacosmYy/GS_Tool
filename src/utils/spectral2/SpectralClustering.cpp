/**
 * @file SpectralClustering.cpp
 * @brief 谱聚类算法实现
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/spectral2/SpectralClustering.h"

#include <QtGlobal>
#include <limits>
#include <random>

/**
 * @brief 构造函数
 * @param parent 父 QObject
 */
SpectralClustering::SpectralClustering(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 构建归一化拉普拉斯矩阵
 *
 * L_norm = I - D^{-1/2} W D^{-1/2}
 * 其中 D 为度矩阵(行和的对角矩阵)。
 *
 * @param affinity 亲和度矩阵 W
 * @param diag 输出 D^{-1/2} 对角元素
 * @return 归一化拉普拉斯矩阵
 */
QVector<QVector<double>> SpectralClustering::buildNormLaplacian(
    const QVector<QVector<double>> &affinity, QVector<double> &diag)
{
    const int n = affinity.size();
    diag.resize(n);

    // 计算度矩阵对角线
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += affinity[i][j];
        }
        // 防止除零
        diag[i] = (sum > 1e-15) ? 1.0 / std::sqrt(sum) : 0.0;
    }

    // L_norm[i][j] = -diag[i] * W[i][j] * diag[j]  (off-diagonal)
    // L_norm[i][i] = 1.0 - diag[i]^2 * W[i][i]
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                L[i][j] = 1.0 - diag[i] * diag[i] * affinity[i][j];
            } else {
                L[i][j] = -diag[i] * affinity[i][j] * diag[j];
            }
        }
    }
    return L;
}

/**
 * @brief 幂迭代法求前 k 个最小特征向量
 *
 * 对矩阵取反 (maxI - L) 后做幂迭代，等价于求 L 的最小特征向量。
 * 每次迭代后做 Gram-Schmidt 正交化。
 *
 * @param matrix 拉普拉斯矩阵
 * @param k 特征向量数
 * @param maxIter 最大迭代次数
 * @return k 个特征向量
 */
QVector<QVector<double>> SpectralClustering::powerEigenvectors(
    const QVector<QVector<double>> &matrix, int k, int maxIter)
{
    const int n = matrix.size();
    // 取反: 求 L 最小特征值 = 求 (maxI - L) 最大特征值
    double maxVal = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            maxVal = std::max(maxVal, std::abs(matrix[i][j]));
        }
    }
    double shift = 2.0 * maxVal; // 足够大的平移

    QVector<QVector<double>> vecs(k);
    std::mt19937 rng(42);

    for (int ev = 0; ev < k; ++ev) {
        // 随机初始化
        QVector<double> v(n, 0.0);
        for (int i = 0; i < n; ++i) {
            v[i] = static_cast<double>(rng()) / static_cast<double>(rng.max());
        }

        for (int iter = 0; iter < maxIter; ++iter) {
            // 矩阵-向量乘法: w = (shift*I - L) * v
            QVector<double> w(n, 0.0);
            for (int i = 0; i < n; ++i) {
                double sum = shift * v[i] - 0.0; // shift*I 部分
                for (int j = 0; j < n; ++j) {
                    sum -= matrix[i][j] * v[j];
                }
                // 减去已求特征向量的分量 (deflation)
                for (int prev = 0; prev < ev; ++prev) {
                    double dot = 0.0;
                    for (int jj = 0; jj < n; ++jj) {
                        dot += vecs[prev][jj] * v[jj];
                    }
                    sum -= dot * vecs[prev][i] * shift;
                }
                w[i] = sum;
            }

            // 归一化
            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += w[i] * w[i];
            }
            norm = std::sqrt(norm);
            if (norm < 1e-15) break;
            for (int i = 0; i < n; ++i) {
                v[i] = w[i] / norm;
            }
        }
        vecs[ev] = v;
    }
    return vecs;
}

/**
 * @brief 简单 K-Means 分配
 *
 * 在特征空间中执行 K-Means 聚类，返回簇标签。
 * 初始化采用随机选取 k 个样本作为中心。
 *
 * @param features n x k 特征矩阵
 * @param k 簇数
 * @param maxIter 最大迭代次数
 * @return 簇标签数组
 */
QVector<int> SpectralClustering::kmeansAssign(
    const QVector<QVector<double>> &features, int k, int maxIter)
{
    const int n = features.size();
    if (n == 0) return {};
    const int dim = features[0].size();

    // 随机初始化中心
    std::mt19937 rng(123);
    QVector<QVector<double>> centers(k);
    std::vector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng);

    for (int c = 0; c < k && c < n; ++c) {
        centers[c] = features[indices[c]];
    }
    // 若 k > n，填充零
    for (int c = n; c < k; ++c) {
        centers[c].resize(dim, 0.0);
    }

    QVector<int> labels(n, 0);

    for (int iter = 0; iter < maxIter; ++iter) {
        bool changed = false;

        // 分配步骤: 每个样本到最近中心
        for (int i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestLabel = 0;
            for (int c = 0; c < k; ++c) {
                double dist = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = features[i][d] - centers[c][d];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestLabel = c;
                }
            }
            if (labels[i] != bestLabel) {
                labels[i] = bestLabel;
                changed = true;
            }
        }

        if (!changed) break;

        // 更新步骤: 重新计算中心
        QVector<QVector<double>> newCenters(k, QVector<double>(dim, 0.0));
        QVector<int> counts(k, 0);
        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            counts[c]++;
            for (int d = 0; d < dim; ++d) {
                newCenters[c][d] += features[i][d];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < dim; ++d) {
                    newCenters[c][d] /= static_cast<double>(counts[c]);
                }
                centers[c] = newCenters[c];
            }
        }
    }

    return labels;
}

/**
 * @brief 执行谱聚类
 *
 * 流程:
 * 1. 验证亲和度矩阵(方阵、对称、k <= n)
 * 2. 构建归一化拉普拉斯
 * 3. 求前 k 个特征向量
 * 4. 对特征向量行做 K-Means
 * 5. 更新统计并发射信号
 *
 * @param affinity n x n 亲和度矩阵
 * @param k 目标簇数
 * @return 簇标签数组
 */
QVector<int> SpectralClustering::cluster(
    const QVector<QVector<double>> &affinity, int k)
{
    m_timer.start();

    const int n = affinity.size();

    // 输入校验
    if (n == 0 || k < 2 || k > n) {
        emit clusteringCompleted(k, 0);
        return {};
    }
    for (int i = 0; i < n; ++i) {
        if (affinity[i].size() != n) {
            emit clusteringCompleted(k, 0);
            return {};
        }
    }

    // 1. 构建归一化拉普拉斯
    QVector<double> diag;
    QVector<QVector<double>> L = buildNormLaplacian(affinity, diag);

    // 2. 求前 k 个最小特征向量
    QVector<QVector<double>> eigvecs = powerEigenvectors(L, k);

    // 3. 组装特征矩阵: 每行 = 样本在 k 个特征向量上的投影
    QVector<QVector<double>> features(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            features[i][j] = eigvecs[j][i];
        }
        // 行归一化
        double norm = 0.0;
        for (int j = 0; j < k; ++j) {
            norm += features[i][j] * features[i][j];
        }
        norm = std::sqrt(norm);
        if (norm > 1e-15) {
            for (int j = 0; j < k; ++j) {
                features[i][j] /= norm;
            }
        }
    }

    // 4. K-Means 分配
    QVector<int> labels = kmeansAssign(features, k);

    // 更新统计
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_stats.totalClusterings++;
    m_timeAccum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeAccum / static_cast<double>(m_stats.totalClusterings);

    emit clusteringCompleted(k, n);
    return labels;
}

/**
 * @brief 重置统计计数器
 */
void SpectralClustering::resetStatistics()
{
    m_stats.totalClusterings = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeAccum = 0.0;
}
