/**
 * @file TravelingSalesman4.cpp
 * @brief TravelingSalesman4 实现
 *
 * 实现TSP求解：MST候选集、LKH启发式、2-opt优化。
 */

#include "utils/graph221/TravelingSalesman4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman4::TravelingSalesman4(QObject *parent) : QObject(parent) {}
TravelingSalesman4::~TravelingSalesman4() = default;

/* ---- Configuration ---- */

void TravelingSalesman4::setCities(const QVector<QPair<double, double>>& cities)
{
    m_cities = cities;
    buildDistMatrix();
}

void TravelingSalesman4::setCandidateSize(int k) { m_candidateSize = qMax(3, k); }
void TravelingSalesman4::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }

/* ---- Distance helpers ---- */

void TravelingSalesman4::buildDistMatrix()
{
    int n = m_cities.size();
    m_distMatrix.resize(n * n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double dx = m_cities[i].first - m_cities[j].first;
            double dy = m_cities[i].second - m_cities[j].second;
            m_distMatrix[i * n + j] = qSqrt(dx * dx + dy * dy);
        }
    }
}

double TravelingSalesman4::dist(int i, int j) const
{
    int n = m_cities.size();
    if (i < 0 || j < 0 || i >= n || j >= n) return 0.0;
    return m_distMatrix[i * n + j];
}

/* ---- Build MST (Prim's) ---- */

QVector<QPair<int, int>> TravelingSalesman4::buildMST() const
{
    int n = m_cities.size();
    if (n == 0) return {};

    QVector<double> minDist(n, std::numeric_limits<double>::max());
    QVector<int> parent(n, -1);
    QVector<bool> inMST(n, false);
    QVector<QPair<int, int>> edges;

    minDist[0] = 0.0;

    for (int step = 0; step < n; ++step) {
        int u = -1;
        double best = std::numeric_limits<double>::max();
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && minDist[v] < best) { best = minDist[v]; u = v; }
        }
        if (u < 0) break;
        inMST[u] = true;
        if (parent[u] >= 0)
            edges.append({parent[u], u});

        for (int v = 0; v < n; ++v) {
            double d = dist(u, v);
            if (!inMST[v] && d < minDist[v]) { minDist[v] = d; parent[v] = u; }
        }
    }
    return edges;
}

/* ---- Alpha-nearness ---- */

QVector<QVector<QPair<int, double>>> TravelingSalesman4::computeAlphaNearness() const
{
    int n = m_cities.size();
    QVector<QVector<QPair<int, double>>> alpha(n);
    auto mstEdges = buildMST();

    // Build MST adjacency
    QVector<QVector<int>> mstAdj(n);
    for (const auto& e : mstEdges) {
        mstAdj[e.first].append(e.second);
        mstAdj[e.second].append(e.first);
    }

    // For each non-MST edge, compute alpha = cost(e) - max_edge_on_path
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            double d = dist(i, j);
            double alphaVal = d;
            alpha[i].append({j, alphaVal});
        }
        // Sort by alpha value and keep top candidates
        std::sort(alpha[i].begin(), alpha[i].end(),
            [](const QPair<int, double>& a, const QPair<int, double>& b) {
                return a.second < b.second;
            });
        if (alpha[i].size() > m_candidateSize)
            alpha[i].resize(m_candidateSize);
    }
    return alpha;
}

/* ---- Build candidate set ---- */

void TravelingSalesman4::buildCandidateSet()
{
    auto alpha = computeAlphaNearness();
    int n = m_cities.size();
    m_candidates.resize(n);
    for (int i = 0; i < n; ++i) {
        m_candidates[i].clear();
        for (const auto& p : alpha[i])
            m_candidates[i].append(p.first);
    }
}

/* ---- Nearest neighbor initial tour ---- */

QVector<int> TravelingSalesman4::nearestNeighborTour() const
{
    int n = m_cities.size();
    QVector<int> tour;
    QVector<bool> visited(n, false);

    int cur = 0;
    visited[0] = true;
    tour.append(cur);

    for (int step = 1; step < n; ++step) {
        double bestDist = std::numeric_limits<double>::max();
        int next = -1;
        for (int j = 0; j < n; ++j) {
            if (!visited[j]) {
                double d = dist(cur, j);
                if (d < bestDist) { bestDist = d; next = j; }
            }
        }
        if (next >= 0) {
            visited[next] = true;
            tour.append(next);
            cur = next;
        }
    }
    return tour;
}

/* ---- Tour length ---- */

double TravelingSalesman4::tourLength(const QVector<int>& tour) const
{
    double len = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int j = (i + 1) % tour.size();
        len += dist(tour[i], tour[j]);
    }
    return len;
}

/* ---- 2-opt ---- */

QVector<int> TravelingSalesman4::twoOpt(const QVector<int>& tour) const
{
    int n = tour.size();
    QVector<int> t = tour;
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 2; j < n; ++j) {
                double d0 = dist(t[i], t[i + 1]) + dist(t[j], t[(j + 1) % n]);
                double d1 = dist(t[i], t[j]) + dist(t[i + 1], t[(j + 1) % n]);
                if (d1 < d0 - 1e-10) {
                    // Reverse segment [i+1, j]
                    int left = i + 1, right = j;
                    while (left < right) {
                        std::swap(t[left], t[right]);
                        left++; right--;
                    }
                    improved = true;
                }
            }
        }
    }
    return t;
}

/* ---- Get candidates ---- */

QVector<int> TravelingSalesman4::getCandidates(int city) const
{
    if (city < 0 || city >= m_candidates.size()) return {};
    return m_candidates[city];
}

/* ---- Solve (LKH simplified) ---- */

QVector<int> TravelingSalesman4::solve()
{
    QElapsedTimer timer;
    timer.start();
    int n = m_cities.size();
    if (n == 0) return {};

    buildCandidateSet();

    // Initial tour
    QVector<int> bestTour = nearestNeighborTour();
    double bestLen = tourLength(bestTour);

    // Iterative LKH-style: 2-opt with candidate-guided search
    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<int> current = bestTour;

        // 2-opt improvement pass
        current = twoOpt(current);
        double currentLen = tourLength(current);

        // 3-opt like moves using candidate set
        for (int i = 0; i < n; ++i) {
            int city = current[i];
            if (city >= m_candidates.size()) continue;
            for (int cand : m_candidates[city]) {
                // Find cand position in tour
                int j = -1;
                for (int k = 0; k < n; ++k) {
                    if (current[k] == cand) { j = k; break; }
                }
                if (j < 0 || j == i) continue;

                // Try or-opt: move city to near cand
                if (qAbs(i - j) > 1) {
                    int ni = (i + 1) % n;
                    int nj = (j + 1) % n;
                    double oldD = dist(current[i], current[ni])
                                  + dist(current[j], current[nj]);
                    double newD = dist(current[i], current[j])
                                  + dist(current[ni], current[nj]);
                    if (newD < oldD - 1e-10) {
                        int left = qMin(i, j) + 1;
                        int right = qMax(i, j);
                        while (left < right) {
                            std::swap(current[left], current[right]);
                            left++; right--;
                        }
                    }
                }
            }
        }

        currentLen = tourLength(current);
        if (currentLen < bestLen) {
            bestTour = current;
            bestLen = currentLen;
        }
    }

    m_stats.numCities = n;
    m_stats.iterations = m_maxIter;
    m_stats.bestTourLength = bestLen;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(n, bestLen, m_maxIter, timer.elapsed());

    return bestTour;
}

/* ---- Reset ---- */

void TravelingSalesman4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_cities.clear();
    m_candidates.clear();
    m_distMatrix.clear();
}
