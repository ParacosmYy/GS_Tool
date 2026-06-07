/**
 * @file KMeans18.cpp
 * @brief KMeans18 实现
 *
 * 实现核K均值：Nyström低秩近似、Gram矩阵、核距离计算。
 */

#include "utils/cluster193/KMeans18.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMeans18::KMeans18(QObject *parent) : QObject(parent) {}
KMeans18::~KMeans18() = default;

/* ---- Configuration ---- */

void KMeans18::setClusters(int k) { m_k = qMax(1, k); }
void KMeans18::setKernel(Kernel kernel) { m_kernel = kernel; }
void KMeans18::setKernelParam(double sigma) { m_sigma = qMax(0.001, sigma); }
void KMeans18::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void KMeans18::setLandmarkRatio(double ratio) { m_landmarkRatio = qBound(0.01, ratio, 0.5); }

/* ---- Kernel value ---- */

double KMeans18::kernelValue(const QVector<double>& a,
                              const QVector<double>& b) const
{
    int d = qMin(a.size(), b.size());
    double sumSq = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sumSq += diff * diff;
    }
    switch (m_kernel) {
    case RBF:
        return qExp(-sumSq / (2.0 * m_sigma * m_sigma));
    case Polynomial:
        return qPow(1.0 + sumSq / m_sigma, 3.0);
    case Linear:
    default:
        return 1.0 - sumSq / (m_sigma * m_sigma + 1.0);
    }
}

/* ---- Select landmarks via k-means++ ---- */

QVector<int> KMeans18::selectLandmarks(int m) const
{
    int n = m_data.size();
    m = qMin(m, n);
    QVector<int> landmarks;
    landmarks.reserve(m);

    // First landmark: random (pick index 0)
    landmarks.append(0);

    QVector<double> minDist(n, std::numeric_limits<double>::max());
    for (int i = 0; i < n; ++i)
        minDist[i] = 1.0 - kernelValue(m_data[i], m_data[0]);

    for (int k = 1; k < m; ++k) {
        // Weighted sampling by distance
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) totalDist += minDist[i];

        double threshold = qrand() / static_cast<double>(RAND_MAX) * totalDist;
        double cumSum = 0.0;
        int chosen = n - 1;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        landmarks.append(chosen);

        for (int i = 0; i < n; ++i) {
            double d = 1.0 - kernelValue(m_data[i], m_data[chosen]);
            minDist[i] = qMin(minDist[i], d);
        }
    }
    return landmarks;
}

/* ---- Nyström approximation ---- */

QVector<QVector<double>> KMeans18::nystromApprox(
    const QVector<int>& landmarks) const
{
    int m = landmarks.size();
    int n = m_data.size();

    // K_nm: n x m cross-kernel
    QVector<QVector<double>> Knm(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            Knm[i][j] = kernelValue(m_data[i], m_data[landmarks[j]]);

    // K_mm: m x m (landmark self-kernel), regularize
    QVector<QVector<double>> Kmm(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j) {
            Kmm[i][j] = kernelValue(m_data[landmarks[i]], m_data[landmarks[j]]);
            if (i == j) Kmm[i][j] += 1e-6;
        }

    // Invert K_mm via Cholesky
    // L * L^T = K_mm
    QVector<QVector<double>> L(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j <= i; ++j) {
            double s = Kmm[i][j];
            for (int k = 0; k < j; ++k) s -= L[i][k] * L[j][k];
            L[i][j] = (i == j) ? qSqrt(qMax(s, 1e-15)) : s / qMax(L[j][j], 1e-15);
        }
    }

    // Kmm_inv = L^{-T} * L^{-1}
    QVector<QVector<double>> KmmInv(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i) {
        QVector<double> ei(m, 0.0); ei[i] = 1.0;
        // Solve L*y = ei
        QVector<double> y(m, 0.0);
        for (int r = 0; r < m; ++r) {
            double s = ei[r];
            for (int c = 0; c < r; ++c) s -= L[r][c] * y[c];
            y[r] = s / qMax(L[r][r], 1e-15);
        }
        // Solve L^T * z = y
        QVector<double> z(m, 0.0);
        for (int r = m - 1; r >= 0; --r) {
            double s = y[r];
            for (int c = r + 1; c < m; ++c) s -= L[c][r] * z[c];
            z[r] = s / qMax(L[r][r], 1e-15);
        }
        for (int j = 0; j < m; ++j) KmmInv[j][i] = z[j];
    }

    return KmmInv;
}

/* ---- Kernel distance ---- */

double KMeans18::kernelDistance(int point, int cluster,
                                 const QVector<QVector<double>>& Knm,
                                 const QVector<QVector<double>>& KmmInv,
                                 const QVector<double>& clusterWeights) const
{
    int m = Knm[0].size();
    int n = m_data.size();
    double cw = qMax(clusterWeights[cluster], 1e-15);

    // ||phi(x) - mu_c||^2 = k(x,x) - 2/N_c * sum_j k(x,x_j) + 1/N_c^2 * sum_{ij} k(x_i,x_j)
    // Via Nyström: k(x,y) ≈ Knm_x^T * KmmInv * Knm_y
    double selfKernel = 1.0; // k(x,x) for normalized kernels

    // Cross term: 2/N_c * sum of k(x, x_j) for x_j in cluster
    double crossTerm = 0.0;
    for (int j = 0; j < n; ++j) {
        if (m_labels[j] != cluster) continue;
        double kxy = 0.0;
        for (int a = 0; a < m; ++a)
            for (int b = 0; b < m; ++b)
                kxy += Knm[point][a] * KmmInv[a][b] * Knm[j][b];
        crossTerm += kxy;
    }
    crossTerm *= 2.0 / cw;

    return qMax(selfKernel - crossTerm, 0.0);
}

/* ---- Fit ---- */

QVector<int> KMeans18::fit(const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || k <= 0) return {};
    k = qMin(k, n);
    m_k = k;
    m_data = data;

    // Select landmarks
    int m = qMax(4, static_cast<int>(n * m_landmarkRatio));
    QVector<int> landmarks = selectLandmarks(m);
    QVector<QVector<double>> KmmInv = nystromApprox(landmarks);

    // Build cross-kernel matrix
    QVector<QVector<double>> Knm(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            Knm[i][j] = kernelValue(data[i], data[landmarks[j]]);

    // Initialize labels randomly
    m_labels.resize(n);
    for (int i = 0; i < n; ++i) m_labels[i] = i % k;

    // Iterate
    for (int it = 0; it < m_maxIter; ++it) {
        // Count cluster sizes
        QVector<double> clusterCounts(k, 0.0);
        for (int i = 0; i < n; ++i) clusterCounts[m_labels[i]] += 1.0;

        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestCluster = 0;
            for (int c = 0; c < k; ++c) {
                double d = kernelDistance(i, c, Knm, KmmInv, clusterCounts);
                if (d < bestDist) { bestDist = d; bestCluster = c; }
            }
            if (m_labels[i] != bestCluster) { m_labels[i] = bestCluster; changed = true; }
        }
        if (!changed) break;
    }

    // Compute centers (mean in original space per cluster)
    QVector<int> counts(k, 0);
    int dim = data[0].size();
    m_centers = QVector<QVector<double>>(k, QVector<double>(dim, 0.0));
    for (int i = 0; i < n; ++i) {
        int c = m_labels[i];
        counts[c]++;
        for (int d = 0; d < dim; ++d) m_centers[c][d] += data[i][d];
    }
    for (int c = 0; c < k; ++c)
        if (counts[c] > 0)
            for (int d = 0; d < dim; ++d) m_centers[c][d] /= counts[c];

    m_stats.totalRuns++;
    m_stats.numClusters = k;
    m_stats.numPoints = n;
    m_stats.nystromLandmarks = m;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(k, m_maxIter, timer.elapsed());
    return m_labels;
}

/* ---- Predict ---- */

QVector<int> KMeans18::predict(const QVector<QVector<double>>& points) const
{
    if (m_centers.isEmpty()) return {};
    QVector<int> result(points.size(), 0);
    for (int i = 0; i < points.size(); ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int c = 0; c < m_centers.size(); ++c) {
            double dist = 0.0;
            for (int d = 0; d < points[i].size(); ++d) {
                double diff = points[i][d] - m_centers[c][d];
                dist += diff * diff;
            }
            if (dist < bestDist) { bestDist = dist; result[i] = c; }
        }
    }
    return result;
}

/* ---- Accessors ---- */

QVector<QVector<double>> KMeans18::centers() const { return m_centers; }

/* ---- Reset ---- */

void KMeans18::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_data.clear();
    m_centers.clear();
    m_labels.clear();
}
