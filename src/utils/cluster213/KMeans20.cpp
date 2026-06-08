/**
 * @file KMeans20.cpp
 * @brief KMeans20 实现
 *
 * 实现核K均值聚类：RBF核距离计算、核k-means++初始化、迭代分配。
 */

#include "utils/cluster213/KMeans20.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

KMeans20::KMeans20(QObject *parent) : QObject(parent) {}
KMeans20::~KMeans20() = default;

/* ---- Configuration ---- */

void KMeans20::setParameters(int k, double sigma, int maxIter, double tol)
{
    m_k = qMax(2, k);
    m_sigma = qMax(1e-6, sigma);
    m_maxIter = qMax(1, maxIter);
    m_tol = qMax(1e-12, tol);
    m_stats.sigma = m_sigma;
}

/* ---- RBF kernel ---- */

double KMeans20::rbfKernel(const QVector<double>& a,
                            const QVector<double>& b) const
{
    double dist2 = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i)
        dist2 += qPow(a[i] - b[i], 2);
    double gamma = 1.0 / (2.0 * m_sigma * m_sigma);
    return qExp(-gamma * dist2);
}

/* ---- Precompute kernel matrix ---- */

void KMeans20::computeKernelMatrix()
{
    m_kernelMatrix.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_kernelMatrix[i].resize(m_n, 0.0);
        for (int j = 0; j <= i; ++j) {
            double k = rbfKernel(m_data[i], m_data[j]);
            m_kernelMatrix[i][j] = k;
            m_kernelMatrix[j][i] = k;
        }
    }
}

/* ---- Kernel k-means++ initialization ---- */

void KMeans20::kernelInit()
{
    // First centroid chosen uniformly at random
    QVector<int> centroids;
    centroids.append(std::rand() % m_n);

    for (int c = 1; c < m_k; ++c) {
        QVector<double> dists(m_n, 0.0);
        double total = 0.0;

        for (int i = 0; i < m_n; ++i) {
            // Min kernel distance to any existing centroid
            double minD = std::numeric_limits<double>::max();
            for (int cent : centroids) {
                // ||phi(x_i) - phi(x_c)||^2 = K(ii) - 2K(ic) + K(cc)
                double d = m_kernelMatrix[i][i]
                         - 2.0 * m_kernelMatrix[i][cent]
                         + m_kernelMatrix[cent][cent];
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            total += minD;
        }

        // Weighted random selection
        double r = (std::rand() / static_cast<double>(RAND_MAX)) * total;
        double cum = 0.0;
        int chosen = std::rand() % m_n;
        for (int i = 0; i < m_n; ++i) {
            cum += dists[i];
            if (cum >= r) { chosen = i; break; }
        }
        centroids.append(chosen);
    }

    // Initial assignment based on kernel distance to centroids
    m_labels.resize(m_n, 0);
    for (int i = 0; i < m_n; ++i) {
        double minD = std::numeric_limits<double>::max();
        for (int c = 0; c < m_k; ++c) {
            double d = m_kernelMatrix[i][i]
                     - 2.0 * m_kernelMatrix[i][centroids[c]]
                     + m_kernelMatrix[centroids[c]][centroids[c]];
            if (d < minD) { minD = d; m_labels[i] = c; }
        }
    }
}

/* ---- Kernel distance from sample to cluster ---- */

double KMeans20::kernelDistance(const QVector<double>& sample, int cluster) const
{
    double kxx = 0.0;
    // Compute K(sample, sample)
    double dist2 = 0.0;
    for (int d = 0; d < sample.size(); ++d) dist2 += sample[d] * sample[d];
    double gamma = 1.0 / (2.0 * m_sigma * m_sigma);
    kxx = qExp(-gamma * dist2);

    // Sum of K(sample, x_j) for x_j in cluster
    double sumKxj = 0.0;
    for (int j = 0; j < m_n; ++j) {
        if (m_labels[j] != cluster) continue;
        double d2 = 0.0;
        for (int dim = 0; dim < m_dim; ++dim)
            d2 += qPow(sample[dim] - m_data[j][dim], 2);
        sumKxj += qExp(-gamma * d2);
    }

    double nc = qMax(1.0, m_clusterSizes[cluster]);
    // ||phi(x) - mu_c||^2 = K(x,x) - 2/n_c * sum K(x,x_j) + 1/n_c^2 * sum_{j,l} K(x_j,x_l)
    double sumCluster = 0.0;
    for (int j = 0; j < m_n; ++j) {
        if (m_labels[j] != cluster) continue;
        for (int l = 0; l < m_n; ++l) {
            if (m_labels[l] != cluster) continue;
            sumCluster += m_kernelMatrix[j][l];
        }
    }
    return kxx - 2.0 / nc * sumKxj + sumCluster / (nc * nc);
}

/* ---- Assign clusters ---- */

void KMeans20::assignClusters()
{
    // Update cluster sizes
    m_clusterSizes.resize(m_k, 0.0);
    for (int c = 0; c < m_k; ++c) m_clusterSizes[c] = 0.0;
    for (int i = 0; i < m_n; ++i) m_clusterSizes[m_labels[i]] += 1.0;

    // Compute intra-cluster kernel sums
    QVector<double> clusterKernelSum(m_k, 0.0);
    for (int j = 0; j < m_n; ++j) {
        for (int l = 0; l < m_n; ++l) {
            if (m_labels[j] == m_labels[l])
                clusterKernelSum[m_labels[j]] += m_kernelMatrix[j][l];
        }
    }

    // Reassign each point
    for (int i = 0; i < m_n; ++i) {
        double minD = std::numeric_limits<double>::max();
        int best = 0;
        for (int c = 0; c < m_k; ++c) {
            double nc = qMax(1.0, m_clusterSizes[c]);
            double d = m_kernelMatrix[i][i]
                     - 2.0 / nc * clusterKernelSum[c]  // approx
                     + clusterKernelSum[c] / (nc * nc);
            // Corrected: use K(x_i, cluster_members)
            double sumKic = 0.0;
            for (int j = 0; j < m_n; ++j) {
                if (m_labels[j] == c) sumKic += m_kernelMatrix[i][j];
            }
            d = m_kernelMatrix[i][i] - 2.0 / nc * sumKic
              + clusterKernelSum[c] / (nc * nc);
            if (d < minD) { minD = d; best = c; }
        }
        m_labels[i] = best;
    }
}

/* ---- Compute inertia ---- */

double KMeans20::computeInertia() const
{
    double inertia = 0.0;
    for (int i = 0; i < m_n; ++i) {
        inertia += kernelDistance(m_data[i], m_labels[i]);
    }
    return inertia;
}

/* ---- Fit ---- */

void KMeans20::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_data = data;
    m_n = data.size();
    if (m_n < m_k) return;
    m_dim = data[0].size();

    computeKernelMatrix();
    kernelInit();

    double prevInertia = std::numeric_limits<double>::max();
    int iter = 0;

    for (iter = 0; iter < m_maxIter; ++iter) {
        assignClusters();
        double curInertia = computeInertia();
        if (qAbs(prevInertia - curInertia) < m_tol) break;
        prevInertia = curInertia;
    }

    m_stats.numSamples = m_n;
    m_stats.numClusters = m_k;
    m_stats.dimensions = m_dim;
    m_stats.iterations = iter;
    m_stats.inertia = computeInertia();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fittingCompleted(m_k, iter, m_stats.inertia, timer.elapsed());
}

/* ---- Predict ---- */

int KMeans20::predict(const QVector<double>& sample) const
{
    double minD = std::numeric_limits<double>::max();
    int best = 0;
    for (int c = 0; c < m_k; ++c) {
        double d = kernelDistance(sample, c);
        if (d < minD) { minD = d; best = c; }
    }
    return best;
}

/* ---- Labels ---- */

QVector<int> KMeans20::labels() const { return m_labels; }

/* ---- Inertia ---- */

double KMeans20::inertia() const { return m_stats.inertia; }

/* ---- Reset ---- */

void KMeans20::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_data.clear();
    m_kernelMatrix.clear();
    m_labels.clear();
    m_clusterSizes.clear();
    m_n = 0;
    m_dim = 0;
}
