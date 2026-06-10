/**
 * @file Agglomerative14.cpp
 * @brief Agglomerative14 实现
 *
 * 实现层次凝聚聚类：灵活Lance-Williams更新公式与共表相关性树状图质量评估。
 */

#include "utils/cluster273/Agglomerative14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Agglomerative14::Agglomerative14(QObject *parent)
    : QObject(parent) {}

Agglomerative14::~Agglomerative14() = default;

/* ---- Configuration ---- */

void Agglomerative14::setLinkage(Linkage method) { m_linkage = method; }

void Agglomerative14::setFlexibleParams(double beta, double gamma)
{
    m_beta = beta;
    m_gamma = gamma;
}

void Agglomerative14::setTargetClusters(int n) { m_targetClusters = qBound(1, n, 1000); }

/* ---- Pairwise Euclidean distance (condensed upper triangle) ---- */

QVector<double> Agglomerative14::pairwiseDistances(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    int len = n * (n - 1) / 2;
    QVector<double> dist(len, 0.0);
    int idx = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dSq = 0.0;
            for (int k = 0; k < data[i].size(); ++k) {
                double diff = data[i][k] - data[j][k];
                dSq += diff * diff;
            }
            dist[idx++] = qSqrt(dSq);
        }
    }
    return dist;
}

/* ---- Lance-Williams coefficients ---- */

void Agglomerative14::lanceWilliamsCoeffs(int ni, int nj, int nk,
                                            double& ai, double& aj, double& b, double& g) const
{
    Q_UNUSED(nk);
    double total = ni + nj;
    switch (m_linkage) {
    case SingleLinkage:
        ai = 0.5; aj = 0.5; b = 0.0; g = -0.5; break;
    case CompleteLinkage:
        ai = 0.5; aj = 0.5; b = 0.0; g = 0.5; break;
    case AverageLinkage:
        ai = ni / total; aj = nj / total; b = 0.0; g = 0.0; break;
    case WardLinkage:
        ai = static_cast<double>(ni + nk) / (total + nk);
        aj = static_cast<double>(nj + nk) / (total + nk);
        b = -static_cast<double>(nk) / (total + nk);
        g = 0.0; break;
    case FlexibleLinkage:
        ai = (1.0 - m_beta) / 2.0;
        aj = (1.0 - m_beta) / 2.0;
        b = m_beta;
        g = m_gamma; break;
    }
}

/* ---- Find index of minimum active distance ---- */

int Agglomerative14::findMinDist(const QVector<double>& dist,
                                  const QVector<bool>& active, int n) const
{
    double minVal = 1e18;
    int minIdx = -1;
    for (int i = 0; i < n; ++i) {
        if (!active[i]) continue;
        for (int j = i + 1; j < n; ++j) {
            if (!active[j]) continue;
            int dIdx = i * n - i * (i + 1) / 2 + j - i - 1;
            if (dIdx >= 0 && dIdx < dist.size() && dist[dIdx] < minVal) {
                minVal = dist[dIdx];
                minIdx = dIdx;
            }
        }
    }
    return minIdx;
}

/* ---- Build original distance vector for cophenetic ---- */

void Agglomerative14::buildOriginalDistances(const QVector<QVector<double>>& data)
{
    m_originalDist = pairwiseDistances(data);
}

/* ---- Main fit: agglomerative merging ---- */

QVector<int> Agglomerative14::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    QVector<double> dist = pairwiseDistances(data);
    buildOriginalDistances(data);

    // Track active clusters and sizes
    QVector<bool> active(n, true);
    QVector<int> sizes(n, 1);
    QVector<int> label(n);
    for (int i = 0; i < n; ++i) label[i] = i;

    m_merges.clear();
    int numClusters = n;

    // Map from flat index to (i,j)
    auto flatToPair = [n](int idx) -> QPair<int,int> {
        int i = 0;
        while (idx >= n - i - 1) { idx -= n - i - 1; i++; }
        return {i, i + 1 + idx};
    };

    while (numClusters > m_targetClusters) {
        // Find minimum distance pair
        int minIdx = findMinDist(dist, active, n);
        if (minIdx < 0) break;

        auto pair = flatToPair(minIdx);
        int ci = pair.first, cj = pair.second;

        // Record merge step
        MergeStep step;
        step.clusterI = ci;
        step.clusterJ = cj;
        step.distance = dist[minIdx];
        step.newSize = sizes[ci] + sizes[cj];
        m_merges.append(step);

        // Update distances using Lance-Williams formula
        for (int k = 0; k < n; ++k) {
            if (!active[k] || k == ci || k == cj) continue;
            int ikIdx = qMin(ci, k) * n - qMin(ci, k) * (qMin(ci, k) + 1) / 2
                        + qMax(ci, k) - qMin(ci, k) - 1;
            int jkIdx = qMin(cj, k) * n - qMin(cj, k) * (qMin(cj, k) + 1) / 2
                        + qMax(cj, k) - qMin(cj, k) - 1;
            int ijIdx = qMin(ci, cj) * n - qMin(ci, cj) * (qMin(ci, cj) + 1) / 2
                        + qMax(ci, cj) - qMin(ci, cj) - 1;

            double dik = (ikIdx >= 0 && ikIdx < dist.size()) ? dist[ikIdx] : 0.0;
            double djk = (jkIdx >= 0 && jkIdx < dist.size()) ? dist[jkIdx] : 0.0;
            double dij = (ijIdx >= 0 && ijIdx < dist.size()) ? dist[ijIdx] : 0.0;

            double ai, aj, b, g;
            lanceWilliamsCoeffs(sizes[ci], sizes[cj], sizes[k], ai, aj, b, g);
            double newDist = ai * dik + aj * djk + b * dij
                             + g * qAbs(dik - djk);

            // Store updated distance at ci-k position
            if (ikIdx >= 0 && ikIdx < dist.size()) dist[ikIdx] = newDist;
        }

        // Merge cj into ci
        active[cj] = false;
        sizes[ci] += sizes[cj];
        for (int i = 0; i < n; ++i) {
            if (label[i] == cj) label[i] = ci;
        }
        numClusters--;
    }

    // Relabel to 0..numClusters-1
    QMap<int, int> remap;
    int cid = 0;
    for (int i = 0; i < n; ++i) {
        if (!remap.contains(label[i])) remap[label[i]] = cid++;
        label[i] = remap[label[i]];
    }

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.numMerges = m_merges.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringDone(numClusters, m_stats.copheneticCorrelation, elapsed);

    return label;
}

/* ---- Dendrogram access ---- */

QVector<Agglomerative14::MergeStep> Agglomerative14::dendrogram() const { return m_merges; }

/* ---- Cophenetic correlation coefficient ---- */

double Agglomerative14::copheneticCorrelation(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n < 2 || m_merges.isEmpty()) return 0.0;

    // Build cophenetic distance matrix from dendrogram
    QVector<double> cophDist(n * (n - 1) / 2, 0.0);
    // Union-find to track merges
    QVector<int> parent(n);
    for (int i = 0; i < n; ++i) parent[i] = i;

    auto find = [&parent](int x) -> int {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };

    for (const auto& merge : m_merges) {
        int ri = find(merge.clusterI);
        int rj = find(merge.clusterJ);
        // Set cophenetic distance for all cross-pairs
        for (int a = 0; a < n; ++a) {
            if (find(a) != ri) continue;
            for (int b = a + 1; b < n; ++b) {
                if (find(b) != rj) continue;
                int idx = a * n - a * (a + 1) / 2 + b - a - 1;
                if (idx >= 0 && idx < cophDist.size()) cophDist[idx] = merge.distance;
            }
        }
        // Union
        int root = qMin(ri, rj);
        parent[ri] = root;
        parent[rj] = root;
    }

    // Pearson correlation between original and cophenetic distances
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;
    int m = cophDist.size();
    for (int i = 0; i < m; ++i) {
        double x = m_originalDist[i];
        double y = cophDist[i];
        sumX += x; sumY += y;
        sumXY += x * y;
        sumX2 += x * x; sumY2 += y * y;
    }
    double denom = qSqrt((m * sumX2 - sumX * sumX) * (m * sumY2 - sumY * sumY));
    m_stats.copheneticCorrelation = (denom > 1e-15) ? (m * sumXY - sumX * sumY) / denom : 0.0;
    return m_stats.copheneticCorrelation;
}

/* ---- Cut dendrogram at distance threshold ---- */

QVector<int> Agglomerative14::cutAtDistance(double threshold) const
{
    // Determine number of clusters from merge history
    int n = 0;
    for (const auto& m : m_merges)
        n = qMax(n, qMax(m.clusterI, m.clusterJ) + 1);
    if (n == 0) return {};

    QVector<int> label(n);
    for (int i = 0; i < n; ++i) label[i] = i;

    for (const auto& merge : m_merges) {
        if (merge.distance > threshold) break;
        int target = label[merge.clusterI];
        int source = label[merge.clusterJ];
        for (int i = 0; i < n; ++i)
            if (label[i] == source) label[i] = target;
    }

    // Relabel
    QMap<int, int> remap;
    int cid = 0;
    for (int i = 0; i < n; ++i) {
        if (!remap.contains(label[i])) remap[label[i]] = cid++;
        label[i] = remap[label[i]];
    }
    return label;
}

/* ---- Reset ---- */

void Agglomerative14::resetStatistics()
{
    m_merges.clear();
    m_originalDist.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
