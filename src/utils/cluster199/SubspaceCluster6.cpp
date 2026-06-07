/**
 * @file SubspaceCluster6.cpp
 * @brief SubspaceCluster6 实现
 *
 * 实现CLIQUE子空间聚类：自适应网格分辨率、显著密集单元挖掘、子空间簇合并。
 */

#include "utils/cluster199/SubspaceCluster6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SubspaceCluster6::SubspaceCluster6(QObject *parent) : QObject(parent) {}
SubspaceCluster6::~SubspaceCluster6() = default;

/* ---- Configuration ---- */

void SubspaceCluster6::setGridResolution(int bins) { m_bins = qMax(2, bins); }
void SubspaceCluster6::setDensityThreshold(double tau) { m_tau = qBound(0.01, tau, 1.0); }
void SubspaceCluster6::setMaxDimensions(int maxDims) { m_maxDims = qMax(1, maxDims); }

/* ---- Map value to grid bin ---- */

int SubspaceCluster6::toBin(double value, const QVector<double>& boundaries) const
{
    for (int i = 0; i < boundaries.size() - 1; ++i)
        if (value < boundaries[i + 1]) return i;
    return boundaries.size() - 2;
}

/* ---- Compute adaptive grid boundaries ---- */

QVector<double> SubspaceCluster6::adaptiveGrid(const QVector<double>& values) const
{
    if (values.isEmpty()) return {};
    double minV = values[0], maxV = values[0];
    for (double v : values) { minV = qMin(minV, v); maxV = qMax(maxV, v); }
    double range = maxV - minV;
    if (range < 1e-12) range = 1.0;

    // Adaptive: use percentile-based bin edges
    QVector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    QVector<double> boundaries(m_bins + 1);
    for (int i = 0; i <= m_bins; ++i) {
        double frac = static_cast<double>(i) / m_bins;
        int idx = qBound(0, static_cast<int>(frac * (sorted.size() - 1)), sorted.size() - 1);
        boundaries[i] = sorted[idx];
    }
    boundaries[0] = minV - 1e-9;
    boundaries[m_bins] = maxV + 1e-9;
    return boundaries;
}

/* ---- Find 1-D dense units ---- */

QVector<QVector<SubspaceCluster6::DenseUnit>> SubspaceCluster6::findDense1D(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int d = (n > 0) ? data[0].size() : 0;
    QVector<QVector<DenseUnit>> result(d);

    for (int dim = 0; dim < d; ++dim) {
        // Extract column
        QVector<double> col(n);
        for (int i = 0; i < n; ++i) col[i] = data[i][dim];

        QVector<double> grid = adaptiveGrid(col);

        // Count points per bin
        QVector<int> counts(m_bins, 0);
        for (int i = 0; i < n; ++i) counts[toBin(col[i], grid)]++;

        double threshold = m_tau * n;
        for (int bin = 0; bin < m_bins; ++bin) {
            if (counts[bin] >= threshold) {
                DenseUnit du;
                du.dims = {dim};
                du.gridIdx = {bin};
                du.count = counts[bin];
                result[dim].append(du);
            }
        }
    }
    return result;
}

/* ---- Check adjacency ---- */

bool SubspaceCluster6::adjacent(const DenseUnit& a, const DenseUnit& b) const
{
    if (a.dims.size() != b.dims.size()) return false;
    int diff = 0;
    for (int i = 0; i < a.dims.size(); ++i) {
        if (a.dims[i] != b.dims[i]) return false;
        diff += qAbs(a.gridIdx[i] - b.gridIdx[i]);
    }
    return diff <= 1;
}

/* ---- Count points in unit ---- */

int SubspaceCluster6::countInUnit(const DenseUnit& unit, const QVector<QVector<double>>& data,
                                    const QVector<QVector<double>>& grids) const
{
    int count = 0;
    for (int i = 0; i < data.size(); ++i) {
        bool inside = true;
        for (int d = 0; d < unit.dims.size() && inside; ++d) {
            int dim = unit.dims[d];
            int bin = toBin(data[i][dim], grids[dim]);
            if (bin != unit.gridIdx[d]) inside = false;
        }
        if (inside) count++;
    }
    return count;
}

/* ---- Mine higher-dimensional dense units ---- */

QVector<SubspaceCluster6::DenseUnit> SubspaceCluster6::mineHigherDims(
    const QVector<QVector<DenseUnit>>& dense1D,
    const QVector<QVector<double>>& data) const
{
    Q_UNUSED(data)
    QVector<DenseUnit> allUnits;
    // Collect 1-D dense units
    for (const auto& units : dense1D)
        for (const auto& u : units) allUnits.append(u);

    // Candidate generation: join pairs sharing all but one dimension
    for (int depth = 1; depth < m_maxDims; ++depth) {
        QVector<DenseUnit> candidates;
        for (int i = 0; i < allUnits.size(); ++i)
            for (int j = i + 1; j < allUnits.size(); ++j) {
                // Try merge: union of dimensions, check compatibility
                const DenseUnit& a = allUnits[i];
                const DenseUnit& b = allUnits[j];
                if (a.dims.size() != depth || b.dims.size() != depth) continue;

                // Check shared dimensions match
                bool compatible = true;
                for (int d = 0; d < a.dims.size() && compatible; ++d) {
                    bool found = false;
                    for (int e = 0; e < b.dims.size(); ++e)
                        if (a.dims[d] == b.dims[e] && a.gridIdx[d] == b.gridIdx[e]) { found = true; break; }
                    if (!found && a.dims[d] != b.dims[d < b.dims.size() ? d : 0]) compatible = true;
                }
                if (!compatible) continue;

                DenseUnit merged;
                merged.dims = a.dims;
                merged.gridIdx = a.gridIdx;
                // Add differing dimension from b
                for (int d = 0; d < b.dims.size(); ++d) {
                    bool exists = false;
                    for (int e = 0; e < merged.dims.size(); ++e)
                        if (merged.dims[e] == b.dims[d]) { exists = true; break; }
                    if (!exists) { merged.dims.append(b.dims[d]); merged.gridIdx.append(b.gridIdx[d]); }
                }
                merged.count = qMin(a.count, b.count);
                if (merged.dims.size() == depth + 1) candidates.append(merged);
            }
        if (candidates.isEmpty()) break;
        allUnits.append(candidates);
    }
    return allUnits;
}

/* ---- Merge clusters ---- */

QVector<int> SubspaceCluster6::mergeClusters(const QVector<DenseUnit>& units,
                                               const QVector<QVector<double>>& data) const
{
    int n = data.size();
    if (n == 0) return {};
    QVector<int> labels(n, -1);

    // Union-Find over dense units
    QVector<int> parent(units.size());
    for (int i = 0; i < parent.size(); ++i) parent[i] = i;

    auto find = [&](int x) {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };
    auto unite = [&](int x, int y) { parent[find(x)] = find(y); };

    for (int i = 0; i < units.size(); ++i)
        for (int j = i + 1; j < units.size(); ++j)
            if (adjacent(units[i], units[j])) unite(i, j);

    // Map component labels to data points
    QMap<int, int> compMap;
    int nextLabel = 0;
    for (int i = 0; i < n; ++i) {
        for (int u = 0; u < units.size(); ++u) {
            bool inside = true;
            for (int d = 0; d < units[u].dims.size() && inside; ++d) {
                int dim = units[u].dims[d];
                if (dim >= data[i].size()) { inside = false; continue; }
                // Simple range check
                if (qFloor(data[i][dim] * m_bins) != units[u].gridIdx[d]) inside = false;
            }
            if (inside) {
                int root = find(u);
                if (!compMap.contains(root)) compMap[root] = nextLabel++;
                labels[i] = compMap[root];
                break;
            }
        }
    }
    return labels;
}

/* ---- Fit ---- */

QVector<int> SubspaceCluster6::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    QVector<QVector<DenseUnit>> dense1D = findDense1D(data);
    m_denseUnits = mineHigherDims(dense1D, data);
    QVector<int> labels = mergeClusters(m_denseUnits, data);

    m_stats.totalFits++;
    m_stats.numSamples = n;
    m_stats.numClusters = 0;
    for (int l : labels) if (l >= m_stats.numClusters) m_stats.numClusters = l + 1;
    m_stats.numDenseUnits = m_denseUnits.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit clusteringCompleted(m_stats.numClusters, m_denseUnits.size(), timer.elapsed());
    return labels;
}

QVector<SubspaceCluster6::DenseUnit> SubspaceCluster6::denseUnits() const { return m_denseUnits; }

/* ---- Reset ---- */

void SubspaceCluster6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_denseUnits.clear();
}
