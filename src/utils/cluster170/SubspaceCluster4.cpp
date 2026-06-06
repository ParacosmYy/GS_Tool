/**
 * @file SubspaceCluster4.cpp
 * @brief SubspaceCluster4 实现
 *
 * 实现CLIQUE子空间聚类：1D密集单元检测、Apriori候选生成、多维密度验证。
 */

#include "utils/cluster170/SubspaceCluster4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SubspaceCluster4::SubspaceCluster4(QObject *parent)
    : QObject(parent)
{
}

SubspaceCluster4::~SubspaceCluster4() = default;

/* ---- Configuration ---- */

void SubspaceCluster4::setGridBins(int bins) { m_bins = qMax(2, bins); }
void SubspaceCluster4::setDensityThreshold(double tau) { m_tau = qBound(0.01, tau, 1.0); }
void SubspaceCluster4::setMaxSubspaceDim(int maxDim) { m_maxDim = qMax(1, maxDim); }

/* ---- Map value to bin index ---- */

int SubspaceCluster4::toBin(double value, double minVal, double maxVal) const
{
    if (maxVal <= minVal) return 0;
    int b = static_cast<int>((value - minVal) / (maxVal - minVal) * m_bins);
    return qBound(0, b, m_bins - 1);
}

/* ---- 1D dense unit detection ---- */

QVector<QVector<int>> SubspaceCluster4::find1DDenseUnits(
    const QVector<QVector<double>>& data, int dim) const
{
    int n = data.size();
    if (n == 0) return {};

    /* Compute min/max for this dimension */
    double minV = data[0][dim], maxV = data[0][dim];
    for (int i = 1; i < n; ++i) {
        if (data[i][dim] < minV) minV = data[i][dim];
        if (data[i][dim] > maxV) maxV = data[i][dim];
    }

    /* Count points per bin */
    QVector<int> counts(m_bins, 0);
    for (int i = 0; i < n; ++i)
        counts[toBin(data[i][dim], minV, maxV)]++;

    /* Identify dense bins */
    int threshold = qCeil(m_tau * n);
    QVector<QVector<int>> dense;
    for (int b = 0; b < m_bins; ++b) {
        if (counts[b] >= threshold) {
            dense.append({dim, b});
        }
    }
    return dense;
}

/* ---- Apriori join: generate (k+1)-dim candidates from k-dim dense units ---- */

QVector<QVector<int>> SubspaceCluster4::aprioriJoin(
    const QVector<QVector<int>>& kDense, int k) const
{
    QVector<QVector<int>> candidates;
    int n = kDense.size();

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            /* Check if first (k-1) dims match, last dim differs */
            bool match = true;
            for (int d = 0; d < k - 1 && match; ++d)
                if (kDense[i][d] != kDense[j][d]) match = false;

            if (k >= 1) {
                /* For 1D units: join if different dims */
                if (k == 1 && kDense[i][0] != kDense[j][0]) {
                    QVector<int> cand;
                    int d1 = kDense[i][0], d2 = kDense[j][0];
                    int b1 = kDense[i][1], b2 = kDense[j][1];
                    if (d1 < d2) { cand = {d1, d2, b1, b2}; }
                    else { cand = {d2, d1, b2, b1}; }
                    candidates.append(cand);
                }
            }
        }
    }
    return candidates;
}

/* ---- Check if a candidate subspace is dense ---- */

bool SubspaceCluster4::isDense(const QVector<QVector<double>>& data,
                                const QVector<int>& dims,
                                const QVector<int>& ranges) const
{
    int n = data.size();
    int k = dims.size();
    int count = 0;

    /* Compute min/max per dimension */
    QVector<double> minV(k), maxV(k);
    for (int d = 0; d < k; ++d) {
        minV[d] = data[0][dims[d]];
        maxV[d] = data[0][dims[d]];
        for (int i = 1; i < n; ++i) {
            if (data[i][dims[d]] < minV[d]) minV[d] = data[i][dims[d]];
            if (data[i][dims[d]] > maxV[d]) maxV[d] = data[i][dims[d]];
        }
    }

    int threshold = qCeil(m_tau * n);
    for (int i = 0; i < n; ++i) {
        bool inCell = true;
        for (int d = 0; d < k && inCell; ++d) {
            if (toBin(data[i][dims[d]], minV[d], maxV[d]) != ranges[d])
                inCell = false;
        }
        if (inCell) count++;
    }
    return count >= threshold;
}

/* ---- Extract cluster labels from dense cells ---- */

QVector<int> SubspaceCluster4::extractLabels(int n) const
{
    QVector<int> labels(n, -1);
    int clusterId = 0;

    /* Assign points in dense cells to clusters */
    for (int c = 0; c < m_denseCells.size(); ++c) {
        bool merged = false;
        /* Check overlap with existing clusters */
        for (int p : m_denseCells[c].points) {
            if (labels[p] >= 0) {
                /* Merge: relabel all points in this cell */
                int target = labels[p];
                for (int q : m_denseCells[c].points)
                    labels[q] = target;
                merged = true;
                break;
            }
        }
        if (!merged) {
            for (int p : m_denseCells[c].points)
                labels[p] = clusterId;
            clusterId++;
        }
    }
    return labels;
}

/* ---- Main clustering ---- */

QVector<int> SubspaceCluster4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};
    int dims = data[0].size();
    if (dims == 0) return {};

    m_denseSubspaces.clear();
    m_denseCells.clear();

    /* Phase 1: Find 1D dense units for each dimension */
    QVector<QVector<int>> allDense1D;
    for (int d = 0; d < dims; ++d) {
        QVector<QVector<int>> dense1D = find1DDenseUnits(data, d);
        allDense1D.append(dense1D);
        for (const auto& unit : dense1D)
            m_denseSubspaces.append({unit[0]});
    }

    /* Phase 2: Build dense cells from 1D dense units */
    /* For each dimension, create dense cells */
    for (int d = 0; d < dims; ++d) {
        const auto& units = allDense1D[d];
        double minV = data[0][d], maxV = data[0][d];
        for (int i = 1; i < n; ++i) {
            if (data[i][d] < minV) minV = data[i][d];
            if (data[i][d] > maxV) maxV = data[i][d];
        }
        for (const auto& unit : units) {
            GridCell cell;
            cell.coords = {unit[1]};
            for (int i = 0; i < n; ++i) {
                if (toBin(data[i][d], minV, maxV) == unit[1])
                    cell.points.append(i);
            }
            cell.count = cell.points.size();
            m_denseCells.append(cell);
        }
    }

    /* Phase 3: Apriori candidate generation for higher dimensions */
    int currentDim = 1;
    QVector<QVector<int>> prevDense = allDense1D;

    while (currentDim < m_maxDim && prevDense.size() > 1) {
        QVector<QVector<int>> candidates = aprioriJoin(prevDense, currentDim);
        QVector<QVector<int>> newDense;

        for (const auto& cand : candidates) {
            int k = cand.size() / 2;
            QVector<int> dims_part(cand.constBegin(), cand.constBegin() + k);
            QVector<int> ranges(cand.constBegin() + k, cand.constEnd());

            if (isDense(data, dims_part, ranges)) {
                newDense.append(cand);
                m_denseSubspaces.append(dims_part);
            }
        }

        prevDense = newDense;
        currentDim++;
    }

    /* Phase 4: Extract cluster labels */
    QVector<int> labels = extractLabels(n);

    /* Count unique clusters */
    int numClusters = 0;
    for (int l : labels)
        if (l + 1 > numClusters) numClusters = l + 1;

    /* Relabel to sequential */
    QVector<int> unique;
    for (int l : labels)
        if (l >= 0 && !unique.contains(l)) unique.append(l);
    for (int i = 0; i < n; ++i)
        if (labels[i] >= 0) labels[i] = unique.indexOf(labels[i]);

    m_stats.totalRuns++;
    m_stats.lastClusters = unique.size();
    m_stats.lastSubspaces = m_denseSubspaces.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(m_stats.lastClusters, m_stats.lastSubspaces);
    return labels;
}

/* ---- Accessors ---- */

QVector<QVector<int>> SubspaceCluster4::denseSubspaces() const
{
    return m_denseSubspaces;
}

/* ---- Statistics ---- */

void SubspaceCluster4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
