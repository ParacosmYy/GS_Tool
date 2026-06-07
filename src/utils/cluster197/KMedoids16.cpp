/**
 * @file KMedoids16.cpp
 * @brief KMedoids16 实现
 *
 * 实现K-中心点聚类：CLARANS随机交换策略、轮廓系数评估、多种距离度量。
 */

#include "utils/cluster197/KMedoids16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

KMedoids16::KMedoids16(QObject *parent) : QObject(parent) {}
KMedoids16::~KMedoids16() = default;

/* ---- Configuration ---- */

void KMedoids16::setNumClusters(int k) { m_k = qMax(2, k); }
void KMedoids16::setMaxIterations(int iters) { m_maxIter = qMax(10, iters); }
void KMedoids16::setNumNeighbors(int n) { m_numNeighbors = qMax(1, n); }
void KMedoids16::setDistanceMetric(DistanceMetric m) { m_metric = m; }

/* ---- Distance computation ---- */

double KMedoids16::distance(const QVector<double>& a, const QVector<double>& b) const
{
    int d = qMin(a.size(), b.size());
    if (d == 0) return 0.0;

    if (m_metric == DistanceMetric::Euclidean) {
        double sum = 0.0;
        for (int i = 0; i < d; ++i) { double diff = a[i] - b[i]; sum += diff * diff; }
        return qSqrt(sum);
    } else if (m_metric == DistanceMetric::Manhattan) {
        double sum = 0.0;
        for (int i = 0; i < d; ++i) sum += qAbs(a[i] - b[i]);
        return sum;
    } else {
        // Cosine distance: 1 - cos(a, b)
        double dot = 0.0, nA = 0.0, nB = 0.0;
        for (int i = 0; i < d; ++i) {
            dot += a[i] * b[i]; nA += a[i] * a[i]; nB += b[i] * b[i];
        }
        double denom = qSqrt(nA) * qSqrt(nB);
        return (denom > 1e-12) ? 1.0 - dot / denom : 1.0;
    }
}

/* ---- Distance matrix ---- */

QVector<QVector<double>> KMedoids16::distanceMatrix(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> dm(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = distance(data[i], data[j]);
            dm[i][j] = d; dm[j][i] = d;
        }
    return dm;
}

/* ---- Assign clusters ---- */

QVector<int> KMedoids16::assignClusters(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int m = 0; m < m_medoidIndices.size(); ++m) {
            double d = m_distMatrix[i][m_medoidIndices[m]];
            if (d < bestDist) { bestDist = d; labels[i] = m; }
        }
    }
    return labels;
}

/* ---- Total cost ---- */

double KMedoids16::totalCost(const QVector<QVector<double>>& data,
                              const QVector<int>& medoids) const
{
    double cost = 0.0;
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        double minD = std::numeric_limits<double>::max();
        for (int med : medoids)
            minD = qMin(minD, m_distMatrix[i][med]);
        cost += minD;
    }
    return cost;
}

/* ---- CLARANS swap ---- */

bool KMedoids16::claransSwap(const QVector<QVector<double>>& data, int n)
{
    double bestCost = totalCost(data, m_medoidIndices);
    bool improved = false;

    for (int trial = 0; trial < m_numNeighbors; ++trial) {
        int medIdx = QRandomGenerator::global()->bounded(m_medoidIndices.size());
        int newPt = QRandomGenerator::global()->bounded(n);

        // Skip if already a medoid
        bool isMedoid = false;
        for (int m : m_medoidIndices) if (m == newPt) { isMedoid = true; break; }
        if (isMedoid) continue;

        int oldMed = m_medoidIndices[medIdx];
        m_medoidIndices[medIdx] = newPt;
        double newCost = totalCost(data, m_medoidIndices);

        if (newCost < bestCost) {
            bestCost = newCost;
            improved = true;
        } else {
            m_medoidIndices[medIdx] = oldMed; // Revert
        }
    }
    return improved;
}

/* ---- Silhouette ---- */

double KMedoids16::pointSilhouette(int idx, const QVector<int>& labels, int k) const
{
    int n = labels.size();
    int myCluster = labels[idx];

    // Average distance to own cluster
    double a = 0.0;
    int ownCount = 0;
    for (int i = 0; i < n; ++i) {
        if (labels[i] == myCluster && i != idx) { a += m_distMatrix[idx][i]; ownCount++; }
    }
    a = (ownCount > 0) ? a / ownCount : 0.0;

    // Minimum average distance to other clusters
    double minB = std::numeric_limits<double>::max();
    for (int c = 0; c < k; ++c) {
        if (c == myCluster) continue;
        double b = 0.0; int count = 0;
        for (int i = 0; i < n; ++i) {
            if (labels[i] == c) { b += m_distMatrix[idx][i]; count++; }
        }
        if (count > 0) minB = qMin(minB, b / count);
    }
    if (minB == std::numeric_limits<double>::max()) return 0.0;
    return (minB - a) / qMax(minB, a);
}

double KMedoids16::computeSilhouette(const QVector<QVector<double>>& /*data*/,
                                      const QVector<int>& labels) const
{
    int n = labels.size();
    if (n <= 1) return 0.0;
    int k = m_medoidIndices.size();
    if (k <= 1) return 0.0;

    double sum = 0.0;
    for (int i = 0; i < n; ++i) sum += pointSilhouette(i, labels, k);
    return sum / n;
}

/* ---- Fit ---- */

QVector<int> KMedoids16::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    m_distMatrix = distanceMatrix(data);
    int k = qMin(m_k, n);

    // Initialize: random medoid selection
    m_medoidIndices.clear();
    QVector<bool> chosen(n, false);
    while (m_medoidIndices.size() < k) {
        int idx = QRandomGenerator::global()->bounded(n);
        if (!chosen[idx]) { m_medoidIndices.append(idx); chosen[idx] = true; }
    }

    // CLARANS iterations
    for (int iter = 0; iter < m_maxIter; ++iter) {
        if (!claransSwap(data, n)) break;
    }

    QVector<int> labels = assignClusters(data);
    double sil = computeSilhouette(data, labels);

    m_stats.totalFits++;
    m_stats.numSamples = n;
    m_stats.numClusters = k;
    m_stats.silhouetteScore = sil;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fittingCompleted(k, sil, timer.elapsed());
    return labels;
}

/* ---- Predict ---- */

QVector<int> KMedoids16::predict(const QVector<QVector<double>>& data) const
{
    if (m_medoidIndices.isEmpty()) return {};
    // Build temporary distance matrix for new data vs medoids
    int n = data.size();
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        for (int m = 0; m < m_medoidIndices.size(); ++m) {
            double d = distance(data[i], data[m_medoidIndices[m]]);
            if (d < bestDist) { bestDist = d; labels[i] = m; }
        }
    }
    return labels;
}

/* ---- Medoids ---- */

QVector<int> KMedoids16::medoids() const { return m_medoidIndices; }

/* ---- Reset ---- */

void KMedoids16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_medoidIndices.clear();
    m_distMatrix.clear();
}
