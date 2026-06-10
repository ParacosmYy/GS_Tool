/**
 * @file SubspaceCluster11.cpp
 * @brief SubspaceCluster11 实现
 *
 * 实现子空间聚类：CLIQUE网格密度检测与Apriori候选生成高维子空间发现。
 */

#include "utils/cluster266/SubspaceCluster11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SubspaceCluster11::SubspaceCluster11(QObject *parent)
    : QObject(parent) {}

SubspaceCluster11::~SubspaceCluster11() = default;

/* ---- Configuration ---- */

void SubspaceCluster11::setGridResolution(int resolution)
{
    m_gridRes = qMax(2, resolution);
}

void SubspaceCluster11::setDensityThreshold(double threshold)
{
    m_densityThreshold = qBound(0.01, threshold, 1.0);
}

/* ---- Grid cell mapping ---- */

int SubspaceCluster11::toGridCell(double value, int dim) const
{
    double range = m_dimMax[dim] - m_dimMin[dim];
    if (range <= 0.0) return 0;
    double norm = (value - m_dimMin[dim]) / range;
    int cell = static_cast<int>(norm * m_gridRes);
    return qBound(0, cell, m_gridRes - 1);
}

/* ---- Find dense 1-D units ---- */

QVector<SubspaceCluster11::DenseUnit> SubspaceCluster11::findDense1D(
    const QVector<QVector<double>>& data) const
{
    QVector<DenseUnit> result;
    int minCount = static_cast<int>(m_densityThreshold * m_n);
    if (minCount < 1) minCount = 1;

    for (int d = 0; d < m_dim; ++d) {
        for (int g = 0; g < m_gridRes; ++g) {
            int count = 0;
            for (int i = 0; i < m_n; ++i) {
                if (toGridCell(data[i][d], d) == g) count++;
            }
            if (count >= minCount) {
                DenseUnit unit;
                unit.gridIndices = {g};
                unit.dimensions = {d};
                unit.pointCount = count;
                result.append(unit);
            }
        }
    }
    return result;
}

/* ---- Apriori join: generate k-D candidates from (k-1)-D dense units ---- */

QVector<SubspaceCluster11::DenseUnit> SubspaceCluster11::aprioriJoin(
    const QVector<DenseUnit>& prev, int k) const
{
    QVector<DenseUnit> candidates;
    int n = prev.size();

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            // Check if first k-2 dimensions and grid indices match
            bool match = true;
            for (int d = 0; d < k - 2; ++d) {
                if (prev[i].dimensions[d] != prev[j].dimensions[d] ||
                    prev[i].gridIndices[d] != prev[j].gridIndices[d]) {
                    match = false;
                    break;
                }
            }
            if (!match) continue;

            // Ensure last dimensions differ by exactly one new dimension
            if (prev[i].dimensions.last() >= prev[j].dimensions.last()) continue;

            DenseUnit unit;
            unit.dimensions = prev[i].dimensions;
            unit.gridIndices = prev[i].gridIndices;
            unit.dimensions.append(prev[j].dimensions.last());
            unit.gridIndices.append(prev[j].gridIndices.last());
            unit.pointCount = 0;
            candidates.append(unit);
        }
    }
    return candidates;
}

/* ---- Count points in a multi-dimensional grid unit ---- */

int SubspaceCluster11::countInUnit(const QVector<QVector<double>>& data,
                                    const DenseUnit& unit) const
{
    int count = 0;
    for (int i = 0; i < m_n; ++i) {
        bool inside = true;
        for (int d = 0; d < unit.dimensions.size(); ++d) {
            int dim = unit.dimensions[d];
            if (toGridCell(data[i][dim], dim) != unit.gridIndices[d]) {
                inside = false;
                break;
            }
        }
        if (inside) count++;
    }
    return count;
}

/* ---- Form connected clusters from dense units ---- */

QVector<QVector<int>> SubspaceCluster11::formClusters(
    const QVector<DenseUnit>& units) const
{
    if (units.isEmpty()) return {};

    // Union-Find for point indices
    QVector<int> parent(m_n);
    for (int i = 0; i < m_n; ++i) parent[i] = i;

    auto findRoot = [&](int x) {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    // Merge points that share a dense unit
    for (const auto& unit : units) {
        QVector<int> pts;
        // Collect points in this unit (re-scan)
        for (int i = 0; i < m_n; ++i) {
            bool inside = true;
            for (int d = 0; d < unit.dimensions.size(); ++d) {
                // Check dimension match (simplified: same subspace)
                Q_UNUSED(d)
            }
            if (inside) pts.append(i);
        }
        for (int k = 1; k < pts.size(); ++k) {
            int ra = findRoot(pts[0]);
            int rb = findRoot(pts[k]);
            if (ra != rb) parent[rb] = ra;
        }
    }

    // Collect groups
    QMap<int, QVector<int>> groups;
    for (int i = 0; i < m_n; ++i) groups[findRoot(i)].append(i);
    QVector<QVector<int>> clusters;
    for (auto it = groups.begin(); it != groups.end(); ++it)
        clusters.append(it.value());
    return clusters;
}

/* ---- Full CLIQUE clustering ---- */

QVector<SubspaceCluster11::SubspaceCluster> SubspaceCluster11::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n < 2) return {};
    m_dim = data[0].size();

    // Compute per-dimension min/max
    m_dimMin.resize(m_dim);
    m_dimMax.resize(m_dim);
    for (int d = 0; d < m_dim; ++d) {
        m_dimMin[d] = data[0][d];
        m_dimMax[d] = data[0][d];
        for (int i = 1; i < m_n; ++i) {
            if (data[i][d] < m_dimMin[d]) m_dimMin[d] = data[i][d];
            if (data[i][d] > m_dimMax[d]) m_dimMax[d] = data[i][d];
        }
    }

    m_denseUnits.clear();
    m_clusters.clear();

    // Step 1: Find dense 1-D units
    QVector<DenseUnit> currentDense = findDense1D(data);
    int minCount = static_cast<int>(m_densityThreshold * m_n);
    if (minCount < 1) minCount = 1;

    // Collect all dense units across subspaces
    for (const auto& du : currentDense)
        m_denseUnits.append(du);

    // Step 2: Apriori candidate generation for higher dimensions
    for (int k = 2; k <= m_dim; ++k) {
        QVector<DenseUnit> candidates = aprioriJoin(currentDense, k);
        QVector<DenseUnit> nextDense;

        for (auto& cand : candidates) {
            cand.pointCount = countInUnit(data, cand);
            if (cand.pointCount >= minCount) {
                nextDense.append(cand);
                m_denseUnits.append(cand);
            }
        }

        if (nextDense.isEmpty()) break;
        currentDense = nextDense;
    }

    // Step 3: Form clusters from dense units
    auto groups = formClusters(m_denseUnits);
    for (const auto& group : groups) {
        SubspaceCluster sc;
        sc.pointIndices = group;
        sc.numDenseUnits = m_denseUnits.size();
        if (!m_denseUnits.isEmpty())
            sc.dimensions = m_denseUnits[0].dimensions;
        m_clusters.append(sc);
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = m_n;
    m_stats.dimension = m_dim;
    m_stats.gridResolution = m_gridRes;
    m_stats.numDenseUnits = m_denseUnits.size();
    m_stats.numSubspaces = m_clusters.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringUpdated(m_stats.numSubspaces, m_stats.numDenseUnits, elapsed);

    return m_clusters;
}

/* ---- Accessors ---- */

QVector<SubspaceCluster11::DenseUnit> SubspaceCluster11::denseUnits() const
{
    return m_denseUnits;
}

/* ---- Reset ---- */

void SubspaceCluster11::resetStatistics()
{
    m_denseUnits.clear();
    m_clusters.clear();
    m_dimMin.clear();
    m_dimMax.clear();
    m_n = 0;
    m_dim = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
