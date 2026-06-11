/**
 * @file KMeans31.cpp
 * @brief KMeans31 实现
 *
 * 实现K-means聚类：核密度估计初始化与马氏距离椭圆簇形状检测。
 */

#include "utils/cluster286/KMeans31.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

KMeans31::KMeans31(QObject *parent)
    : QObject(parent) {}

KMeans31::~KMeans31() = default;

/* ---- Configuration ---- */

void KMeans31::setK(int k) { m_k = qBound(1, k, 200); }
void KMeans31::setMaxIter(int iters) { m_maxIter = qBound(10, iters, 10000); }
void KMeans31::setTolerance(double tol) { m_tol = qBound(1e-12, tol, 1.0); }
void KMeans31::setBandwidth(double bw) { m_bandwidth = qBound(0.0, bw, 1e6); }

/* ---- Kernel density estimation at a point ---- */

double KMeans31::kdeAt(const QVector<QVector<double>>& data,
                        const QVector<double>& point, double bw) const
{
    int n = data.size();
    int d = point.size();
    if (n == 0 || d == 0) return 0.0;
    double density = 0.0;
    double bw2 = bw * bw;
    for (int i = 0; i < n; ++i) {
        double dist2 = 0.0;
        for (int j = 0; j < d; ++j)
            dist2 += (data[i][j] - point[j]) * (data[i][j] - point[j]);
        density += qExp(-0.5 * dist2 / bw2);
    }
    density /= (n * qPow(2.0 * M_PI * bw2, d / 2.0));
    return density;
}

/* ---- KDE-based initialization: find density peaks ---- */

QVector<QVector<double>> KMeans31::kdeInit(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n == 0) return {};
    int d = data[0].size();

    // Estimate bandwidth via Silverman's rule if not set
    double bw = m_bandwidth;
    if (bw <= 0.0) {
        double sumVar = 0.0;
        for (int j = 0; j < d; ++j) {
            double mean = 0.0;
            for (int i = 0; i < n; ++i) mean += data[i][j];
            mean /= n;
            double var = 0.0;
            for (int i = 0; i < n; ++i) var += (data[i][j] - mean) * (data[i][j] - mean);
            sumVar += var / n;
        }
        bw = qSqrt(sumVar / d) * qPow(3.0 / (4.0 * n), 1.0 / (d + 4.0));
    }

    // Evaluate KDE at each data point and sort by density
    QVector<QPair<double, int>> densityIdx(n);
    for (int i = 0; i < n; ++i) {
        densityIdx[i] = {kdeAt(data, data[i], bw), i};
    }
    std::sort(densityIdx.begin(), densityIdx.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Select peaks with minimum separation
    QVector<QVector<double>> centers;
    double minSep = bw * 2.0;
    for (int i = 0; i < n && centers.size() < m_k; ++i) {
        const auto& candidate = data[densityIdx[i].second];
        bool tooClose = false;
        for (const auto& c : centers) {
            if (euclidean(candidate, c) < minSep) { tooClose = true; break; }
        }
        if (!tooClose) centers.append(candidate);
    }
    // Fill remaining with random if not enough peaks
    std::mt19937 rng(42);
    while (centers.size() < m_k) {
        centers.append(data[rng() % n]);
    }
    return centers;
}

/* ---- Euclidean distance ---- */

double KMeans31::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    for (int i = 0; i < a.size(); ++i) s += (a[i] - b[i]) * (a[i] - b[i]);
    return qSqrt(s);
}

/* ---- Mahalanobis distance ---- */

double KMeans31::mahalanobis(const QVector<double>& x, const Cluster& c) const
{
    int d = x.size();
    if (d == 0) return 0.0;
    double dist = 0.0;
    for (int i = 0; i < d; ++i) {
        double diff_i = x[i] - c.mean[i];
        double sum = 0.0;
        for (int j = 0; j < d; ++j)
            sum += c.invCovariance[i][j] * (x[j] - c.mean[j]);
        dist += diff_i * sum;
    }
    return qSqrt(qMax(dist, 0.0));
}

/* ---- Update cluster covariance ---- */

void KMeans31::updateCovariance(Cluster& c, const QVector<QVector<double>>& data,
                                 const QVector<int>& labels, int cid)
{
    int n = data.size();
    int d = data[0].size();
    c.count = 0;
    c.mean.fill(0.0, d);

    for (int i = 0; i < n; ++i) {
        if (labels[i] != cid) continue;
        c.count++;
        for (int j = 0; j < d; ++j) c.mean[j] += data[i][j];
    }
    if (c.count == 0) return;
    for (int j = 0; j < d; ++j) c.mean[j] /= c.count;

    c.covariance = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
    for (int i = 0; i < n; ++i) {
        if (labels[i] != cid) continue;
        for (int a = 0; a < d; ++a) {
            double da = data[i][a] - c.mean[a];
            for (int b = 0; b < d; ++b)
                c.covariance[a][b] += da * (data[i][b] - c.mean[b]);
        }
    }
    for (int a = 0; a < d; ++a)
        for (int b = 0; b < d; ++b)
            c.covariance[a][b] /= c.count;
    // Regularize
    for (int a = 0; a < d; ++a) c.covariance[a][a] += 1e-6;
    c.invCovariance = invertMatrix(c.covariance);
}

/* ---- Matrix inversion (Gauss-Jordan) ---- */

QVector<QVector<double>> KMeans31::invertMatrix(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<QVector<double>> A = mat;
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) inv[i][i] = 1.0;

    for (int col = 0; col < n; ++col) {
        double pivot = A[col][col];
        if (qAbs(pivot) < 1e-15) pivot = 1e-10;
        for (int j = 0; j < n; ++j) { A[col][j] /= pivot; inv[col][j] /= pivot; }
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = A[row][col];
            for (int j = 0; j < n; ++j) {
                A[row][j] -= factor * A[col][j];
                inv[row][j] -= factor * inv[col][j];
            }
        }
    }
    return inv;
}

/* ---- Main fit ---- */

KMeans31::FitResult KMeans31::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;
    int n = data.size();
    if (n == 0) return result;
    int d = data[0].size();
    m_dim = d;

    // KDE-based initialization
    QVector<QVector<double>> centers = kdeInit(data);

    // Initialize clusters
    QVector<Cluster> clusters(m_k);
    for (int j = 0; j < m_k; ++j) {
        clusters[j].mean = centers[j];
        clusters[j].covariance = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
        for (int a = 0; a < d; ++a) clusters[j].covariance[a][a] = 1.0;
        clusters[j].invCovariance = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
        for (int a = 0; a < d; ++a) clusters[j].invCovariance[a][a] = 1.0;
    }

    result.labels.fill(0, n);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Assignment step: use Mahalanobis distance
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            int bestJ = 0;
            double bestDist = 1e300;
            for (int j = 0; j < m_k; ++j) {
                double dist = mahalanobis(data[i], clusters[j]);
                if (dist < bestDist) { bestDist = dist; bestJ = j; }
            }
            if (result.labels[i] != bestJ) { result.labels[i] = bestJ; changed = true; }
        }
        if (!changed) break;

        // Update step: recompute means and covariances
        for (int j = 0; j < m_k; ++j)
            updateCovariance(clusters[j], data, result.labels, j);
        result.iterations = iter + 1;
    }

    // Compute inertia
    result.inertia = 0.0;
    for (int i = 0; i < n; ++i) {
        double dist = mahalanobis(data[i], clusters[result.labels[i]]);
        result.inertia += dist * dist;
    }

    m_clusters = clusters;
    result.clusters = clusters;

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numClusters = m_k;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitDone(n, m_k, result.inertia, elapsed);
    return result;
}

/* ---- Predict ---- */

QVector<int> KMeans31::predict(const QVector<QVector<double>>& samples) const
{
    QVector<int> labels(samples.size());
    for (int i = 0; i < samples.size(); ++i) {
        int bestJ = 0;
        double bestDist = 1e300;
        for (int j = 0; j < m_clusters.size(); ++j) {
            double dist = mahalanobis(samples[i], m_clusters[j]);
            if (dist < bestDist) { bestDist = dist; bestJ = j; }
        }
        labels[i] = bestJ;
    }
    return labels;
}

/* ---- Reset ---- */

void KMeans31::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_clusters.clear();
}
