/**
 * @file KMeans26.cpp
 * @brief KMeans26 实现
 *
 * 实现核K均值：核化距离与迭代主化非线性可分聚类。
 */

#include "utils/cluster255/KMeans26.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

KMeans26::KMeans26(QObject *parent)
    : QObject(parent) {}
KMeans26::~KMeans26() = default;

/* ---- Configuration ---- */

void KMeans26::setClusters(int k) { m_k = qMax(2, k); }
void KMeans26::setKernelType(KernelType type) { m_kernel = type; }
void KMeans26::setSigma(double sigma) { m_sigma = qMax(0.01, sigma); }
void KMeans26::setPolyDegree(int degree) { m_polyDegree = qMax(1, degree); }
void KMeans26::setMaxIterations(int iters) { m_maxIter = qMax(5, iters); }

/* ---- Kernel value computation ---- */

double KMeans26::kernelValue(const QVector<double>& a,
                              const QVector<double>& b) const
{
    switch (m_kernel) {
    case KernelType::RBF: {
        double sqDist = 0.0;
        for (int d = 0; d < m_dims; ++d) {
            double diff = a[d] - b[d];
            sqDist += diff * diff;
        }
        return qExp(-sqDist / (2.0 * m_sigma * m_sigma));
    }
    case KernelType::Polynomial: {
        double dot = 0.0;
        for (int d = 0; d < m_dims; ++d)
            dot += a[d] * b[d];
        return qPow(dot + 1.0, m_polyDegree);
    }
    case KernelType::Linear: {
        double dot = 0.0;
        for (int d = 0; d < m_dims; ++d)
            dot += a[d] * b[d];
        return dot;
    }
    }
    return 0.0;
}

/* ---- Build kernel matrix ---- */

void KMeans26::buildKernelMatrix()
{
    m_kernelMatrix.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_kernelMatrix[i].resize(m_n);
        for (int j = 0; j < m_n; ++j)
            m_kernelMatrix[i][j] = kernelValue(m_data[i], m_data[j]);
    }
}

/* ---- Kernel distance from point to cluster ---- */

double KMeans26::kernelDistance(int pointIdx, int cluster) const
{
    // ||phi(x) - mu_c||^2 = K(x,x) - 2/|C|*sum(K(x,xi)) + 1/|C|^2*sum sum K(xi,xj)
    double kxx = m_kernelMatrix[pointIdx][pointIdx];

    // Collect cluster members
    QVector<int> members;
    for (int i = 0; i < m_n; ++i)
        if (m_labels[i] == cluster) members.append(i);

    int nc = members.size();
    if (nc == 0) return std::numeric_limits<double>::max();

    // Average kernel from point to cluster
    double avgKx = 0.0;
    for (int m : members)
        avgKx += m_kernelMatrix[pointIdx][m];
    avgKx /= nc;

    // Average pairwise kernel within cluster
    double avgKK = 0.0;
    for (int i = 0; i < nc; ++i)
        for (int j = 0; j < nc; ++j)
            avgKK += m_kernelMatrix[members[i]][members[j]];
    avgKK /= (nc * nc);

    return kxx - 2.0 * avgKx + avgKK;
}

/* ---- Initialize via kernel k-means++ ---- */

void KMeans26::initialize()
{
    m_labels.resize(m_n);
    std::fill(m_labels.begin(), m_labels.end(), 0);

    // Pick first center randomly
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, m_n - 1);
    QVector<int> centers;
    centers.append(uni(rng));

    for (int c = 1; c < m_k; ++c) {
        // Compute min kernel distance to existing centers
        QVector<double> minDist(m_n, std::numeric_limits<double>::max());
        double totalDist = 0.0;
        for (int i = 0; i < m_n; ++i) {
            for (int cen : centers) {
                double d = m_kernelMatrix[i][i] - 2.0 * m_kernelMatrix[i][cen]
                           + m_kernelMatrix[cen][cen];
                minDist[i] = qMin(minDist[i], qMax(0.0, d));
            }
            totalDist += minDist[i];
        }
        // Weighted random selection
        std::uniform_real_distribution<double> rDist(0.0, totalDist);
        double threshold = rDist(rng);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < m_n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        centers.append(chosen);
    }

    // Assign each point to nearest center using kernel distance
    for (int i = 0; i < m_n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int c = 0; c < m_k; ++c) {
            double d = m_kernelMatrix[i][i] - 2.0 * m_kernelMatrix[i][centers[c]]
                       + m_kernelMatrix[centers[c]][centers[c]];
            if (d < bestDist) { bestDist = d; m_labels[i] = c; }
        }
    }
}

/* ---- Compute objective cost ---- */

double KMeans26::computeCost() const
{
    double cost = 0.0;
    for (int i = 0; i < m_n; ++i)
        cost += kernelDistance(i, m_labels[i]);
    return cost;
}

/* ---- Main fit: kernel k-means with iterative majorization ---- */

bool KMeans26::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_k) return false;
    m_data = data;
    m_n = data.size();
    m_dims = data[0].size();

    buildKernelMatrix();
    initialize();

    // Iterative majorization: reassign until convergence
    int totalIters = 0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        int changed = 0;
        for (int i = 0; i < m_n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestCluster = m_labels[i];
            for (int c = 0; c < m_k; ++c) {
                double d = kernelDistance(i, c);
                if (d < bestDist) { bestDist = d; bestCluster = c; }
            }
            if (bestCluster != m_labels[i]) {
                m_labels[i] = bestCluster;
                changed++;
            }
        }
        totalIters++;
        if (changed == 0) break;
    }

    // Compute centroids in input space for reference
    m_centroids.resize(m_k);
    for (int c = 0; c < m_k; ++c) {
        m_centroids[c].resize(m_dims, 0.0);
        int count = 0;
        for (int i = 0; i < m_n; ++i) {
            if (m_labels[i] == c) {
                for (int d = 0; d < m_dims; ++d)
                    m_centroids[c][d] += m_data[i][d];
                count++;
            }
        }
        if (count > 0) {
            for (int d = 0; d < m_dims; ++d)
                m_centroids[c][d] /= count;
        }
    }

    double cost = computeCost();
    double elapsed = timer.elapsed();

    m_stats.numClusters = m_k;
    m_stats.numPoints = m_n;
    m_stats.numIterations = totalIters;
    m_stats.finalCost = cost;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fittingCompleted(m_k, cost, elapsed);
    return true;
}

/* ---- Predict cluster for new sample ---- */

int KMeans26::predict(const QVector<double>& sample) const
{
    if (m_centroids.isEmpty()) return 0;
    int best = 0;
    double bestDist = std::numeric_limits<double>::max();
    for (int c = 0; c < m_k; ++c) {
        double d = 0.0;
        for (int dim = 0; dim < m_dims; ++dim) {
            double diff = sample[dim] - m_centroids[c][dim];
            d += diff * diff;
        }
        if (d < bestDist) { bestDist = d; best = c; }
    }
    return best;
}

/* ---- Get labels ---- */

QVector<int> KMeans26::labels() const { return m_labels; }

/* ---- Get centroids ---- */

QVector<QVector<double>> KMeans26::centroids() const { return m_centroids; }

/* ---- Reset ---- */

void KMeans26::resetStatistics()
{
    m_data.clear();
    m_labels.clear();
    m_centroids.clear();
    m_kernelMatrix.clear();
    m_n = 0;
    m_dims = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
