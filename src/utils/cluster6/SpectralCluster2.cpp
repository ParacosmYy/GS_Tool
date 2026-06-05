/**
 * @file SpectralCluster2.cpp
 * @brief 谱聚类算法实现,基于归一化切(Normalized Cut)
 */

#include "SpectralCluster2.h"
#include <QElapsedTimer>
#include <QMap>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>

SpectralCluster2::SpectralCluster2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<int> SpectralCluster2::cluster(const QVector<QVector<double>>& data,
                                       int k, double sigma,
                                       GraphMode graphMode,
                                       LaplacianMode lapMode)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> labels;
    int n = data.size();
    if (n == 0 || k <= 0 || k > n) return labels;

    /* 步骤1: 构建相似度权重矩阵 */
    QVector<QVector<double>> W = buildWeightMatrix(data, sigma, graphMode);

    /* 步骤2: 计算拉普拉斯矩阵 */
    QVector<QVector<double>> L = computeLaplacian(W, lapMode);

    /* 步骤3: 特征分解,取前k个最小特征值对应的特征向量 */
    QVector<QVector<double>> embedded = eigenDecompose(L, k);

    /* 步骤4: 对嵌入空间做k-means聚类 */
    labels = kmeansEmbed(embedded, k, 100);

    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(k, n);
    return labels;
}

int SpectralCluster2::estimateK(const QVector<QVector<double>>& data,
                                int maxK, double sigma)
{
    int n = data.size();
    if (n == 0) return 1;

    maxK = qMin(maxK, n);
    QVector<QVector<double>> W = buildWeightMatrix(data, sigma, KNearest);
    QVector<QVector<double>> L = computeLaplacian(W, Symmetric);

    /* 计算前(maxK+1)个特征值 */
    QVector<QVector<double>> eigvecs = eigenDecompose(L, maxK + 1);

    /* 用幂迭代近似特征值(通过瑞利商) */
    QVector<double> eigenvalues(maxK + 1, 0.0);
    for (int i = 0; i < eigvecs.size() && i <= maxK; ++i) {
        double num = 0.0, den = 0.0;
        int dim = eigvecs[i].size();
        for (int j = 0; j < dim; ++j) {
            double Lv = 0.0;
            for (int p = 0; p < dim; ++p)
                Lv += L[j][p] * eigvecs[i][p];
            num += eigvecs[i][j] * Lv;
            den += eigvecs[i][j] * eigvecs[i][j];
        }
        eigenvalues[i] = (den > 1e-15) ? num / den : 0.0;
    }

    /* 特征间隙法: 找最大间隙 */
    int bestK = 1;
    double maxGap = 0.0;
    for (int i = 1; i < maxK; ++i) {
        double gap = std::abs(eigenvalues[i + 1] - eigenvalues[i]);
        if (gap > maxGap) {
            maxGap = gap;
            bestK = i + 1;
        }
    }

    return qBound(1, bestK, maxK);
}

double SpectralCluster2::normalizedCut(const QVector<QVector<double>>& data,
                                       const QVector<int>& labels,
                                       double sigma)
{
    int n = data.size();
    if (n == 0 || labels.size() != n) return 0.0;

    QVector<QVector<double>> W = buildWeightMatrix(data, sigma, Full);

    /* 计算度向量 */
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            d[i] += W[i][j];

    /* 按标签分组 */
    QMap<int, QVector<int>> groups;
    for (int i = 0; i < n; ++i)
        groups[labels[i]].append(i);

    double ncut = 0.0;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        const QVector<int>& A = it.value();
        double cutVal = 0.0, assocVal = 0.0;

        for (int i : A) {
            assocVal += d[i];
            for (int j = 0; j < n; ++j) {
                if (!A.contains(j))
                    cutVal += W[i][j];
            }
        }
        ncut += (assocVal > 1e-15) ? cutVal / assocVal : 0.0;
    }

    return ncut;
}

QVector<QVector<double>> SpectralCluster2::buildWeightMatrix(
    const QVector<QVector<double>>& data, double sigma,
    GraphMode mode, int knn) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    double sigma2 = 2.0 * sigma * sigma;

    /* 计算所有点对的RBF相似度 */
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dist = euclideanDist(data[i], data[j]);
            double sim = std::exp(-dist * dist / sigma2);
            W[i][j] = sim;
            W[j][i] = sim;
        }
    }

    if (mode == KNearest && knn > 0 && knn < n) {
        /* 保留k近邻: 对每行取top-k,其余置零 */
        for (int i = 0; i < n; ++i) {
            QVector<QPair<double, int>> sorted;
            for (int j = 0; j < n; ++j) {
                if (j != i) sorted.append({W[i][j], j});
            }
            std::sort(sorted.begin(), sorted.end(),
                      [](const auto& a, const auto& b) {
                          return a.first > b.first;
                      });
            QVector<double> newRow(n, 0.0);
            for (int t = 0; t < qMin(knn, sorted.size()); ++t) {
                int j = sorted[t].second;
                newRow[j] = W[i][j];
            }
            W[i] = newRow;
        }
        /* 对称化: W = max(W, W^T) */
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) {
                double v = std::max(W[i][j], W[j][i]);
                W[i][j] = v;
                W[j][i] = v;
            }
    }

    return W;
}

QVector<QVector<double>> SpectralCluster2::computeLaplacian(
    const QVector<QVector<double>>& W, LaplacianMode mode) const
{
    int n = W.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));

    /* 度矩阵D */
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            d[i] += W[i][j];

    if (mode == Unnormalized) {
        /* L = D - W */
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                L[i][j] = (i == j ? d[i] : 0.0) - W[i][j];
    } else {
        /* D^{-1/2} */
        QVector<double> dInvSqrt(n, 0.0);
        for (int i = 0; i < n; ++i)
            dInvSqrt[i] = (d[i] > 1e-15) ? 1.0 / std::sqrt(d[i]) : 0.0;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j)
                    L[i][j] = 1.0 - (dInvSqrt[i] > 0 ? 1.0 : 0.0);
                else
                    L[i][j] = -dInvSqrt[i] * W[i][j] * dInvSqrt[j];
            }
        }
    }

    return L;
}

QVector<QVector<double>> SpectralCluster2::eigenDecompose(
    const QVector<QVector<double>>& mat, int numEigenvectors) const
{
    int n = mat.size();
    numEigenvectors = qMin(numEigenvectors, n);

    /* 使用随机SVD近似特征向量(适合中等规模数据) */
    std::mt19937 rng(42);
    std::normal_distribution<double> dist(0.0, 1.0);

    /* 随机初始化投影矩阵 */
    QVector<QVector<double>> Q(n, QVector<double>(numEigenvectors, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < numEigenvectors; ++j)
            Q[i][j] = dist(rng);

    /* 幂迭代 + QR正交化(5轮) */
    for (int iter = 0; iter < 5; ++iter) {
        /* Y = L * Q */
        QVector<QVector<double>> Y(n, QVector<double>(numEigenvectors, 0.0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < numEigenvectors; ++j)
                for (int p = 0; p < n; ++p)
                    Y[i][j] += mat[i][p] * Q[p][j];

        /* Gram-Schmidt正交化 */
        for (int j = 0; j < numEigenvectors; ++j) {
            for (int k = 0; k < j; ++k) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i)
                    dot += Y[i][j] * Y[i][k];
                for (int i = 0; i < n; ++i)
                    Y[i][j] -= dot * Y[i][k];
            }
            double norm = 0.0;
            for (int i = 0; i < n; ++i)
                norm += Y[i][j] * Y[i][j];
            norm = std::sqrt(norm);
            if (norm > 1e-15)
                for (int i = 0; i < n; ++i)
                    Y[i][j] /= norm;
        }
        Q = Y;
    }

    /* 提取各特征向量 */
    QVector<QVector<double>> result(numEigenvectors);
    for (int j = 0; j < numEigenvectors; ++j) {
        result[j].resize(n);
        for (int i = 0; i < n; ++i)
            result[j][i] = Q[i][j];
    }

    return result;
}

QVector<int> SpectralCluster2::kmeansEmbed(
    const QVector<QVector<double>>& embedded, int k, int maxIter) const
{
    int n = embedded[0].size();
    int dim = embedded.size();
    QVector<int> labels(n, 0);

    std::mt19937 rng(123);
    std::uniform_int_distribution<int> uid(0, n - 1);

    /* 随机初始化聚类中心 */
    QVector<QVector<double>> centers(k, QVector<double>(dim, 0.0));
    for (int c = 0; c < k; ++c) {
        int idx = uid(rng);
        for (int d = 0; d < dim; ++d)
            centers[c][d] = embedded[d][idx];
    }

    /* 迭代 */
    for (int iter = 0; iter < maxIter; ++iter) {
        bool changed = false;

        /* 分配 */
        for (int i = 0; i < n; ++i) {
            QVector<double> point(dim);
            for (int d = 0; d < dim; ++d)
                point[d] = embedded[d][i];

            double bestDist = 1e30;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double dist = euclideanDist(point, centers[c]);
                if (dist < bestDist) { bestDist = dist; bestC = c; }
            }
            if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
        }

        if (!changed) break;

        /* 更新中心 */
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(dim, 0.0);
            int count = 0;
            for (int i = 0; i < n; ++i) {
                if (labels[i] == c) {
                    for (int d = 0; d < dim; ++d)
                        sum[d] += embedded[d][i];
                    count++;
                }
            }
            if (count > 0)
                for (int d = 0; d < dim; ++d)
                    centers[c][d] = sum[d] / count;
        }
    }

    return labels;
}

double SpectralCluster2::euclideanDist(const QVector<double>& a,
                                       const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return std::sqrt(sum);
}

SpectralCluster2::Stats SpectralCluster2::stats() const { return m_stats; }

void SpectralCluster2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
