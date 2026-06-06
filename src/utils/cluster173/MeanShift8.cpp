/**
 * @file MeanShift8.cpp
 * @brief MeanShift8 实现
 *
 * 实现均值漂移聚类：气球估计器变带宽、模式收敛检测、簇合并。
 */

#include "utils/cluster173/MeanShift8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MeanShift8::MeanShift8(QObject *parent)
    : QObject(parent)
{
}

MeanShift8::~MeanShift8() = default;

/* ---- Configuration ---- */

void MeanShift8::setKernel(Kernel k) { m_kernel = k; }
void MeanShift8::setBandwidthStrategy(BandwidthStrategy s) { m_bwStrategy = s; }
void MeanShift8::setBandwidth(double h) { m_bandwidth = qMax(1e-6, h); }
void MeanShift8::setKNNK(int k) { m_knnK = qMax(1, k); }
void MeanShift8::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void MeanShift8::setConvergenceThreshold(double eps) { m_convergenceEps = qMax(1e-10, eps); }
void MeanShift8::setClusterMergeThreshold(double t) { m_mergeThreshold = qMax(1e-6, t); }

/* ---- Distance ---- */

double MeanShift8::euclidean(const QVector<double>& a,
                              const QVector<double>& b)
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---- Kernel function ---- */

double MeanShift8::kernelValue(double dist, double h) const
{
    double u = dist / h;
    if (u > 1.0 && m_kernel != Gaussian) return 0.0;

    switch (m_kernel) {
    case Gaussian:
        return qExp(-0.5 * u * u);
    case Epanechnikov:
        return 0.75 * (1.0 - u * u);
    case Uniform:
        return 0.5;
    }
    return 0.0;
}

/* ---- Balloon bandwidth ---- */

double MeanShift8::balloonBandwidth(const QVector<QVector<double>>& data,
                                     int idx) const
{
    int n = data.size();
    if (n <= 1) return m_bandwidth;

    /* Balloon estimator: bandwidth proportional to local density */
    double sumDist = 0.0;
    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        double d = euclidean(data[idx], data[i]);
        if (d < m_bandwidth * 3.0) {
            sumDist += d;
            ++count;
        }
    }
    if (count == 0) return m_bandwidth;
    double localDensity = count / (sumDist + 1e-10);
    /* Inverse relationship: sparse regions get larger bandwidth */
    return m_bandwidth * qMax(0.5, 1.0 / (localDensity + 1.0));
}

/* ---- KNN bandwidth ---- */

double MeanShift8::knnBandwidth(const QVector<QVector<double>>& data,
                                 int idx) const
{
    int n = data.size();
    if (n <= m_knnK) return m_bandwidth;

    /* Find distance to k-th nearest neighbor */
    QVector<double> dists;
    dists.reserve(n - 1);
    for (int i = 0; i < n; ++i) {
        if (i == idx) continue;
        dists.append(euclidean(data[idx], data[i]));
    }
    std::sort(dists.begin(), dists.end());
    return qMax(1e-6, dists[qMin(m_knnK - 1, dists.size() - 1)]);
}

/* ---- Mean shift single point ---- */

QVector<double> MeanShift8::shiftPoint(const QVector<QVector<double>>& data,
                                        const QVector<double>& point,
                                        const QVector<double>& bw) const
{
    int dims = point.size();
    int n = data.size();
    QVector<double> numerator(dims, 0.0);
    double denominator = 0.0;

    for (int i = 0; i < n; ++i) {
        double h = (bw.size() > i) ? bw[i] : m_bandwidth;
        double d = euclidean(point, data[i]);
        double w = kernelValue(d, h);
        if (w < 1e-15) continue;
        for (int j = 0; j < dims; ++j)
            numerator[j] += w * data[i][j];
        denominator += w;
    }

    if (denominator < 1e-15) return point;

    QVector<double> shifted(dims);
    for (int j = 0; j < dims; ++j)
        shifted[j] = numerator[j] / denominator;
    return shifted;
}

/* ---- Merge modes ---- */

void MeanShift8::mergeModes(const QVector<double>& mergeDist)
{
    int n = m_modes.size();
    QVector<int> labels(n, -1);
    int clusterCount = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] >= 0) continue;
        labels[i] = clusterCount;
        for (int j = i + 1; j < n; ++j) {
            if (labels[j] >= 0) continue;
            if (euclidean(m_modes[i], m_modes[j]) < mergeDist[j])
                labels[j] = clusterCount;
        }
        ++clusterCount;
    }

    /* Rebuild mode list */
    QVector<QVector<double>> newModes(clusterCount);
    QVector<int> counts(clusterCount, 0);
    for (int i = 0; i < n; ++i) {
        int c = labels[i];
        if (counts[c] == 0) {
            newModes[c] = m_modes[i];
        }
        counts[c]++;
    }
    m_modes = newModes;

    /* Update point labels */
    for (int i = 0; i < m_labels.size(); ++i)
        m_labels[i] = (m_labels[i] >= 0 && m_labels[i] < n)
                          ? labels[m_labels[i]] : 0;
}

/* ---- Main fit ---- */

QVector<int> MeanShift8::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    /* Compute per-point bandwidth */
    m_bandwidths.resize(n);
    for (int i = 0; i < n; ++i) {
        switch (m_bwStrategy) {
        case Balloon:
            m_bandwidths[i] = balloonBandwidth(data, i);
            break;
        case KNN:
            m_bandwidths[i] = knnBandwidth(data, i);
            break;
        default:
            m_bandwidths[i] = m_bandwidth;
        }
    }

    /* Run mean shift for each point */
    QVector<QVector<double>> allModes(n);
    m_labels.resize(n);
    int totalIter = 0;

    for (int i = 0; i < n; ++i) {
        QVector<double> current = data[i];
        for (int iter = 0; iter < m_maxIter; ++iter) {
            QVector<double> shifted = shiftPoint(data, current, m_bandwidths);
            if (euclidean(shifted, current) < m_convergenceEps)
                break;
            current = shifted;
            ++totalIter;
        }
        allModes[i] = current;
        emit iterationProgress(i, n);
    }

    m_modes = allModes;

    /* Merge close modes */
    m_labels.fill(0);
    for (int i = 0; i < n; ++i) m_labels[i] = i;
    mergeModes(QVector<double>(n, m_mergeThreshold));

    /* Update stats */
    double bwSum = 0.0;
    for (int i = 0; i < n; ++i) bwSum += m_bandwidths[i];

    m_stats.totalRuns++;
    m_stats.numClusters = m_modes.size();
    m_stats.numIterations = totalIter;
    m_stats.avgBandwidth = (n > 0) ? bwSum / n : 0.0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(m_stats.numClusters, totalIter);
    return m_labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> MeanShift8::modes() const { return m_modes; }
QVector<double> MeanShift8::bandwidths() const { return m_bandwidths; }

void MeanShift8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
