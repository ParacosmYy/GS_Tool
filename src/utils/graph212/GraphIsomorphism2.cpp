/**
 * @file GraphIsomorphism2.cpp
 * @brief GraphIsomorphism2 实现
 *
 * 实现图同构检测：颜色精炼、2维Weisfeiler-Lehman测试、不变量计算。
 */

#include "utils/graph212/GraphIsomorphism2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QCryptographicHash>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GraphIsomorphism2::GraphIsomorphism2(QObject *parent) : QObject(parent) {}
GraphIsomorphism2::~GraphIsomorphism2() = default;

/* ---- Configuration ---- */

void GraphIsomorphism2::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void GraphIsomorphism2::setUse2WL(bool enable) { m_use2WL = enable; }

/* ---- Hash helper ---- */

quint64 GraphIsomorphism2::hashTuple(const QVector<int>& tuple) const
{
    QByteArray data;
    for (int v : tuple) {
        data.append(reinterpret_cast<const char*>(&v), sizeof(int));
    }
    auto hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    quint64 result = 0;
    for (int i = 0; i < 8; ++i)
        result = (result << 8) | static_cast<quint64>(static_cast<unsigned char>(hash[i]));
    return result;
}

/* ---- Quick invariant check ---- */

bool GraphIsomorphism2::checkInvariants(const Graph& g1, const Graph& g2) const
{
    if (g1.numVertices != g2.numVertices) return false;

    // Compare degree sequences
    auto d1 = degreeSequence(g1);
    auto d2 = degreeSequence(g2);
    return d1 == d2;
}

/* ---- Degree sequence ---- */

QVector<int> GraphIsomorphism2::degreeSequence(const Graph& g) const
{
    QVector<int> degrees(g.numVertices);
    for (int i = 0; i < g.numVertices; ++i)
        degrees[i] = g.adjacency[i].size();
    std::sort(degrees.begin(), degrees.end(), std::greater<int>());
    return degrees;
}

/* ---- 1-D WL color refinement ---- */

QVector<int> GraphIsomorphism2::colorRefine1D(const Graph& g) const
{
    int n = g.numVertices;
    if (n == 0) return {};

    // Initial coloring by degree
    QVector<int> colors(n);
    for (int i = 0; i < n; ++i)
        colors[i] = g.adjacency[i].size();

    // Iterate refinement
    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<QPair<int, QVector<int>>> signatures(n);
        for (int i = 0; i < n; ++i) {
            QVector<int> neighborColors;
            for (int nb : g.adjacency[i])
                neighborColors.append(colors[nb]);
            std::sort(neighborColors.begin(), neighborColors.end());
            signatures[i] = {colors[i], neighborColors};
        }

        // Relabel colors
        QVector<quint64> hashes(n);
        for (int i = 0; i < n; ++i) {
            QVector<int> tuple;
            tuple.append(signatures[i].first);
            tuple.append(signatures[i].second.size());
            for (int c : signatures[i].second) tuple.append(c);
            hashes[i] = hashTuple(tuple);
        }

        // Map hashes to consecutive integers
        QVector<quint64> sortedHashes = hashes;
        std::sort(sortedHashes.begin(), sortedHashes.end());
        sortedHashes.erase(std::unique(sortedHashes.begin(), sortedHashes.end()),
                          sortedHashes.end());

        QVector<int> newColors(n);
        for (int i = 0; i < n; ++i) {
            newColors[i] = std::lower_bound(sortedHashes.begin(), sortedHashes.end(),
                                            hashes[i]) - sortedHashes.begin();
        }

        if (newColors == colors) break;
        colors = newColors;
    }
    return colors;
}

/* ---- 2-D WL color refinement ---- */

QVector<QVector<int>> GraphIsomorphism2::colorRefine2D(const Graph& g) const
{
    int n = g.numVertices;
    if (n == 0) return {};

    // Initial 2D coloring: color[u][v] based on (is_same, deg_u, deg_v, edge_exists)
    QVector<QVector<int>> colors(n, QVector<int>(n, 0));
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            int same = (u == v) ? 1 : 0;
            int edge = g.adjacency[u].contains(v) ? 1 : 0;
            colors[u][v] = same * 100 + g.adjacency[u].size() * 10 + edge;
        }
    }

    for (int iter = 0; iter < qMin(m_maxIter, 30); ++iter) {
        QVector<QVector<int>> newColors(n, QVector<int>(n, 0));

        for (int u = 0; u < n; ++u) {
            for (int v = 0; v < n; ++v) {
                // Collect (colors[u][w], colors[w][v]) for all w
                QVector<int> sig;
                sig.append(colors[u][v]);
                QVector<QPair<int, int>> pairs;
                for (int w = 0; w < n; ++w)
                    pairs.append({colors[u][w], colors[w][v]});
                std::sort(pairs.begin(), pairs.end());
                for (const auto& p : pairs) {
                    sig.append(p.first);
                    sig.append(p.second);
                }
                newColors[u][v] = static_cast<int>(hashTuple(sig) % 1000000007);
            }
        }

        // Check convergence
        bool changed = false;
        for (int u = 0; u < n && !changed; ++u)
            for (int v = 0; v < n && !changed; ++v)
                if (newColors[u][v] != colors[u][v]) changed = true;

        if (!changed) break;
        colors = newColors;
    }
    return colors;
}

/* ---- Canonical form ---- */

QByteArray GraphIsomorphism2::canonicalForm(const Graph& g) const
{
    auto colors = colorRefine1D(g);
    QVector<int> sorted = colors;
    std::sort(sorted.begin(), sorted.end());

    QByteArray result;
    for (int c : sorted)
        result.append(reinterpret_cast<const char*>(&c), sizeof(int));
    return result;
}

/* ---- Eigen invariant ---- */

QVector<double> GraphIsomorphism2::powerIteration(
    const QVector<QVector<double>>& matrix, int k) const
{
    int n = matrix.size();
    if (n == 0) return {};

    QVector<double> eigenvalues;
    QVector<QVector<double>> A = matrix;

    for (int ev = 0; ev < k && ev < n; ++ev) {
        QVector<double> v(n, 1.0 / qSqrt(n));
        for (int iter = 0; iter < 100; ++iter) {
            QVector<double> Av(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    Av[i] += A[i][j] * v[j];
            double norm = 0.0;
            for (double x : Av) norm += x * x;
            norm = qSqrt(norm);
            if (norm < 1e-15) break;
            for (int i = 0; i < n; ++i) v[i] = Av[i] / norm;
        }
        // Rayleigh quotient
        double lambda = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                lambda += v[i] * A[i][j] * v[j];
        eigenvalues.append(lambda);

        // Deflate
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                A[i][j] -= lambda * v[i] * v[j];
    }
    std::sort(eigenvalues.begin(), eigenvalues.end());
    return eigenvalues;
}

QVector<double> GraphIsomorphism2::eigenInvariant(const Graph& g) const
{
    int n = g.numVertices;
    QVector<QVector<double>> adj(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int nb : g.adjacency[i])
            adj[i][nb] = 1.0;
    return powerIteration(adj, qMin(5, n));
}

/* ---- Isomorphism test ---- */

bool GraphIsomorphism2::isIsomorphic(const Graph& g1, const Graph& g2)
{
    QElapsedTimer timer;
    timer.start();

    // Quick reject via invariants
    if (!checkInvariants(g1, g2)) {
        m_stats.totalTests++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;
        emit testCompleted(false, 0, timer.elapsed());
        return false;
    }

    // Eigenvalue check
    auto e1 = eigenInvariant(g1);
    auto e2 = eigenInvariant(g2);
    bool eigenMatch = true;
    if (e1.size() == e2.size()) {
        for (int i = 0; i < e1.size(); ++i) {
            if (qAbs(e1[i] - e2[i]) > 1e-6) { eigenMatch = false; break; }
        }
    } else {
        eigenMatch = false;
    }
    if (!eigenMatch) {
        m_stats.totalTests++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;
        emit testCompleted(false, 0, timer.elapsed());
        return false;
    }

    bool result;
    int iterations = 0;

    if (m_use2WL) {
        auto c1 = colorRefine2D(g1);
        auto c2 = colorRefine2D(g2);

        // Compare canonical color multisets
        QVector<int> flat1, flat2;
        for (int i = 0; i < g1.numVertices; ++i)
            for (int j = 0; j < g1.numVertices; ++j)
                flat1.append(c1[i][j]);
        for (int i = 0; i < g2.numVertices; ++i)
            for (int j = 0; j < g2.numVertices; ++j)
                flat2.append(c2[i][j]);
        std::sort(flat1.begin(), flat1.end());
        std::sort(flat2.begin(), flat2.end());
        result = (flat1 == flat2);
        iterations = 30;
    } else {
        auto c1 = colorRefine1D(g1);
        auto c2 = colorRefine1D(g2);
        QVector<int> s1 = c1, s2 = c2;
        std::sort(s1.begin(), s1.end());
        std::sort(s2.begin(), s2.end());
        result = (s1 == s2);
        iterations = m_maxIter;
    }

    m_stats.totalTests++;
    m_stats.graphSize1 = g1.numVertices;
    m_stats.graphSize2 = g2.numVertices;
    m_stats.wlIterations = iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTests;

    emit testCompleted(result, iterations, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void GraphIsomorphism2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
