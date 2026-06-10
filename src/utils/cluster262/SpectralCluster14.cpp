/**
 * @file SpectralCluster14.cpp
 * @brief SpectralCluster14 实现
 *
 * 实现谱聚类：Shi-Malik归一化割特征向量二分图分割。
 */

#include "utils/cluster262/SpectralCluster14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralCluster14::SpectralCluster14(QObject *parent)
    : QObject(parent) {}

SpectralCluster14::~SpectralCluster14() = default;

/* ---- Configuration ---- */

void SpectralCluster14::setParameters(int numClusters, double sigma, int maxIterations)
{
    m_K = qMax(2, numClusters);
    m_sigma = qMax(0.01, sigma);
    m_maxIter = qMax(1, maxIterations);
}

/* ---- Matrix-vector multiply ---- */

QVector<double> SpectralCluster14::matVec(const QVector<QVector<double>>& M,
                                           const QVector<double>& v) const
{
    int n = M.size();
    QVector<double> r(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            r[i] += M[i][j] * v[j];
    return r;
}

double SpectralCluster14::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    for (int i = 0; i < a.size(); ++i) s += a[i] * b[i];
    return s;
}

void SpectralCluster14::normalizeVec(QVector<double>& v) const
{
    double norm = qSqrt(dot(v, v));
    if (norm > 1e-15)
        for (int i = 0; i < v.size(); ++i) v[i] /= norm;
}

/* ---- Build RBF affinity matrix ---- */

void SpectralCluster14::buildAffinity(const QVector<QVector<double>>& data)
{
    m_n = data.size();
    m_affinity.resize(m_n);
    double sig2 = 2.0 * m_sigma * m_sigma;
    for (int i = 0; i < m_n; ++i) {
        m_affinity[i].resize(m_n, 0.0);
        for (int j = 0; j < m_n; ++j) {
            if (i == j) continue;
            double dist2 = 0.0;
            for (int d = 0; d < data[i].size(); ++d)
                dist2 += (data[i][d] - data[j][d]) * (data[i][d] - data[j][d]);
            m_affinity[i][j] = qExp(-dist2 / sig2);
        }
    }
}

/* ---- Normalized Laplacian ---- */

void SpectralCluster14::computeNormalizedLaplacian(QVector<QVector<double>>& L) const
{
    int n = m_n;
    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double deg = 0.0;
        for (int j = 0; j < n; ++j) deg += m_affinity[i][j];
        dInvSqrt[i] = (deg > 1e-15) ? 1.0 / qSqrt(deg) : 0.0;
    }
    // L_norm = I - D^{-1/2} W D^{-1/2}
    L.resize(n);
    for (int i = 0; i < n; ++i) {
        L[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j)
            L[i][j] = -dInvSqrt[i] * m_affinity[i][j] * dInvSqrt[j];
        L[i][i] += 1.0;
    }
}

/* ---- Eigenvectors via inverse power iteration with deflation ---- */

void SpectralCluster14::computeEigenvectors(const QVector<QVector<double>>& L, int k)
{
    int n = m_n;
    m_eigvecs.resize(k);
    for (int ev = 0; ev < k; ++ev) {
        m_eigvecs[ev].resize(n);
        // Random init
        QVector<double> v(n);
        for (int i = 0; i < n; ++i)
            v[i] = static_cast<double>(qrand()) / RAND_MAX - 0.5;

        // Deflate: remove projection on previous eigenvectors
        for (int prev = 0; prev < ev; ++prev) {
            double proj = dot(v, m_eigvecs[prev]);
            for (int i = 0; i < n; ++i)
                v[i] -= proj * m_eigvecs[prev][i];
        }
        normalizeVec(v);

        // Power iteration on L (smallest eigenvectors => iterate on L directly
        // since L's smallest eigenvalue ~0 gives the Fiedler vector)
        for (int iter = 0; iter < 200; ++iter) {
            QVector<double> Lv = matVec(L, v);
            // Deflate
            for (int prev = 0; prev < ev; ++prev) {
                double proj = dot(Lv, m_eigvecs[prev]);
                for (int i = 0; i < n; ++i)
                    Lv[i] -= proj * m_eigvecs[prev][i];
            }
            normalizeVec(Lv);
            // Convergence check
            double change = 0.0;
            for (int i = 0; i < n; ++i)
                change += (Lv[i] - v[i]) * (Lv[i] - v[i]);
            v = Lv;
            if (change < 1e-12) break;
        }
        m_eigvecs[ev] = v;
    }
}

/* ---- K-means on eigenvector embedding ---- */

void SpectralCluster14::kMeansOnEmbedding()
{
    int n = m_n;
    int k = qMin(m_K, n);
    // Transpose: rows become embedding vectors
    QVector<QVector<double>> embed(n);
    for (int i = 0; i < n; ++i) {
        embed[i].resize(k);
        for (int j = 0; j < k; ++j)
            embed[i][j] = m_eigvecs[j][i];
        // Normalize row
        double norm = qSqrt(dot(embed[i], embed[i]));
        if (norm > 1e-15)
            for (int j = 0; j < k; ++j) embed[i][j] /= norm;
    }

    // Init centroids from first K points
    QVector<QVector<double>> centers(k);
    for (int j = 0; j < k; ++j) centers[j] = embed[j];

    m_labels.resize(n);
    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;
        // Assign
        for (int i = 0; i < n; ++i) {
            int best = 0;
            double bestDist = std::numeric_limits<double>::max();
            for (int j = 0; j < k; ++j) {
                double d = 0.0;
                for (int d2 = 0; d2 < k; ++d2) {
                    double diff = embed[i][d2] - centers[j][d2];
                    d += diff * diff;
                }
                if (d < bestDist) { bestDist = d; best = j; }
            }
            if (m_labels[i] != best) { m_labels[i] = best; changed = true; }
        }
        if (!changed) break;
        // Update centroids
        QVector<int> counts(k, 0);
        for (auto& c : centers) c.fill(0.0);
        for (int i = 0; i < n; ++i) {
            counts[m_labels[i]]++;
            for (int d = 0; d < k; ++d)
                centers[m_labels[i]][d] += embed[i][d];
        }
        for (int j = 0; j < k; ++j)
            if (counts[j] > 0)
                for (int d = 0; d < k; ++d)
                    centers[j][d] /= counts[j];
    }
}

/* ---- Compute Ncut ---- */

double SpectralCluster14::computeNcut() const
{
    double ncut = 0.0;
    for (int c = 0; c < m_K; ++c) {
        double cut = 0.0, assoc = 0.0;
        for (int i = 0; i < m_n; ++i) {
            if (m_labels[i] != c) continue;
            for (int j = 0; j < m_n; ++j) {
                assoc += m_affinity[i][j];
                if (m_labels[j] != c) cut += m_affinity[i][j];
            }
        }
        if (assoc > 1e-15) ncut += cut / assoc;
    }
    return ncut;
}

/* ---- Fit from data ---- */

bool SpectralCluster14::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    if (data.size() < m_K) return false;
    m_n = data.size();
    buildAffinity(data);
    return fitFromAffinity(m_affinity);
}

/* ---- Fit from precomputed affinity ---- */

bool SpectralCluster14::fitFromAffinity(const QVector<QVector<double>>& affinity)
{
    QElapsedTimer timer;
    timer.start();
    m_affinity = affinity;
    m_n = affinity.size();
    if (m_n < m_K) return false;

    QVector<QVector<double>> L;
    computeNormalizedLaplacian(L);
    int k = qMin(m_K, m_n);
    computeEigenvectors(L, k);
    kMeansOnEmbedding();

    double ncut = computeNcut();
    double elapsed = timer.elapsed();

    m_stats.numPoints = m_n;
    m_stats.numClusters = m_K;
    m_stats.numEigenUsed = k;
    m_stats.ncutValue = ncut;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringUpdated(m_K, ncut, elapsed);
    return true;
}

/* ---- Accessors ---- */

QVector<int> SpectralCluster14::labels() const { return m_labels; }
QVector<QVector<double>> SpectralCluster14::eigenvectors() const { return m_eigvecs; }
double SpectralCluster14::ncutValue() const { return m_stats.ncutValue; }

/* ---- Reset ---- */

void SpectralCluster14::resetStatistics()
{
    m_affinity.clear();
    m_eigvecs.clear();
    m_labels.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
