/**
 * @file SpectralCluster13.cpp
 * @brief SpectralCluster13 实现
 *
 * 实现谱聚类：Shi-Malik归一化割与旋转对齐特征向量离散化。
 */

#include "utils/cluster248/SpectralCluster13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralCluster13::SpectralCluster13(QObject *parent) : QObject(parent) {}
SpectralCluster13::~SpectralCluster13() = default;

/* ---- Configuration ---- */

void SpectralCluster13::setNumClusters(int k) { m_numClusters = qMax(2, k); }
void SpectralCluster13::setKernelParams(double sigma, int knn)
{
    m_sigma = qMax(0.01, sigma);
    m_knn = qMax(1, knn);
}
void SpectralCluster13::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }

/* ---- Euclidean distance ---- */

double SpectralCluster13::distance(const QVector<double>& a,
                                    const QVector<double>& b) const
{
    double d = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int k = 0; k < dim; ++k) {
        double diff = a[k] - b[k];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Build RBF similarity matrix with kNN sparsification ---- */

void SpectralCluster13::buildSimilarityMatrix()
{
    int n = m_data.size();
    m_similarity.resize(n);
    double twoSigmaSq = 2.0 * m_sigma * m_sigma;

    // Compute full RBF similarities
    QVector<QVector<double>> fullSim(n);
    for (int i = 0; i < n; ++i) {
        fullSim[i].resize(n, 0.0);
        m_similarity[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j) {
            double d = distance(m_data[i], m_data[j]);
            fullSim[i][j] = qExp(-d * d / twoSigmaSq);
        }
    }

    // kNN sparsification: keep only k nearest neighbors + self
    for (int i = 0; i < n; ++i) {
        QVector<QPair<double, int>> dists;
        dists.reserve(n);
        for (int j = 0; j < n; ++j)
            dists.append({fullSim[i][j], j});
        std::sort(dists.begin(), dists.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        int keep = qMin(m_knn + 1, n);
        for (int k = 0; k < keep; ++k)
            m_similarity[i][dists[k].second] = fullSim[i][dists[k].second];
    }

    // Symmetrize: W[i][j] = max(W[i][j], W[j][i])
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double v = qMax(m_similarity[i][j], m_similarity[j][i]);
            m_similarity[i][j] = v;
            m_similarity[j][i] = v;
        }
}

/* ---- Solve generalized eigenproblem via power iteration ---- */

void SpectralCluster13::solveEigensystem()
{
    int n = m_data.size();
    int k = m_numClusters;

    // Degree matrix D and D^{-1/2}
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            d[i] += m_similarity[i][j];

    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i)
        dInvSqrt[i] = (d[i] > 1e-15) ? 1.0 / qSqrt(d[i]) : 0.0;

    // Normalized Laplacian: L_sym = I - D^{-1/2} W D^{-1/2}
    // Find k smallest eigenvectors via power method on (I - L_sym)
    m_eigvecs.resize(n);
    for (int i = 0; i < n; ++i)
        m_eigvecs[i].resize(k, 0.0);

    QVector<QVector<double>> basis(k);
    for (int ev = 0; ev < k; ++ev) {
        basis[ev].resize(n);
        // Initialize with random values
        for (int i = 0; i < n; ++i)
            basis[ev][i] = 0.01 * (i + 1) * (ev + 1);

        // Power iteration for L_sym eigenvectors
        for (int iter = 0; iter < m_maxIter; ++iter) {
            // Multiply by D^{-1/2} W D^{-1/2}
            QVector<double> y(n, 0.0);
            for (int i = 0; i < n; ++i) {
                if (dInvSqrt[i] < 1e-15) continue;
                for (int j = 0; j < n; ++j) {
                    if (dInvSqrt[j] < 1e-15) continue;
                    y[i] += dInvSqrt[i] * m_similarity[i][j] *
                            dInvSqrt[j] * basis[ev][j];
                }
            }

            // Deflation: remove projections onto previous eigenvectors
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += y[i] * basis[prev][i];
                for (int i = 0; i < n; ++i) y[i] -= dot * basis[prev][i];
            }

            // Normalize
            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += y[i] * y[i];
            norm = qSqrt(qMax(norm, 1e-15));
            for (int i = 0; i < n; ++i) basis[ev][i] = y[i] / norm;
        }

        for (int i = 0; i < n; ++i)
            m_eigvecs[i][ev] = basis[ev][i];
    }
}

/* ---- Normalize eigenvectors row-wise ---- */

void SpectralCluster13::normalizeEigenvectors()
{
    int n = m_eigvecs.size();
    for (int i = 0; i < n; ++i) {
        double norm = 0.0;
        for (double v : m_eigvecs[i]) norm += v * v;
        norm = qSqrt(qMax(norm, 1e-15));
        if (norm > 1e-15)
            for (auto& v : m_eigvecs[i]) v /= norm;
    }
}

/* ---- Rotation alignment for discretization ---- */

void SpectralCluster13::alignRotation(QVector<QVector<double>>& rotation)
{
    // Compute Z = T * R^T and find optimal discrete partition
    int n = m_eigvecs.size();
    int k = m_numClusters;

    QVector<int> labels(n, 0);
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Assign each point to nearest rotated centroid
        QVector<int> newLabels(n, 0);
        for (int i = 0; i < n; ++i) {
            double bestDot = -1e30;
            for (int c = 0; c < k; ++c) {
                double dot = 0.0;
                for (int j = 0; j < k; ++j)
                    dot += m_eigvecs[i][j] * rotation[j][c];
                if (dot > bestDot) { bestDot = dot; newLabels[i] = c; }
            }
        }

        // Check convergence
        bool changed = false;
        for (int i = 0; i < n; ++i)
            if (newLabels[i] != labels[i]) { changed = true; break; }
        labels = newLabels;
        if (!changed) break;

        // Update rotation from SVD of T^T * Z
        QVector<QVector<double>> TTZ(k, QVector<double>(k, 0.0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < k; ++j)
                for (int c = 0; c < k; ++c)
                    TTZ[j][c] += m_eigvecs[i][j] * (labels[i] == c ? 1.0 : 0.0);

        // Simple orthogonal approximation via Gram-Schmidt
        for (int c = 0; c < k; ++c) {
            for (int prev = 0; prev < c; ++prev) {
                double dot = 0.0;
                for (int j = 0; j < k; ++j) dot += TTZ[j][c] * rotation[j][prev];
                for (int j = 0; j < k; ++j) TTZ[j][c] -= dot * rotation[j][prev];
            }
            double norm = 0.0;
            for (int j = 0; j < k; ++j) norm += TTZ[j][c] * TTZ[j][c];
            norm = qSqrt(qMax(norm, 1e-15));
            for (int j = 0; j < k; ++j) rotation[j][c] = TTZ[j][c] / norm;
        }
    }
    m_labels = labels;
}

/* ---- Discretize eigenvectors via rotation alignment ---- */

void SpectralCluster13::discretizeRotationAlignment()
{
    int k = m_numClusters;
    // Initialize rotation as identity
    QVector<QVector<double>> rotation(k, QVector<double>(k, 0.0));
    for (int i = 0; i < k; ++i) rotation[i][i] = 1.0;
    alignRotation(rotation);
}

/* ---- Compute normalized cut value ---- */

double SpectralCluster13::ncutValue() const
{
    int n = m_labels.size();
    if (n == 0) return 0.0;
    int k = m_numClusters;

    // Compute degree per cluster and inter-cluster weights
    QVector<double> assoc(k, 0.0);
    QVector<double> cut(k, 0.0);
    for (int i = 0; i < n; ++i) {
        int ci = m_labels[i];
        for (int j = 0; j < n; ++j) {
            assoc[ci] += m_similarity[i][j];
            if (m_labels[j] != ci) cut[ci] += m_similarity[i][j];
        }
    }

    double ncut = 0.0;
    for (int c = 0; c < k; ++c)
        if (assoc[c] > 1e-15) ncut += cut[c] / assoc[c];
    return ncut / 2.0;
}

/* ---- Main fit ---- */

QVector<int> SpectralCluster13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};
    m_data = data;

    buildSimilarityMatrix();
    solveEigensystem();
    normalizeEigenvectors();
    discretizeRotationAlignment();

    m_ncutValue = ncutValue();

    m_stats.numSamples = n;
    m_stats.numDimensions = data[0].size();
    m_stats.numClusters = m_numClusters;
    m_stats.ncutValue = m_ncutValue;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_numClusters, m_ncutValue, timer.elapsed());
    return m_labels;
}

/* ---- Get eigenvectors ---- */

QVector<QVector<double>> SpectralCluster13::eigenvectors() const { return m_eigvecs; }

/* ---- Reset ---- */

void SpectralCluster13::resetStatistics()
{
    m_data.clear();
    m_similarity.clear();
    m_eigvecs.clear();
    m_labels.clear();
    m_ncutValue = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
