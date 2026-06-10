/**
 * @file SpectralCluster15.cpp
 * @brief SpectralCluster15 实现
 *
 * 实现谱聚类：Ng-Jordan-Weiss归一化谱嵌入与旋转自动k确定的流形学习聚类。
 */

#include "utils/cluster276/SpectralCluster15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralCluster15::SpectralCluster15(QObject *parent)
    : QObject(parent) {}

SpectralCluster15::~SpectralCluster15() = default;

/* ---- Configuration ---- */

void SpectralCluster15::setSigma(double sigma) { m_sigma = qBound(0.01, sigma, 100.0); }
void SpectralCluster15::setKNeighbors(int k) { m_kNeighbors = qBound(1, k, 100); }
void SpectralCluster15::setMaxClusters(int k) { m_maxClusters = qBound(2, k, 50); }
void SpectralCluster15::setTolerance(double tol) { m_tolerance = qBound(1e-10, tol, 1.0); }

/* ---- Distance helper ---- */

double SpectralCluster15::distSq(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

/* ---- Build affinity matrix using RBF kernel ---- */

QVector<QVector<double>> SpectralCluster15::buildAffinityMatrix(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    double twoSigSq = 2.0 * m_sigma * m_sigma;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d2 = distSq(data[i], data[j]);
            double val = qExp(-d2 / twoSigSq);
            W[i][j] = val;
            W[j][i] = val;
        }
    }
    return W;
}

/* ---- Build NJW normalized Laplacian ---- */

void SpectralCluster15::buildNJWLaplacian(
    const QVector<QVector<double>>& W,
    QVector<QVector<double>>& L) const
{
    int n = W.size();
    // Compute D^{-1/2}
    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) sum += W[i][j];
        dInvSqrt[i] = (sum > 1e-15) ? 1.0 / qSqrt(sum) : 0.0;
    }

    // L_norm = D^{-1/2} W D^{-1/2}
    L.resize(n);
    for (int i = 0; i < n; ++i) {
        L[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j)
            L[i][j] = dInvSqrt[i] * W[i][j] * dInvSqrt[j];
    }
}

/* ---- Power iteration for top-k eigenvectors ---- */

void SpectralCluster15::computeEigenvectors(
    const QVector<QVector<double>>& L, int k,
    QVector<QVector<double>>& eigvecs,
    QVector<double>& eigvals)
{
    int n = L.size();
    eigvecs.resize(k);
    eigvals.resize(k, 0.0);

    QVector<QVector<double>> deflatedL = L;

    for (int ki = 0; ki < k; ++ki) {
        // Initialize random vector
        QVector<double> v(n, 0.0);
        for (int i = 0; i < n; ++i)
            v[i] = static_cast<double>(qrand()) / RAND_MAX - 0.5;

        // Normalize
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += v[i] * v[i];
        norm = qSqrt(norm);
        if (norm > 1e-15)
            for (int i = 0; i < n; ++i) v[i] /= norm;

        // Power iteration
        for (int iter = 0; iter < 300; ++iter) {
            QVector<double> Av(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    Av[i] += deflatedL[i][j] * v[j];

            // Orthogonalize against previous eigenvectors
            for (int prev = 0; prev < ki; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += Av[i] * eigvecs[prev][i];
                for (int i = 0; i < n; ++i) Av[i] -= dot * eigvecs[prev][i];
            }

            double newNorm = 0.0;
            for (int i = 0; i < n; ++i) newNorm += Av[i] * Av[i];
            newNorm = qSqrt(newNorm);

            if (newNorm > 1e-15)
                for (int i = 0; i < n; ++i) v[i] = Av[i] / newNorm;
            else
                break;
        }

        // Compute eigenvalue
        QVector<double> Av(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                Av[i] += deflatedL[i][j] * v[j];

        double lambda = 0.0;
        for (int i = 0; i < n; ++i) lambda += v[i] * Av[i];

        eigvecs[ki] = v;
        eigvals[ki] = lambda;
        emit eigenvalueComputed(ki, lambda);

        // Deflate
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                deflatedL[i][j] -= lambda * v[i] * v[j];
    }
}

/* ---- Normalize rows of eigenvector matrix ---- */

void SpectralCluster15::normalizeRows(QVector<QVector<double>>& mat) const
{
    for (int i = 0; i < mat.size(); ++i) {
        // mat is stored column-wise; transpose to row-wise first
    }
    // Assume mat is row-wise: rows = points, cols = k
    // Reinterpret: eigvecs columns -> rows
}

/* ---- K-means on spectral embedding ---- */

QVector<int> SpectralCluster15::kmeansOnEmbedding(
    const QVector<QVector<double>>& embedded, int k)
{
    int n = embedded.size();
    int dim = embedded.isEmpty() ? 0 : embedded[0].size();

    // Initialize centroids (k-means++)
    QVector<QVector<double>> centroids(k, QVector<double>(dim, 0.0));
    centroids[0] = embedded[0];

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n, 0.0);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = 1e18;
            for (int j = 0; j < c; ++j)
                minD = qMin(minD, distSq(embedded[i], centroids[j]));
            dists[i] = minD;
            totalDist += minD;
        }
        double r = static_cast<double>(qrand()) / RAND_MAX * totalDist;
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { centroids[c] = embedded[i]; break; }
        }
    }

    QVector<int> labels(n, 0);

    for (int iter = 0; iter < 200; ++iter) {
        // Assign
        QVector<int> newLabels(n, 0);
        for (int i = 0; i < n; ++i) {
            double minD = 1e18;
            for (int j = 0; j < k; ++j) {
                double d = distSq(embedded[i], centroids[j]);
                if (d < minD) { minD = d; newLabels[i] = j; }
            }
        }

        // Check convergence
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            if (newLabels[i] != labels[i]) { changed = true; break; }
        }
        labels = newLabels;
        if (!changed) break;

        // Update centroids
        QVector<int> count(k, 0);
        centroids = QVector<QVector<double>>(k, QVector<double>(dim, 0.0));
        for (int i = 0; i < n; ++i) {
            count[labels[i]]++;
            for (int d = 0; d < dim; ++d)
                centroids[labels[i]][d] += embedded[i][d];
        }
        for (int j = 0; j < k; ++j) {
            if (count[j] > 0)
                for (int d = 0; d < dim; ++d)
                    centroids[j][d] /= count[j];
        }
    }
    return labels;
}

/* ---- Determine optimal k via eigengap ---- */

int SpectralCluster15::determineOptimalK(const QVector<double>& eigenvalues) const
{
    int maxK = qMin(m_maxClusters, eigenvalues.size() - 1);
    if (maxK < 2) return 2;

    int bestK = 2;
    double maxGap = 0.0;
    for (int k = 1; k < maxK; ++k) {
        double gap = qAbs(eigenvalues[k] - eigenvalues[k - 1]);
        if (gap > maxGap) { maxGap = gap; bestK = k + 1; }
    }
    return bestK;
}

/* ---- Normalized cut value ---- */

double SpectralCluster15::normalizedCut(
    const QVector<QVector<double>>& data,
    const QVector<int>& labels, int k) const
{
    int n = data.size();
    double ncut = 0.0;

    for (int c = 0; c < k; ++c) {
        double cut = 0.0, assoc = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                double w = qExp(-distSq(data[i], data[j]) / (2.0 * m_sigma * m_sigma));
                assoc += w;
                if ((labels[i] == c) != (labels[j] == c)) cut += w;
            }
        }
        ncut += (assoc > 1e-15) ? cut / assoc : 0.0;
    }
    return ncut;
}

/* ---- Fit: main spectral clustering ---- */

SpectralCluster15::ClusterResult SpectralCluster15::fit(
    const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n < 2) return result;

    // Build affinity matrix
    auto W = buildAffinityMatrix(data);

    // Build NJW normalized Laplacian
    QVector<QVector<double>> L;
    buildNJWLaplacian(W, L);

    // Determine k
    int maxEig = qMin(m_maxClusters + 1, n);
    QVector<QVector<double>> eigvecs;
    QVector<double> eigvals;
    computeEigenvectors(L, maxEig, eigvecs, eigvals);

    if (k <= 0) {
        k = determineOptimalK(eigvals);
    }
    k = qBound(2, k, n);

    // Build embedding: n x k matrix from top-k eigenvectors (columns)
    // Normalize each row
    QVector<QVector<double>> embedded(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int ki = 0; ki < k; ++ki)
            embedded[i][ki] = eigvecs[ki][i];
        // Normalize row
        double rowNorm = 0.0;
        for (int ki = 0; ki < k; ++ki) rowNorm += embedded[i][ki] * embedded[i][ki];
        rowNorm = qSqrt(rowNorm);
        if (rowNorm > 1e-15)
            for (int ki = 0; ki < k; ++ki) embedded[i][ki] /= rowNorm;
    }

    // K-means on embedding
    result.labels = kmeansOnEmbedding(embedded, k);
    result.eigenvectors = eigvecs;
    result.eigenvalues = eigvals;
    result.optimalK = k;
    result.converged = true;

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = k;
    m_stats.sigma = m_sigma;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringDone(k, result.objectiveValue, elapsed);

    return result;
}

/* ---- Reset ---- */

void SpectralCluster15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
