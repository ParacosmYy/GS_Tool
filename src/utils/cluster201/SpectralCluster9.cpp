/**
 * @file SpectralCluster9.cpp
 * @brief SpectralCluster9 实现
 *
 * 实现谱聚类：RBF相似度图、归一化Laplacian、Fiedler向量二分、递归多类分裂。
 */

#include "utils/cluster201/SpectralCluster9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SpectralCluster9::SpectralCluster9(QObject *parent) : QObject(parent) {}
SpectralCluster9::~SpectralCluster9() = default;

/* ---- Configuration ---- */

void SpectralCluster9::setNumClusters(int k) { m_k = qMax(2, k); }
void SpectralCluster9::setSigma(double sigma) { m_sigma = qMax(0.01, sigma); }
void SpectralCluster9::setKNeighbors(int k) { m_kNeighbors = qMax(1, k); }

/* ---- Distance ---- */

double SpectralCluster9::distance(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Build affinity matrix ---- */

QVector<QVector<double>> SpectralCluster9::buildAffinity(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    double twoSigmaSq = 2.0 * m_sigma * m_sigma;

    for (int i = 0; i < n; ++i) {
        // Find k-nearest neighbors
        QVector<QPair<double, int>> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            dists.append({distance(data[i], data[j]), j});
        }
        std::sort(dists.begin(), dists.end());

        int count = qMin(m_kNeighbors, dists.size());
        for (int k = 0; k < count; ++k) {
            int j = dists[k].second;
            double sim = qExp(-dists[k].first * dists[k].first / twoSigmaSq);
            W[i][j] = sim;
            W[j][i] = sim;
        }
    }
    return W;
}

/* ---- Compute normalized Laplacian ---- */

void SpectralCluster9::computeLaplacian(const QVector<QVector<double>>& affinity,
                                          QVector<QVector<double>>& laplacian) const
{
    int n = affinity.size();
    laplacian = QVector<QVector<double>>(n, QVector<double>(n, 0.0));

    // Compute degree matrix D and D^{-1/2}
    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double deg = 0.0;
        for (int j = 0; j < n; ++j) deg += affinity[i][j];
        dInvSqrt[i] = (deg > 1e-15) ? 1.0 / qSqrt(deg) : 0.0;
    }

    // L_sym = I - D^{-1/2} W D^{-1/2}
    for (int i = 0; i < n; ++i) {
        laplacian[i][i] = 1.0;
        for (int j = 0; j < n; ++j) {
            laplacian[i][j] -= dInvSqrt[i] * affinity[i][j] * dInvSqrt[j];
        }
    }
}

/* ---- Power iteration ---- */

QVector<double> SpectralCluster9::powerIteration(const QVector<QVector<double>>& mat, int maxIter)
{
    int n = mat.size();
    if (n == 0) return {};

    QVector<double> v(n, 1.0 / qSqrt(n));
    normalizeVector(v);

    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<double> mv(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                mv[i] += mat[i][j] * v[j];
        normalizeVector(mv);
        v = mv;
    }
    return v;
}

/* ---- Normalize vector ---- */

void SpectralCluster9::normalizeVector(QVector<double>& v)
{
    double norm = 0.0;
    for (double x : v) norm += x * x;
    norm = qSqrt(norm);
    if (norm > 1e-15)
        for (double& x : v) x /= norm;
}

/* ---- Fiedler vector ---- */

QVector<double> SpectralCluster9::fiedlerVector(const QVector<QVector<double>>& laplacian) const
{
    int n = laplacian.size();
    if (n < 2) return {};

    // First eigenvector of L_sym (constant) — deflate it
    QVector<double> ones(n, 1.0 / qSqrt(n));

    // Iteratively compute eigenvectors via deflation
    QVector<double> u1 = powerIteration(laplacian, 200);

    // Compute inverse power shifted for Fiedler (2nd eigenvector)
    // Use random init orthogonal to u1
    QVector<double> v(n);
    for (int i = 0; i < n; ++i) v[i] = (i % 2 == 0) ? 1.0 : -1.0;

    // Orthogonalize against u1
    double dot = 0.0;
    for (int i = 0; i < n; ++i) dot += v[i] * u1[i];
    for (int i = 0; i < n; ++i) v[i] -= dot * u1[i];
    normalizeVector(v);

    // Power iteration on deflated matrix
    for (int iter = 0; iter < 300; ++iter) {
        QVector<double> mv(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                mv[i] += laplacian[i][j] * v[j];

        // Deflate against u1
        double d = 0.0;
        for (int i = 0; i < n; ++i) d += mv[i] * u1[i];
        for (int i = 0; i < n; ++i) mv[i] -= d * u1[i];

        normalizeVector(mv);
        v = mv;
    }
    return v;
}

/* ---- Bi-partition ---- */

QPair<QVector<int>, QVector<int>> SpectralCluster9::biPartition(const QVector<double>& fiedler) const
{
    QVector<int> groupA, groupB;
    for (int i = 0; i < fiedler.size(); ++i) {
        if (fiedler[i] >= 0.0) groupA.append(i);
        else groupB.append(i);
    }
    return {groupA, groupB};
}

/* ---- Recursive split ---- */

QVector<int> SpectralCluster9::recursiveSplit(const QVector<QVector<double>>& data,
                                                const QVector<int>& indices, int targetK) const
{
    if (targetK <= 1 || indices.size() <= 1) {
        QVector<int> labels(indices.size(), m_k - targetK);
        return labels;
    }

    // Extract sub-data
    QVector<QVector<double>> subData;
    subData.reserve(indices.size());
    for (int idx : indices) subData.append(data[idx]);

    QVector<QVector<double>> aff = buildAffinity(subData);
    QVector<QVector<double>> lap;
    computeLaplacian(aff, lap);
    QVector<double> fv = fiedlerVector(lap);

    auto [gA, gB] = biPartition(fv);

    // Map back to original indices
    QVector<int> idxA, idxB;
    for (int i : gA) idxA.append(indices[i]);
    for (int i : gB) idxB.append(indices[i]);

    int kA = qMax(1, targetK / 2);
    int kB = targetK - kA;
    if (idxA.isEmpty()) return recursiveSplit(data, idxB, targetK);
    if (idxB.isEmpty()) return recursiveSplit(data, idxA, targetK);

    QVector<int> lA = recursiveSplit(data, idxA, kA);
    QVector<int> lB = recursiveSplit(data, idxB, kB);

    // Merge labels: offset lB by kA
    QVector<int> result;
    result.reserve(lA.size() + lB.size());
    for (int l : lA) result.append(l);
    for (int l : lB) result.append(l + kA);
    return result;
}

/* ---- Fit ---- */

QVector<int> SpectralCluster9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    QVector<int> indices;
    indices.reserve(n);
    for (int i = 0; i < n; ++i) indices.append(i);

    QVector<int> result(n);
    QVector<int> splitLabels = recursiveSplit(data, indices, m_k);

    // Map split labels back to original order
    if (splitLabels.size() == n) {
        result = splitLabels;
    }

    m_stats.totalFits++;
    m_stats.numSamples = n;
    m_stats.numClusters = m_k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit clusteringCompleted(m_k, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void SpectralCluster9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
