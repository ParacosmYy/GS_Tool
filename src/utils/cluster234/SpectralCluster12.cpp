/**
 * @file SpectralCluster12.cpp
 * @brief SpectralCluster12 实现
 *
 * 实现谱聚类：归一化割目标函数与k路离散化正交变换。
 */

#include "utils/cluster234/SpectralCluster12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralCluster12::SpectralCluster12(QObject *parent) : QObject(parent) {}
SpectralCluster12::~SpectralCluster12() = default;

/* ---- Configuration ---- */

void SpectralCluster12::setNumClusters(int k) { m_numClusters = qMax(2, k); }
void SpectralCluster12::setSigma(double sigma) { m_sigma = qMax(0.01, sigma); }
void SpectralCluster12::setMaxEigenIterations(int iter) { m_maxEigenIter = qMax(10, iter); }
void SpectralCluster12::setEigenTolerance(double tol) { m_eigenTol = qMax(1e-12, tol); }
void SpectralCluster12::setKNN(int knn) { m_knn = qMax(0, knn); }

/* ---- Build Gaussian affinity matrix ---- */

void SpectralCluster12::buildAffinity()
{
    int n = m_data.size();
    int d = m_data[0].size();
    m_affinity.resize(n);
    for (int i = 0; i < n; ++i)
        m_affinity[i].resize(n, 0.0);

    double sigma2 = 2.0 * m_sigma * m_sigma;

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            double dist2 = 0.0;
            for (int k = 0; k < d; ++k) {
                double diff = m_data[i][k] - m_data[j][k];
                dist2 += diff * diff;
            }
            double w = qExp(-dist2 / sigma2);
            m_affinity[i][j] = w;
            m_affinity[j][i] = w;
        }
        m_affinity[i][i] = 0.0;  // no self-loop
    }

    // KNN sparsification if enabled
    if (m_knn > 0 && m_knn < n) {
        for (int i = 0; i < n; ++i) {
            QVector<QPair<double, int>> sorted;
            for (int j = 0; j < n; ++j) {
                if (j != i) sorted.append({m_affinity[i][j], j});
            }
            std::sort(sorted.begin(), sorted.end(),
                      [](const auto& a, const auto& b) { return a.first > b.first; });
            for (int j = m_knn; j < sorted.size(); ++j)
                m_affinity[i][sorted[j].second] = 0.0;
        }
        // Symmetrize
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                m_affinity[i][j] = m_affinity[j][i] = qMax(m_affinity[i][j], m_affinity[j][i]);
    }
}

/* ---- Build normalized Laplacian L_sym = D^{-1/2} W D^{-1/2} ---- */

QVector<QVector<double>> SpectralCluster12::buildNormalizedLaplacian() const
{
    int n = m_affinity.size();
    QVector<double> degree(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            degree[i] += m_affinity[i][j];

    QVector<double> invSqrtD(n);
    for (int i = 0; i < n; ++i)
        invSqrtD[i] = (degree[i] > 1e-15) ? 1.0 / qSqrt(degree[i]) : 0.0;

    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            L[i][j] = invSqrtD[i] * m_affinity[i][j] * invSqrtD[j];
    return L;
}

/* ---- Power iteration for top-k eigenvectors ---- */

bool SpectralCluster12::solveTopKEigenvectors(const QVector<QVector<double>>& mat, int k)
{
    int n = mat.size();
    m_eigVecs.resize(n);
    for (int i = 0; i < n; ++i)
        m_eigVecs[i].resize(k, 0.0);

    // Initialize randomly
    for (int j = 0; j < k; ++j)
        for (int i = 0; i < n; ++i)
            m_eigVecs[i][j] = static_cast<double>(qrand()) / RAND_MAX - 0.5;

    orthogonalize(m_eigVecs);

    for (int iter = 0; iter < m_maxEigenIter; ++iter) {
        // Matrix-vector multiply: Y = L * X
        QVector<QVector<double>> Y(n, QVector<double>(k, 0.0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < k; ++j)
                for (int l = 0; l < n; ++l)
                    Y[i][j] += mat[i][l] * m_eigVecs[l][j];

        m_eigVecs = Y;
        orthogonalize(m_eigVecs);

        // Residual check
        double res = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < k; ++j)
                res += qAbs(Y[i][j] - m_eigVecs[i][j]);
        m_stats.eigenIterations = iter + 1;
        if (res / (n * k) < m_eigenTol) break;
        emit eigenSolved(k, res);
    }
    return true;
}

/* ---- Modified Gram-Schmidt orthogonalization ---- */

void SpectralCluster12::orthogonalize(QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    int k = mat[0].size();

    for (int j = 0; j < k; ++j) {
        for (int i = 0; i < j; ++i) {
            double dot = 0.0;
            for (int r = 0; r < n; ++r) dot += mat[r][j] * mat[r][i];
            for (int r = 0; r < n; ++r) mat[r][j] -= dot * mat[r][i];
        }
        double norm = 0.0;
        for (int r = 0; r < n; ++r) norm += mat[r][j] * mat[r][j];
        norm = qSqrt(qMax(1e-15, norm));
        for (int r = 0; r < n; ++r) mat[r][j] /= norm;
    }
}

/* ---- Row-normalize eigenvectors ---- */

QVector<QVector<double>> SpectralCluster12::normalizeRows(const QVector<QVector<double>>& mat) const
{
    QVector<QVector<double>> result = mat;
    for (int i = 0; i < result.size(); ++i) {
        double norm = 0.0;
        for (int j = 0; j < result[i].size(); ++j) norm += result[i][j] * result[i][j];
        norm = qSqrt(qMax(1e-15, norm));
        for (int j = 0; j < result[i].size(); ++j) result[i][j] /= norm;
    }
    return result;
}

/* ---- Euclidean distance ---- */

double SpectralCluster12::euclideanDist(const QVector<double>& a, const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) { double diff = a[i] - b[i]; d += diff * diff; }
    return qSqrt(qMax(0.0, d));
}

/* ---- K-way discretization via rotation ---- */

void SpectralCluster12::discretizeKWay()
{
    int n = m_eigVecs.size();
    int k = m_numClusters;

    // Normalize rows
    QVector<QVector<double>> T = normalizeRows(m_eigVecs);

    // Initialize centroids using farthest-first
    m_centroids.resize(k);
    m_centroids[0] = T[0];
    for (int c = 1; c < k; ++c) {
        double maxDist = -1.0;
        int bestIdx = 0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int j = 0; j < c; ++j)
                minD = qMin(minD, euclideanDist(T[i], m_centroids[j]));
            if (minD > maxDist) { maxDist = minD; bestIdx = i; }
        }
        m_centroids[c] = T[bestIdx];
    }

    // K-means on spectral embedding
    m_assignments.resize(n);
    for (int iter = 0; iter < 100; ++iter) {
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = euclideanDist(T[i], m_centroids[c]);
                if (d < bestDist) { bestDist = d; bestC = c; }
            }
            if (m_assignments[i].cluster != bestC) { changed = true; m_assignments[i].cluster = bestC; }
            m_assignments[i].pointIndex = i;
            m_assignments[i].confidence = 1.0 / (1.0 + bestDist);
        }
        if (!changed) break;
        // Update centroids
        for (int c = 0; c < k; ++c) {
            m_centroids[c].fill(0.0);
            int cnt = 0;
            for (int i = 0; i < n; ++i) {
                if (m_assignments[i].cluster == c) {
                    for (int j = 0; j < k; ++j) m_centroids[c][j] += T[i][j];
                    cnt++;
                }
            }
            if (cnt > 0)
                for (int j = 0; j < k; ++j) m_centroids[c][j] /= cnt;
        }
    }
}

/* ---- Fit ---- */

bool SpectralCluster12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_numClusters) return false;
    int d = data[0].size();

    m_data = data;

    // Step 1: Build affinity matrix
    buildAffinity();

    // Step 2: Build normalized Laplacian
    QVector<QVector<double>> L = buildNormalizedLaplacian();

    // Step 3: Solve top-k eigenvectors
    solveTopKEigenvectors(L, m_numClusters);

    // Step 4: K-way discretization
    discretizeKWay();

    // Compute Ncut value
    m_stats.ncutValue = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (m_assignments[i].cluster != m_assignments[j].cluster)
                m_stats.ncutValue += m_affinity[i][j] * 0.5;

    m_stats.numPoints = n;
    m_stats.numDimensions = d;
    m_stats.numClusters = m_numClusters;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitCompleted(m_numClusters, m_stats.ncutValue, timer.elapsed());
    return true;
}

/* ---- Predict ---- */

int SpectralCluster12::predict(const QVector<double>& point) const
{
    if (m_centroids.isEmpty()) return -1;
    // Find nearest training point's cluster
    double bestDist = std::numeric_limits<double>::max();
    int bestC = 0;
    int k = m_numClusters;
    for (int c = 0; c < k; ++c) {
        double d = 0.0;
        int dim = qMin(point.size(), m_centroids[c].size());
        for (int i = 0; i < dim; ++i) {
            double diff = point[i] - m_centroids[c][i];
            d += diff * diff;
        }
        if (d < bestDist) { bestDist = d; bestC = c; }
    }
    return bestC;
}

/* ---- Accessors ---- */

QVector<SpectralCluster12::Assignment> SpectralCluster12::assignments() const { return m_assignments; }
QVector<QVector<double>> SpectralCluster12::eigenvectors() const { return m_eigVecs; }

/* ---- Reset ---- */

void SpectralCluster12::resetStatistics()
{
    m_data.clear(); m_affinity.clear(); m_eigVecs.clear();
    m_assignments.clear(); m_centroids.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
