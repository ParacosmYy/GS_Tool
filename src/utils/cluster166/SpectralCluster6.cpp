/**
 * @file SpectralCluster6.cpp
 * @brief SpectralCluster6 实现
 *
 * 实现谱聚类：相似度矩阵构建、归一化拉普拉斯、幂迭代特征向量、k-means。
 */

#include "utils/cluster166/SpectralCluster6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

SpectralCluster6::SpectralCluster6(QObject *parent)
    : QObject(parent)
{
}

SpectralCluster6::~SpectralCluster6() = default;

void SpectralCluster6::setKernelType(KernelType type) { m_kernel = type; }
void SpectralCluster6::setSigma(double sigma) { m_sigma = qMax(0.01, sigma); }
void SpectralCluster6::setKNN(int k) { m_knn = qMax(1, k); }
void SpectralCluster6::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }

void SpectralCluster6::buildAffinityMatrix(const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_W.assign(n, QVector<double>(n, 0.0));

    if (m_kernel == RBF) {
        /* Gaussian RBF kernel: w_ij = exp(-||xi-xj||^2 / (2*sigma^2)) */
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) {
                double d = euclidean(data[i], data[j]);
                double w = qExp(-d * d / (2.0 * m_sigma * m_sigma));
                m_W[i][j] = w;
                m_W[j][i] = w;
            }
    } else if (m_kernel == KNN) {
        /* KNN: w_ij = exp(-d^2/2sigma^2) if j is among i's k-nearest */
        for (int i = 0; i < n; ++i) {
            QVector<QPair<double, int>> dists;
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                dists.append({euclidean(data[i], data[j]), j});
            }
            std::sort(dists.begin(), dists.end());
            for (int k = 0; k < qMin(m_knn, dists.size()); ++k) {
                int j = dists[k].second;
                double w = qExp(-dists[k].first * dists[k].first / (2.0 * m_sigma * m_sigma));
                m_W[i][j] = qMax(m_W[i][j], w);
                m_W[j][i] = m_W[i][j];
            }
        }
    } else {
        /* Epsilon neighborhood */
        double eps = m_sigma;
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) {
                double d = euclidean(data[i], data[j]);
                if (d < eps) {
                    m_W[i][j] = 1.0;
                    m_W[j][i] = 1.0;
                }
            }
    }
}

void SpectralCluster6::buildNormalizedLaplacian(int n)
{
    /* L_sym = I - D^{-1/2} W D^{-1/2} */
    m_L.assign(n, QVector<double>(n, 0.0));
    QVector<double> dInvSqrt(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) sum += m_W[i][j];
        dInvSqrt[i] = (sum > 1e-15) ? 1.0 / qSqrt(sum) : 0.0;
    }

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            m_L[i][j] = -dInvSqrt[i] * m_W[i][j] * dInvSqrt[j];
    for (int i = 0; i < n; ++i)
        m_L[i][i] += 1.0;
}

void SpectralCluster6::computeEigenvectors(int n, int k)
{
    /* Power iteration for k smallest eigenvectors of L */
    m_embeddings.assign(k, QVector<double>(n, 0.0));

    for (int ev = 0; ev < k; ++ev) {
        /* Random init */
        QVector<double> v(n);
        for (int i = 0; i < n; ++i)
            v[i] = static_cast<double>(qrand()) / RAND_MAX + 0.1;

        /* Orthogonalize against previous eigenvectors */
        for (int prev = 0; prev < ev; ++prev) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += v[i] * m_embeddings[prev][i];
            for (int i = 0; i < n; ++i) v[i] -= dot * m_embeddings[prev][i];
        }

        for (int iter = 0; iter < 300; ++iter) {
            /* Matrix-vector product: L * v */
            QVector<double> Lv(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    Lv[i] += m_L[i][j] * v[j];

            /* Re-orthogonalize */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += Lv[i] * m_embeddings[prev][i];
                for (int i = 0; i < n; ++i) Lv[i] -= dot * m_embeddings[prev][i];
            }

            /* Normalize */
            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += Lv[i] * Lv[i];
            norm = qSqrt(qMax(norm, 1e-30));
            for (int i = 0; i < n; ++i) v[i] = Lv[i] / norm;
        }

        for (int i = 0; i < n; ++i) m_embeddings[ev][i] = v[i];
    }
}

QVector<int> SpectralCluster6::kmeansOnEmbeddings(int k)
{
    int n = m_embeddings[0].size();
    int dim = m_embeddings.size();

    /* Initialize centroids randomly */
    m_centroids.assign(k, QVector<double>(dim, 0.0));
    for (int c = 0; c < k; ++c) {
        int idx = c * n / k;
        for (int d = 0; d < dim; ++d)
            m_centroids[c][d] = m_embeddings[d][idx];
    }

    QVector<int> labels(n, 0);
    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* Assign */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e30;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = m_embeddings[dd][i] - m_centroids[c][dd];
                    d += diff * diff;
                }
                if (d < bestDist) { bestDist = d; labels[i] = c; }
            }
        }
        /* Update centroids */
        QVector<double> counts(k, 0.0);
        for (auto& ctr : m_centroids) ctr.assign(dim, 0.0);
        for (int i = 0; i < n; ++i) {
            counts[labels[i]] += 1.0;
            for (int d = 0; d < dim; ++d)
                m_centroids[labels[i]][d] += m_embeddings[d][i];
        }
        for (int c = 0; c < k; ++c)
            if (counts[c] > 1e-15)
                for (int d = 0; d < dim; ++d)
                    m_centroids[c][d] /= counts[c];
    }
    return labels;
}

double SpectralCluster6::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

QVector<int> SpectralCluster6::fit(const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || k <= 0) return QVector<int>();
    k = qMin(k, n);

    buildAffinityMatrix(data);
    buildNormalizedLaplacian(n);
    computeEigenvectors(n, k);
    QVector<int> labels = kmeansOnEmbeddings(k);

    m_stats.totalRuns++;
    m_stats.lastClusterCount = k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0) ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(k, m_stats.lastIterations);
    return labels;
}

QVector<QVector<double>> SpectralCluster6::embeddings() const { return m_embeddings; }
QVector<QVector<double>> SpectralCluster6::centroids() const { return m_centroids; }

void SpectralCluster6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
