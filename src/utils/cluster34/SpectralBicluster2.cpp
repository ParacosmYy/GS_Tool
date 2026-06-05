/**
 * @file SpectralBicluster2.cpp
 * @brief 谱共聚类实现 - 基于SVD分解与Fiedler向量实现行列联合聚类
 *
 * 对输入矩阵进行归一化处理后，通过SVD提取前k/l个奇异向量，
 * 再使用KMeans对行和列分别聚类，最终返回行列标签。
 */

#include "utils/cluster34/SpectralBicluster2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SpectralBicluster2::SpectralBicluster2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置行聚类数
 * @param k 行方向的聚类数量，必须 >= 2
 */
void SpectralBicluster2::setNumRowClusters(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 设置列聚类数
 * @param l 列方向的聚类数量，必须 >= 2
 */
void SpectralBicluster2::setNumColClusters(int l)
{
    m_l = qMax(2, l);
}

/**
 * @brief 对输入矩阵执行谱共聚类
 *
 * 步骤:
 * 1. 对矩阵做行归一化(每行除以其和)和列归一化
 * 2. 计算归一化矩阵的SVD
 * 3. 取前k个左奇异向量作为行特征，前l个右奇异向量作为列特征
 * 4. 对特征向量做KMeans聚类
 *
 * @param matrix 输入数据矩阵 (rows x cols)
 * @return QPair<行聚类标签, 列聚类标签>
 */
QPair<QVector<int>, QVector<int>> SpectralBicluster2::fit(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    const int rows = matrix.size();
    const int cols = (rows > 0) ? matrix[0].size() : 0;

    QVector<int> rowLabels(rows, 0);
    QVector<int> colLabels(cols, 0);

    if (rows < 2 || cols < 2) {
        m_stats.totalClusterings++;
        m_stats.totalElementsProcessed += rows * cols;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
        return {rowLabels, colLabels};
    }

    /* 第一步: 构建归一化矩阵 */
    QVector<QVector<double>> norm(rows, QVector<double>(cols, 0.0));

    /* 计算行和与列和 */
    QVector<double> rowSums(rows, 0.0);
    QVector<double> colSums(cols, 0.0);
    double totalSum = 0.0;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            rowSums[i] += matrix[i][j];
            colSums[j] += matrix[i][j];
            totalSum += matrix[i][j];
        }
    }

    /* 归一化: A_ij / sqrt(rowSum_i * colSum_j) */
    if (totalSum > 0.0) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                double denom = qSqrt(qMax(rowSums[i], 1e-12) * qMax(colSums[j], 1e-12));
                norm[i][j] = matrix[i][j] / denom;
            }
        }
    }

    /* 第二步: 幂迭代法计算前k个左奇异向量 (简化SVD) */
    auto powerIteration = [&](QVector<QVector<double>>& A, int rank, bool left)
        -> QVector<QVector<double>> {
        int m = left ? rows : cols;
        int n = left ? cols : rows;
        QVector<QVector<double>> basis(rank, QVector<double>(m, 0.0));

        std::mt19937 rng(42);
        for (int r = 0; r < rank; ++r) {
            /* 随机初始化向量 */
            QVector<double> v(m, 0.0);
            for (int i = 0; i < m; ++i)
                v[i] = (rng() % 10000) / 10000.0 - 0.5;

            /* 正交化到已有基 */
            for (int p = 0; p < r; ++p) {
                double dot = 0.0;
                for (int i = 0; i < m; ++i) dot += v[i] * basis[p][i];
                for (int i = 0; i < m; ++i) v[i] -= dot * basis[p][i];
            }

            /* 迭代求解 */
            for (int iter = 0; iter < 50; ++iter) {
                QVector<double> Av(n, 0.0);
                if (left) {
                    /* u = A * v */
                    for (int j = 0; j < n; ++j)
                        for (int i = 0; i < m; ++i)
                            Av[j] += A[i][j] * v[i];
                    /* v = A^T * Av */
                    QVector<double> newV(m, 0.0);
                    for (int i = 0; i < m; ++i)
                        for (int j = 0; j < n; ++j)
                            newV[i] += A[i][j] * Av[j];
                    v = newV;
                } else {
                    /* v = A^T * u，直接用norm作为A */
                    for (int j = 0; j < n; ++j)
                        for (int i = 0; i < m; ++i)
                            Av[j] += (left ? A[i][j] : A[j][i]) * v[i];
                    v = Av;
                }

                /* 正交化 */
                for (int p = 0; p < r; ++p) {
                    double dot = 0.0;
                    for (int i = 0; i < m; ++i) dot += v[i] * basis[p][i];
                    for (int i = 0; i < m; ++i) v[i] -= dot * basis[p][i];
                }

                /* 归一化 */
                double nrm = 0.0;
                for (int i = 0; i < m; ++i) nrm += v[i] * v[i];
                nrm = qSqrt(qMax(nrm, 1e-30));
                for (int i = 0; i < m; ++i) v[i] /= nrm;
            }
            basis[r] = v;
        }
        return basis;
    };

    /* 获取行特征 (前k个左奇异向量) */
    auto rowBasis = powerIteration(norm, m_k, true);

    /* 获取列特征 - 使用矩阵转置 */
    QVector<QVector<double>> normT(cols, QVector<double>(rows, 0.0));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            normT[j][i] = norm[i][j];
    auto colBasis = powerIteration(normT, m_l, true);

    /* 第三步: KMeans聚类 */
    auto kMeans = [](const QVector<QVector<double>>& features, int k) -> QVector<int> {
        const int n = features.size();
        if (n == 0 || k <= 0) return QVector<int>();
        const int dim = features[0].size();
        k = qMin(k, n);

        /* 初始化质心: 均匀采样 */
        std::mt19937 rng(123);
        QVector<QVector<double>> centroids(k);
        for (int c = 0; c < k; ++c) {
            centroids[c] = features[(c * n) / k];
        }

        QVector<int> labels(n, 0);

        for (int iter = 0; iter < 50; ++iter) {
            bool changed = false;

            /* 分配步骤 */
            for (int i = 0; i < n; ++i) {
                double bestDist = 1e30;
                int bestC = 0;
                for (int c = 0; c < k; ++c) {
                    double dist = 0.0;
                    for (int d = 0; d < dim; ++d) {
                        double diff = features[i][d] - centroids[c][d];
                        dist += diff * diff;
                    }
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestC = c;
                    }
                }
                if (labels[i] != bestC) {
                    labels[i] = bestC;
                    changed = true;
                }
            }

            if (!changed) break;

            /* 更新质心 */
            QVector<QVector<double>> newCentroids(k, QVector<double>(dim, 0.0));
            QVector<int> counts(k, 0);
            for (int i = 0; i < n; ++i) {
                counts[labels[i]]++;
                for (int d = 0; d < dim; ++d)
                    newCentroids[labels[i]][d] += features[i][d];
            }
            for (int c = 0; c < k; ++c) {
                if (counts[c] > 0) {
                    for (int d = 0; d < dim; ++d)
                        newCentroids[c][d] /= counts[c];
                } else {
                    newCentroids[c] = features[rng() % n];
                }
            }
            centroids = newCentroids;
        }
        return labels;
    };

    /* 将行特征转为按样本组织 */
    QVector<QVector<double>> rowFeatures(rows, QVector<double>(m_k, 0.0));
    for (int r = 0; r < m_k; ++r)
        for (int i = 0; i < rows; ++i)
            rowFeatures[i][r] = rowBasis[r][i];

    QVector<QVector<double>> colFeatures(cols, QVector<double>(m_l, 0.0));
    for (int r = 0; r < m_l; ++r)
        for (int j = 0; j < cols; ++j)
            colFeatures[j][r] = colBasis[r][j];

    rowLabels = kMeans(rowFeatures, m_k);
    colLabels = kMeans(colFeatures, m_l);

    /* 更新统计信息 */
    m_stats.totalClusterings++;
    m_stats.totalElementsProcessed += rows * cols;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringComplete(m_k, m_l);
    return {rowLabels, colLabels};
}

/**
 * @brief 重置所有统计数据
 */
void SpectralBicluster2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
