/**
 * @file SpectralCluster11.cpp
 * @brief SpectralCluster11 实现
 *
 * 实现谱聚类：未归一化拉普拉斯、特征间隙自动聚类数选择。
 */

#include "utils/cluster220/SpectralCluster11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralCluster11::SpectralCluster11(QObject *parent) : QObject(parent) {}
SpectralCluster11::~SpectralCluster11() = default;

/* ---- Configuration ---- */

void SpectralCluster11::setParameters(int maxClusters, double sigma, int maxIter)
{
    m_maxClusters = qMax(2, maxClusters);
    m_sigma = qMax(0.01, sigma);
    m_maxIter = qMax(10, maxIter);
}

/* ---- RBF similarity matrix ---- */

void SpectralCluster11::computeSimilarity(const QVector<QVector<double>>& data,
                                            QVector<QVector<double>>& W) const
{
    int n = data.size();
    W.resize(n);
    double s2 = 2.0 * m_sigma * m_sigma;
    for (int i = 0; i < n; ++i) {
        W[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j) {
            double d = 0.0;
            for (int k = 0; k < qMin(data[i].size(), data[j].size()); ++k) {
                double diff = data[i][k] - data[j][k];
                d += diff * diff;
            }
            W[i][j] = qExp(-d / s2);
        }
    }
}

/* ---- Degree matrix ---- */

void SpectralCluster11::computeDegree(const QVector<QVector<double>>& W,
                                        QVector<double>& D) const
{
    int n = W.size();
    D.resize(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            D[i] += W[i][j];
}

/* ---- Unnormalized Laplacian L = D - W ---- */

void SpectralCluster11::computeLaplacian(const QVector<QVector<double>>& W,
                                           const QVector<double>& D,
                                           QVector<QVector<double>>& L) const
{
    int n = W.size();
    L.resize(n);
    for (int i = 0; i < n; ++i) {
        L[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j)
            L[i][j] = (i == j) ? D[i] - W[i][j] : -W[i][j];
    }
}

/* ---- QR-based eigen decomposition (smallest k eigenvalues) ---- */

void SpectralCluster11::eigenDecompose(QVector<QVector<double>>& L, int numEig)
{
    int n = L.size();
    if (n == 0 || numEig <= 0) return;
    numEig = qMin(numEig, n);

    // Initialize eigenvectors to identity
    m_eigenvectors.resize(numEig);
    for (int k = 0; k < numEig; ++k) {
        m_eigenvectors[k].resize(n, 0.0);
        m_eigenvectors[k][k % n] = 1.0;
    }

    // Power iteration with deflation for smallest eigenvalues
    m_eigenvalues.resize(numEig, 0.0);
    for (int k = 0; k < numEig; ++k) {
        QVector<double>& v = m_eigenvectors[k];
        // Normalize
        double nv = 0.0;
        for (int i = 0; i < n; ++i) nv += v[i] * v[i];
        if (nv > 1e-12) for (int i = 0; i < n; ++i) v[i] /= qSqrt(nv);

        // Inverse iteration shifted toward 0 for smallest eigenvalues
        for (int iter = 0; iter < 50; ++iter) {
            QVector<double> w(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    w[i] += L[i][j] * v[j];

            // Deflate against previously found eigenvectors
            for (int p = 0; p < k; ++p) {
                double proj = 0.0;
                for (int i = 0; i < n; ++i) proj += w[i] * m_eigenvectors[p][i];
                for (int i = 0; i < n; ++i) w[i] -= proj * m_eigenvectors[p][i];
            }

            double nw = 0.0;
            for (int i = 0; i < n; ++i) nw += w[i] * w[i];
            if (nw < 1e-15) break;
            for (int i = 0; i < n; ++i) v[i] = w[i] / qSqrt(nw);
        }

        // Rayleigh quotient
        double lambda = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                lambda += v[i] * L[i][j] * v[j];
        m_eigenvalues[k] = lambda;
    }
}

/* ---- Eigengap heuristic ---- */

int SpectralCluster11::eigengapHeuristic(const QVector<double>& evals) const
{
    if (evals.size() < 3) return 1;
    double maxGap = 0.0;
    int bestK = 1;
    int limit = qMin(evals.size() - 1, m_maxClusters);
    for (int k = 1; k < limit; ++k) {
        double gap = qAbs(evals[k + 1] - evals[k]);
        if (gap > maxGap) { maxGap = gap; bestK = k + 1; }
    }
    return qBound(1, bestK, m_maxClusters);
}

/* ---- K-means on eigenvector rows ---- */

QVector<int> SpectralCluster11::kmeansOnEigenvectors(
    const QVector<QVector<double>>& evecs, int k) const
{
    int n = evecs.isEmpty() ? 0 : evecs[0].size();
    int d = evecs.size();
    if (n == 0 || k <= 0) return QVector<int>(n, 0);

    // Initialize centroids from first k points
    QVector<QVector<double>> cents(k);
    for (int c = 0; c < k; ++c) {
        cents[c].resize(d);
        for (int j = 0; j < d; ++j)
            cents[c][j] = evecs[j][c % n];
    }

    QVector<int> labels(n, 0);
    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;
        // Assign
        for (int i = 0; i < n; ++i) {
            double bestD = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double dd = 0.0;
                for (int j = 0; j < d; ++j) {
                    double diff = evecs[j][i] - cents[c][j];
                    dd += diff * diff;
                }
                if (dd < bestD) { bestD = dd; bestC = c; }
            }
            if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
        }
        if (!changed) break;

        // Update centroids
        QVector<double> counts(k, 0.0);
        for (auto& c : cents) c.fill(0.0);
        for (int i = 0; i < n; ++i) {
            counts[labels[i]] += 1.0;
            for (int j = 0; j < d; ++j)
                cents[labels[i]][j] += evecs[j][i];
        }
        for (int c = 0; c < k; ++c)
            if (counts[c] > 0.0)
                for (int j = 0; j < d; ++j) cents[c][j] /= counts[c];
    }
    return labels;
}

/* ---- Fit with auto cluster count ---- */

SpectralCluster11::ClusterResult SpectralCluster11::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return ClusterResult();
    int n = data.size();
    m_dim = data[0].size();
    m_stats.numPoints = n;
    m_stats.dim = m_dim;
    m_stats.maxClusters = m_maxClusters;

    // Build similarity, degree, Laplacian
    QVector<QVector<double>> W, L;
    QVector<double> D;
    computeSimilarity(data, W);
    computeDegree(W, D);
    computeLaplacian(W, D, L);

    // Eigendecompose (get maxClusters smallest eigenvectors)
    int numEig = qMin(m_maxClusters + 1, n);
    eigenDecompose(L, numEig);

    // Eigengap heuristic
    int k = eigengapHeuristic(m_eigenvalues);
    m_clusters = k;
    m_stats.numClusters = k;

    // K-means on first k eigenvectors
    QVector<QVector<double>> usedEvecs(k);
    for (int i = 0; i < k; ++i) usedEvecs[i] = m_eigenvectors[i];
    QVector<int> labels = kmeansOnEigenvectors(usedEvecs, k);

    // Compute centroids for prediction
    m_centroids.resize(k);
    for (int c = 0; c < k; ++c) {
        m_centroids[c].resize(m_dim, 0.0);
        double cnt = 0.0;
        for (int i = 0; i < n; ++i) {
            if (labels[i] == c) {
                for (int d = 0; d < m_dim; ++d)
                    m_centroids[c][d] += data[i][d];
                cnt += 1.0;
            }
        }
        if (cnt > 0) for (int d = 0; d < m_dim; ++d) m_centroids[c][d] /= cnt;
    }

    ClusterResult result;
    result.labels = labels;
    result.eigenvectors = m_eigenvectors;
    result.eigenvalues = m_eigenvalues;
    result.numClusters = k;
    result.iterations = m_maxIter;
    m_stats.totalIterations += result.iterations;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(k, result.iterations, timer.elapsed());
    return result;
}

/* ---- Fit with specified cluster count ---- */

SpectralCluster11::ClusterResult SpectralCluster11::fit(
    const QVector<QVector<double>>& data, int clusters)
{
    m_maxClusters = qMax(2, clusters);
    return fit(data);
}

/* ---- Predict ---- */

QVector<int> SpectralCluster11::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size(), 0);
    for (int i = 0; i < data.size(); ++i) {
        double bestD = std::numeric_limits<double>::max();
        for (int c = 0; c < m_centroids.size(); ++c) {
            double d = 0.0;
            for (int k = 0; k < qMin(data[i].size(), m_centroids[c].size()); ++k) {
                double diff = data[i][k] - m_centroids[c][k];
                d += diff * diff;
            }
            if (d < bestD) { bestD = d; labels[i] = c; }
        }
    }
    return labels;
}

/* ---- Centroids ---- */

QVector<QVector<double>> SpectralCluster11::centroids() const { return m_centroids; }

/* ---- Reset ---- */

void SpectralCluster11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
    m_eigenvectors.clear();
    m_eigenvalues.clear();
    m_clusters = 0;
    m_dim = 0;
}
