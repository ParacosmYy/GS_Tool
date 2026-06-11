/**
 * @file SpectralCluster16.cpp
 * @brief SpectralCluster16 实现
 *
 * 实现谱聚类：归一化割比率与k路离散化特征映射嵌入实现多类图划分。
 */

#include "utils/cluster290/SpectralCluster16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralCluster16::SpectralCluster16(QObject *parent)
    : QObject(parent) {}

SpectralCluster16::~SpectralCluster16() = default;

/* ---- Configuration ---- */

void SpectralCluster16::setNumClusters(int k) { m_k = qBound(2, k, 200); }
void SpectralCluster16::setSigma(double sigma) { m_sigma = qBound(0.01, sigma, 100.0); }
void SpectralCluster16::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 5000); }
void SpectralCluster16::setKnnNeighbors(int knn) { m_knn = qBound(1, knn, 100); }

/* ---- Squared Euclidean distance ---- */

double SpectralCluster16::squaredDist(const QVector<double>& a,
                                        const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return d;
}

/* ---- Build RBF similarity matrix ---- */

QVector<QVector<double>> SpectralCluster16::buildSimilarity(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    double twoSigSq = 2.0 * m_sigma * m_sigma;

    for (int i = 0; i < n; ++i) {
        // Collect distances to all neighbors for kNN sparsification
        QVector<QPair<double, int>> dists(n);
        for (int j = 0; j < n; ++j) {
            double d = squaredDist(data[i], data[j]);
            double sim = qExp(-d / twoSigSq);
            dists[j] = {sim, j};
        }
        // Keep top-knn + self
        std::sort(dists.begin(), dists.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        int keep = qMin(m_knn + 1, n);
        for (int kk = 0; kk < keep; ++kk) {
            int j = dists[kk].second;
            W[i][j] = qMax(W[i][j], dists[kk].first);
            W[j][i] = W[i][j]; // Symmetric
        }
    }
    return W;
}

/* ---- Build normalized Laplacian ---- */

void SpectralCluster16::buildNormalizedLaplacian(
    const QVector<QVector<double>>& W,
    QVector<QVector<double>>& L) const
{
    int n = W.size();
    QVector<double> D(n, 0.0);

    // Compute degree vector
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            D[i] += W[i][j];

    // D^{-1/2}
    QVector<double> Dh(n, 0.0);
    for (int i = 0; i < n; ++i)
        Dh[i] = (D[i] > 1e-15) ? 1.0 / qSqrt(D[i]) : 0.0;

    // L_sym = I - D^{-1/2} W D^{-1/2}
    L.assign(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        L[i][i] = 1.0;
        for (int j = 0; j < n; ++j)
            L[i][j] -= Dh[i] * W[i][j] * Dh[j];
    }
}

/* ---- Extract top-k eigenvectors via power iteration ---- */

QVector<QVector<double>> SpectralCluster16::extractEigenvectors(
    const QVector<QVector<double>>& L, int k) const
{
    int n = L.size();
    QVector<QVector<double>> eigenvecs(k, QVector<double>(n, 0.0));

    for (int ev = 0; ev < k; ++ev) {
        // Initialize with random vector
        QVector<double> v(n);
        for (int i = 0; i < n; ++i)
            v[i] = static_cast<double>(qrand()) / RAND_MAX - 0.5;

        // Orthogonalize against previous eigenvectors
        for (int prev = 0; prev < ev; ++prev) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i)
                dot += v[i] * eigenvecs[prev][i];
            for (int i = 0; i < n; ++i)
                v[i] -= dot * eigenvecs[prev][i];
        }

        // Power iteration on (I - L) to find smallest eigenvectors
        for (int iter = 0; iter < m_maxIter; ++iter) {
            QVector<double> Lv(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    Lv[i] += (1.0 - L[i][j]) * v[j]; // (I - L) = normalized affinity

            // Deflate previous eigenvectors
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i)
                    dot += Lv[i] * eigenvecs[prev][i];
                for (int i = 0; i < n; ++i)
                    Lv[i] -= dot * eigenvecs[prev][i];
            }

            // Normalize
            double norm = 0.0;
            for (int i = 0; i < n; ++i)
                norm += Lv[i] * Lv[i];
            norm = qSqrt(norm);
            if (norm < 1e-15) break;
            for (int i = 0; i < n; ++i)
                v[i] = Lv[i] / norm;
        }

        eigenvecs[ev] = v;
    }

    // Transpose to [n×k]
    QVector<QVector<double>> result(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            result[i][j] = eigenvecs[j][i];

    return result;
}

/* ---- K-means discretization on embeddings ---- */

QVector<int> SpectralCluster16::kmeansDiscretize(
    const QVector<QVector<double>>& emb, int k) const
{
    int n = emb.size();
    int d = emb[0].size();

    // Initialize centroids from first k points
    QVector<QVector<double>> centroids(k);
    for (int j = 0; j < k; ++j)
        centroids[j] = emb[j % n];

    QVector<int> labels(n, 0);
    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;
        // Assign points to nearest centroid
        for (int i = 0; i < n; ++i) {
            double bestD = 1e300;
            int bestJ = 0;
            for (int j = 0; j < k; ++j) {
                double d2 = squaredDist(emb[i], centroids[j]);
                if (d2 < bestD) { bestD = d2; bestJ = j; }
            }
            if (labels[i] != bestJ) { labels[i] = bestJ; changed = true; }
        }
        if (!changed) break;

        // Update centroids
        centroids.assign(k, QVector<double>(d, 0.0));
        QVector<int> counts(k, 0);
        for (int i = 0; i < n; ++i) {
            counts[labels[i]]++;
            for (int dd = 0; dd < d; ++dd)
                centroids[labels[i]][dd] += emb[i][dd];
        }
        for (int j = 0; j < k; ++j)
            if (counts[j] > 0)
                for (int dd = 0; dd < d; ++dd)
                    centroids[j][dd] /= counts[j];
    }
    return labels;
}

/* ---- Normalized cut ratio ---- */

double SpectralCluster16::normalizedCutRatio(
    const QVector<QVector<double>>& similarity,
    const QVector<int>& labels) const
{
    int n = labels.size();
    int k = m_k;
    QVector<double> assoc(k, 0.0);  // Association within cluster
    QVector<double> cut(k, 0.0);     // Cut to outside

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double w = (i < similarity.size() && j < similarity[i].size())
                           ? similarity[i][j] : 0.0;
            if (labels[i] == labels[j])
                assoc[labels[i]] += w;
            else {
                cut[labels[i]] += w;
                cut[labels[j]] += w;
            }
        }
    }

    double ncut = 0.0;
    for (int c = 0; c < k; ++c) {
        double denom = assoc[c] + cut[c];
        if (denom > 1e-15)
            ncut += cut[c] / denom;
    }
    return ncut;
}

/* ---- Main fit ---- */

SpectralCluster16::ClusterResult SpectralCluster16::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n < m_k) return result;
    m_dims = data[0].size();

    // Build similarity graph
    auto W = buildSimilarity(data);

    // Build normalized Laplacian
    QVector<QVector<double>> L;
    buildNormalizedLaplacian(W, L);

    // Extract spectral embeddings
    auto embeddings = extractEigenvectors(L, m_k);

    // Normalize rows of embedding
    for (int i = 0; i < n; ++i) {
        double norm = 0.0;
        for (int j = 0; j < m_k; ++j)
            norm += embeddings[i][j] * embeddings[i][j];
        norm = qSqrt(norm);
        if (norm > 1e-15)
            for (int j = 0; j < m_k; ++j)
                embeddings[i][j] /= norm;
    }

    // K-way discretization
    result.labels = kmeansDiscretize(embeddings, m_k);
    result.embeddings = embeddings;
    result.numClusters = m_k;
    result.ncutValue = normalizedCutRatio(W, result.labels);

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = m_k;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitDone(n, m_k, result.ncutValue, elapsed);
    return result;
}

/* ---- Reset ---- */

void SpectralCluster16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_dims = 0;
}
