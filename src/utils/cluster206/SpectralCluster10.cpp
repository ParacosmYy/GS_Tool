/**
 * @file SpectralCluster10.cpp
 * @brief SpectralCluster10 实现
 *
 * 实现谱聚类：归一化拉普拉斯、幂迭代特征分解、特征向量旋转自动K选择。
 */

#include "utils/cluster206/SpectralCluster10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralCluster10::SpectralCluster10(QObject *parent) : QObject(parent) {}
SpectralCluster10::~SpectralCluster10() = default;

/* ---- Configuration ---- */

void SpectralCluster10::setMaxClusters(int maxK) { m_maxK = qMax(2, maxK); }
void SpectralCluster10::setSigma(double sigma) { m_sigma = qMax(0.01, sigma); }
void SpectralCluster10::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }

/* ---- Gaussian affinity ---- */

double SpectralCluster10::gaussianAffinity(const QVector<double>& a,
                                             const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qExp(-sum / (2.0 * m_sigma * m_sigma));
}

/* ---- Build affinity matrix ---- */

void SpectralCluster10::buildAffinityMatrix(const QVector<QVector<double>>& data)
{
    m_data = data;
    m_n = data.size();
    if (m_n == 0) return;
    m_dim = data[0].size();

    m_affinity.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_affinity[i].resize(m_n, 0.0);
        m_affinity[i][i] = 0.0; // No self-loops
        for (int j = i + 1; j < m_n; ++j) {
            double w = gaussianAffinity(data[i], data[j]);
            m_affinity[i][j] = w;
            m_affinity[j][i] = w;
        }
    }
}

/* ---- Normalize Laplacian: D^{-1/2} W D^{-1/2} ---- */

void SpectralCluster10::normalizeLaplacian()
{
    // Compute degree vector
    QVector<double> degree(m_n, 0.0);
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            degree[i] += m_affinity[i][j];

    // D^{-1/2} W D^{-1/2}
    for (int i = 0; i < m_n; ++i) {
        double di = qSqrt(qMax(degree[i], 1e-15));
        for (int j = 0; j < m_n; ++j) {
            double dj = qSqrt(qMax(degree[j], 1e-15));
            m_affinity[i][j] /= (di * dj);
        }
    }

    // Convert to Laplacian: I - normalized_W
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            m_affinity[i][j] = -m_affinity[i][j];
        }
        m_affinity[i][i] += 1.0;
    }
}

/* ---- Power iteration for top eigenvectors ---- */

void SpectralCluster10::powerIteration(int numEigens)
{
    numEigens = qMin(numEigens, m_n);
    m_eigenvectors.resize(numEigens);
    m_eigenvalues.resize(numEigens, 0.0);

    // Use deflation to find top eigenvectors of the normalized Laplacian
    // We want smallest eigenvalues, so we subtract from identity (inverse power)
    for (int ev = 0; ev < numEigens; ++ev) {
        QVector<double> v(m_n);
        for (int i = 0; i < m_n; ++i)
            v[i] = static_cast<double>(qrand()) / RAND_MAX - 0.5;

        double norm = 0.0;
        for (double x : v) norm += x * x;
        norm = qSqrt(qMax(norm, 1e-15));
        for (double& x : v) x /= norm;

        for (int iter = 0; iter < m_maxIter; ++iter) {
            // Matrix-vector product: L * v
            QVector<double> w(m_n, 0.0);
            for (int i = 0; i < m_n; ++i)
                for (int j = 0; j < m_n; ++j)
                    w[i] += m_affinity[i][j] * v[j];

            // Deflate against previous eigenvectors
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < m_n; ++i)
                    dot += w[i] * m_eigenvectors[prev][i];
                for (int i = 0; i < m_n; ++i)
                    w[i] -= dot * m_eigenvectors[prev][i];
            }

            // Rayleigh quotient
            double lambda = 0.0;
            norm = 0.0;
            for (int i = 0; i < m_n; ++i) {
                lambda += v[i] * w[i];
                norm += w[i] * w[i];
            }
            norm = qSqrt(qMax(norm, 1e-15));

            for (int i = 0; i < m_n; ++i)
                v[i] = w[i] / norm;

            m_eigenvalues[ev] = lambda;
            m_stats.eigenIterations = iter;
        }

        m_eigenvectors[ev] = v;
    }
}

/* ---- Compute eigenvectors ---- */

void SpectralCluster10::computeEigenvectors(int numEigens)
{
    normalizeLaplacian();
    powerIteration(numEigens);
}

/* ---- Rotation cost for auto-K ---- */

double SpectralCluster10::rotationCost(int k) const
{
    // Rotate eigenvector matrix and measure clustering quality
    // Lower cost = better separated clusters
    int n = m_n;
    if (n == 0 || k <= 0) return 1e10;

    double cost = 0.0;
    for (int i = 0; i < n; ++i) {
        double minDist = 1e10;
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double dist = 0.0;
            for (int ev = 0; ev < qMin(k, m_eigenvectors.size()); ++ev) {
                double d = m_eigenvectors[ev][i] - m_eigenvectors[ev][j];
                dist += d * d;
            }
            minDist = qMin(minDist, dist);
        }
        cost += minDist;
    }
    return cost / n;
}

/* ---- Auto-select K ---- */

int SpectralCluster10::autoSelectK() const
{
    int bestK = 2;
    double bestCost = std::numeric_limits<double>::max();
    for (int k = 2; k <= m_maxK; ++k) {
        double c = rotationCost(k);
        if (c < bestCost) {
            bestCost = c;
            bestK = k;
        }
    }
    return bestK;
}

/* ---- K-means on eigenvector rows ---- */

void SpectralCluster10::kMeansOnEigen(int k)
{
    int n = m_n;
    int d = m_eigenvectors.size();
    m_labels.resize(n, 0);

    // Initialize centers from first k points
    QVector<QVector<double>> centers(k);
    for (int j = 0; j < k; ++j) {
        centers[j].resize(d);
        for (int p = 0; p < d; ++p)
            centers[j][p] = m_eigenvectors[p][j % n];
    }

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Assign
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e10;
            int bestJ = 0;
            for (int j = 0; j < k; ++j) {
                double dist = 0.0;
                for (int p = 0; p < d; ++p) {
                    double diff = m_eigenvectors[p][i] - centers[j][p];
                    dist += diff * diff;
                }
                if (dist < bestDist) { bestDist = dist; bestJ = j; }
            }
            if (m_labels[i] != bestJ) { m_labels[i] = bestJ; changed = true; }
        }
        if (!changed) break;

        // Update centers
        QVector<double> counts(k, 0.0);
        for (auto& c : centers) std::fill(c.begin(), c.end(), 0.0);
        for (int i = 0; i < n; ++i) {
            counts[m_labels[i]] += 1.0;
            for (int p = 0; p < d; ++p)
                centers[m_labels[i]][p] += m_eigenvectors[p][i];
        }
        for (int j = 0; j < k; ++j) {
            if (counts[j] > 1e-12)
                for (int p = 0; p < d; ++p)
                    centers[j][p] /= counts[j];
        }
    }
}

/* ---- Fit ---- */

void SpectralCluster10::fit(const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    buildAffinityMatrix(data);
    if (m_n == 0) return;

    int useK = (k <= 0) ? autoSelectK() : k;
    computeEigenvectors(useK);
    kMeansOnEigen(useK);

    m_stats.numPoints = m_n;
    m_stats.numClusters = useK;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(useK, m_stats.eigenIterations, timer.elapsed());
}

/* ---- Compute normalized cut ---- */

double SpectralCluster10::computeNcut() const
{
    if (m_n == 0) return 0.0;
    int k = m_stats.numClusters;
    double ncut = 0.0;

    for (int c = 0; c < k; ++c) {
        double cut = 0.0, assoc = 0.0;
        for (int i = 0; i < m_n; ++i) {
            for (int j = 0; j < m_n; ++j) {
                double w = (i < m_affinity.size() && j < m_affinity[i].size())
                           ? m_affinity[i][j] : 0.0;
                if (m_labels[i] == c) assoc += w;
                if (m_labels[i] == c && m_labels[j] != c) cut += w;
            }
        }
        if (assoc > 1e-15) ncut += cut / assoc;
    }
    return ncut;
}

/* ---- Getters ---- */

QVector<int> SpectralCluster10::getLabels() const { return m_labels; }
QVector<QVector<double>> SpectralCluster10::getEigenvectors() const { return m_eigenvectors; }

/* ---- Reset ---- */

void SpectralCluster10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_affinity.clear();
    m_eigenvectors.clear();
    m_eigenvalues.clear();
    m_labels.clear();
    m_data.clear();
}
