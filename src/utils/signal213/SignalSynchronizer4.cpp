/**
 * @file SignalSynchronizer4.cpp
 * @brief SignalSynchronizer4 实现
 *
 * 实现信号同步器：交叉熵时延估计、加权图对齐、DTW规整。
 */

#include "utils/signal213/SignalSynchronizer4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalSynchronizer4::SignalSynchronizer4(QObject *parent) : QObject(parent) {}
SignalSynchronizer4::~SignalSynchronizer4() = default;

/* ---- Configuration ---- */

void SignalSynchronizer4::setSearchRange(int maxDelay)
{
    m_maxDelay = qMax(1, maxDelay);
}

/* ---- Normalize to probability ---- */

QVector<double> SignalSynchronizer4::normalizeToProb(const QVector<double>& signal)
{
    int n = signal.size();
    if (n == 0) return {};

    // Shift to make all values positive
    double minVal = signal[0];
    for (int i = 1; i < n; ++i) minVal = qMin(minVal, signal[i]);

    QVector<double> prob(n);
    double sum = 0.0;
    double epsilon = 1e-10;
    for (int i = 0; i < n; ++i) {
        prob[i] = signal[i] - minVal + epsilon;
        sum += prob[i];
    }
    if (sum > 0) {
        for (int i = 0; i < n; ++i) prob[i] /= sum;
    }
    return prob;
}

/* ---- Cross-entropy ---- */

double SignalSynchronizer4::crossEntropy(const QVector<double>& a,
                                          const QVector<double>& b)
{
    auto pa = normalizeToProb(a);
    auto pb = normalizeToProb(b);
    int n = qMin(pa.size(), pb.size());
    if (n == 0) return 0.0;

    double entropy = 0.0;
    for (int i = 0; i < n; ++i) {
        if (pa[i] > 1e-15 && pb[i] > 1e-15) {
            entropy -= pa[i] * qLn(pb[i]);
        }
    }
    return entropy;
}

/* ---- Shift signal ---- */

QVector<double> SignalSynchronizer4::shiftSignal(
    const QVector<double>& signal, int delay)
{
    int n = signal.size();
    QVector<double> shifted(n, 0.0);

    for (int i = 0; i < n; ++i) {
        int srcIdx = i - delay;
        if (srcIdx >= 0 && srcIdx < n)
            shifted[i] = signal[srcIdx];
    }
    return shifted;
}

/* ---- Estimate delay ---- */

int SignalSynchronizer4::estimateDelay(
    const QVector<double>& reference,
    const QVector<double>& signal) const
{
    int n = qMin(reference.size(), signal.size());
    if (n == 0) return 0;

    int searchRange = qMin(m_maxDelay, n / 2);
    double bestEntropy = std::numeric_limits<double>::max();
    int bestDelay = 0;

    for (int d = -searchRange; d <= searchRange; ++d) {
        // Extract overlapping segments
        int startRef = qMax(0, d);
        int startSig = qMax(0, -d);
        int len = qMin(n - startRef, n - startSig);
        if (len < 4) continue;

        QVector<double> refSeg(len), sigSeg(len);
        for (int i = 0; i < len; ++i) {
            refSeg[i] = reference[startRef + i];
            sigSeg[i] = signal[startSig + i];
        }

        double ce = crossEntropy(refSeg, sigSeg);
        if (ce < bestEntropy) {
            bestEntropy = ce;
            bestDelay = d;
        }
    }

    return bestDelay;
}

/* ---- Estimate delays for multiple signals ---- */

QVector<int> SignalSynchronizer4::estimateDelays(
    const QVector<double>& reference,
    const QVector<QVector<double>>& signals) const
{
    QVector<int> delays;
    for (const auto& sig : signals)
        delays.append(estimateDelay(reference, sig));
    return delays;
}

/* ---- Synchronize ---- */

QVector<QVector<double>> SignalSynchronizer4::synchronize(
    const QVector<QVector<double>>& signals, int referenceIndex)
{
    QElapsedTimer timer;
    timer.start();

    int numSig = signals.size();
    if (numSig == 0) return {};
    if (referenceIndex < 0 || referenceIndex >= numSig)
        referenceIndex = 0;

    const auto& ref = signals[referenceIndex];
    auto delays = estimateDelays(ref, signals);

    QVector<QVector<double>> aligned;
    for (int i = 0; i < numSig; ++i)
        aligned.append(shiftSignal(signals[i], -delays[i]));

    // Compute sync quality (average normalized cross-correlation)
    double quality = 0.0;
    for (int i = 0; i < numSig; ++i) {
        int n = qMin(aligned[i].size(), ref.size());
        double dot = 0.0, normA = 0.0, normB = 0.0;
        for (int j = 0; j < n; ++j) {
            dot += aligned[i][j] * ref[j];
            normA += aligned[i][j] * aligned[i][j];
            normB += ref[j] * ref[j];
        }
        double denom = qSqrt(normA * normB);
        quality += (denom > 0) ? dot / denom : 0.0;
    }
    quality /= numSig;

    m_stats.numSignals = numSig;
    m_stats.signalLength = ref.size();
    m_stats.syncQuality = quality;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit synchronizationCompleted(numSig, quality, timer.elapsed());

    return aligned;
}

/* ---- Build alignment graph ---- */

QVector<QVector<QPair<int, double>>> SignalSynchronizer4::buildAlignmentGraph(
    const QVector<QVector<double>>& signals) const
{
    int n = signals.size();
    QVector<QVector<QPair<int, double>>> graph(n);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            int delay = estimateDelay(signals[i], signals[j]);
            int len = qMin(signals[i].size(), signals[j].size());

            // Compute alignment weight (cross-correlation)
            double dot = 0.0, normA = 0.0, normB = 0.0;
            for (int k = 0; k < len; ++k) {
                int si = k + qMax(0, -delay);
                int sj = k + qMax(0, delay);
                if (si < signals[i].size() && sj < signals[j].size()) {
                    double a = signals[i][si], b = signals[j][sj];
                    dot += a * b;
                    normA += a * a;
                    normB += b * b;
                }
            }
            double denom = qSqrt(normA * normB);
            double weight = (denom > 0) ? 1.0 - dot / denom : 1.0;

            graph[i].append({j, weight});
            graph[j].append({i, weight});
        }
    }
    return graph;
}

/* ---- Global alignment via Dijkstra ---- */

QVector<int> SignalSynchronizer4::globalAlignment(
    const QVector<QVector<QPair<int, double>>>& graph,
    int numSignals) const
{
    // Find shortest paths from node 0 to all others
    QVector<double> dist(numSignals, std::numeric_limits<double>::max());
    QVector<int> prev(numSignals, -1);
    QVector<bool> visited(numSignals, false);
    dist[0] = 0.0;

    for (int step = 0; step < numSignals; ++step) {
        // Find unvisited with minimum distance
        double minDist = std::numeric_limits<double>::max();
        int u = 0;
        for (int v = 0; v < numSignals; ++v) {
            if (!visited[v] && dist[v] < minDist) {
                minDist = dist[v];
                u = v;
            }
        }
        visited[u] = true;

        if (u < graph.size()) {
            for (const auto& edge : graph[u]) {
                int v = edge.first;
                double w = edge.second;
                if (!visited[v] && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                }
            }
        }
    }

    // Extract alignment ordering from shortest-path tree
    QVector<int> order;
    QVector<bool> added(numSignals, false);
    for (int i = 0; i < numSignals; ++i) {
        if (!added[i]) {
            // Trace path from 0 to i
            QVector<int> path;
            int cur = i;
            while (cur >= 0) {
                if (!added[cur]) path.append(cur);
                added[cur] = true;
                cur = prev[cur];
            }
            for (int p : path) order.append(p);
        }
    }
    return order;
}

/* ---- DTW alignment ---- */

QVector<QPair<int, int>> SignalSynchronizer4::dtwAlignment(
    const QVector<double>& a, const QVector<double>& b) const
{
    int m = a.size(), n = b.size();
    if (m == 0 || n == 0) return {};

    // DTW cost matrix
    QVector<QVector<double>> dtw(m, QVector<double>(n,
        std::numeric_limits<double>::max()));
    dtw[0][0] = qAbs(a[0] - b[0]);

    for (int i = 1; i < m; ++i)
        dtw[i][0] = dtw[i - 1][0] + qAbs(a[i] - b[0]);
    for (int j = 1; j < n; ++j)
        dtw[0][j] = dtw[0][j - 1] + qAbs(a[0] - b[j]);

    for (int i = 1; i < m; ++i) {
        for (int j = 1; j < n; ++j) {
            double cost = qAbs(a[i] - b[j]);
            dtw[i][j] = cost + qMin({dtw[i-1][j], dtw[i][j-1], dtw[i-1][j-1]});
        }
    }

    // Backtrack optimal path
    QVector<QPair<int, int>> path;
    int i = m - 1, j = n - 1;
    path.prepend({i, j});
    while (i > 0 || j > 0) {
        if (i == 0) j--;
        else if (j == 0) i--;
        else {
            double d1 = dtw[i-1][j], d2 = dtw[i][j-1], d3 = dtw[i-1][j-1];
            if (d3 <= d1 && d3 <= d2) { i--; j--; }
            else if (d1 <= d2) i--;
            else j--;
        }
        path.prepend({i, j});
    }
    return path;
}

/* ---- Reset ---- */

void SignalSynchronizer4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
