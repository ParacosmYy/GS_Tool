/**
 * @file GraphColoring12.cpp
 * @brief GraphColoring12 实现
 *
 * 实现图着色：进化变异与Tabu搜索元启发式着色数最小化。
 */

#include "utils/graph300/GraphColoring12.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphColoring12::GraphColoring12(QObject *parent)
    : QObject(parent) {}

GraphColoring12::~GraphColoring12() = default;

/* ---- Configuration ---- */

void GraphColoring12::setAdjacencyList(const QVector<QVector<int>>& adj)
{
    m_adj = adj;
    m_n = adj.size();
    m_adjMatrix.resize(m_n, QVector<bool>(m_n, false));
    for (int i = 0; i < m_n; ++i)
        for (int j : m_adj[i])
            if (j >= 0 && j < m_n) m_adjMatrix[i][j] = true;
}

void GraphColoring12::setAdjacencyMatrix(const QVector<QVector<int>>& matrix)
{
    m_n = matrix.size();
    m_adjMatrix.resize(m_n, QVector<bool>(m_n, false));
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_adj[i].clear();
        for (int j = 0; j < m_n; ++j) {
            if (matrix[i][j] != 0 && i != j) {
                m_adjMatrix[i][j] = true;
                m_adj[i].append(j);
            }
        }
    }
}

void GraphColoring12::setMaxIterations(int iters) { m_maxIter = qBound(10, iters, 100000); }
void GraphColoring12::setPopulationSize(int size) { m_popSize = qBound(4, size, 200); }
void GraphColoring12::setMutationRate(double rate) { m_mutationRate = qBound(0.01, rate, 0.5); }
void GraphColoring12::setTabuTenure(int tenure) { m_tabuTenure = qBound(1, tenure, 100); }

/* ---- Count conflicts ---- */

int GraphColoring12::countConflicts(const QVector<int>& colors) const
{
    int conflicts = 0;
    for (int i = 0; i < m_n; ++i)
        for (int j : m_adj[i])
            if (j > i && colors[i] == colors[j])
                conflicts++;
    return conflicts;
}

/* ---- Random coloring ---- */

QVector<int> GraphColoring12::randomColoring(int k, unsigned int& seed) const
{
    QVector<int> colors(m_n);
    for (int i = 0; i < m_n; ++i) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        colors[i] = static_cast<int>(seed % static_cast<unsigned int>(k));
    }
    return colors;
}

/* ---- DSATUR greedy coloring ---- */

QVector<int> GraphColoring12::dsaturColoring() const
{
    QVector<int> colors(m_n, -1);
    QVector<int> saturation(m_n, 0);
    QVector<bool> colored(m_n, false);

    for (int step = 0; step < m_n; ++step) {
        // Pick uncolored vertex with max saturation (break ties by degree)
        int best = -1, bestSat = -1, bestDeg = -1;
        for (int v = 0; v < m_n; ++v) {
            if (colored[v]) continue;
            if (saturation[v] > bestSat ||
                (saturation[v] == bestSat && m_adj[v].size() > bestDeg)) {
                bestSat = saturation[v];
                bestDeg = m_adj[v].size();
                best = v;
            }
        }
        if (best < 0) break;

        // Find smallest color not used by neighbors
        QVector<bool> usedC(m_n + 1, false);
        for (int nb : m_adj[best])
            if (colors[nb] >= 0) usedC[colors[nb]] = true;
        int c = 0;
        while (usedC[c]) c++;
        colors[best] = c;
        colored[best] = true;

        // Update saturation degrees
        for (int nb : m_adj[best]) {
            if (!colored[nb]) {
                QVector<bool> usedColors(m_n + 1, false);
                for (int nb2 : m_adj[nb])
                    if (colors[nb2] >= 0) usedColors[colors[nb2]] = true;
                saturation[nb] = 0;
                for (bool u : usedColors) if (u) saturation[nb]++;
            }
        }
    }
    return colors;
}

/* ---- Tabu search local improvement ---- */

QVector<int> GraphColoring12::tabuImprove(const QVector<int>& colors, int k, int maxIter)
{
    QVector<int> current = colors;
    int currentConf = countConflicts(current);

    // Tabu list: vertex -> color -> remaining tenure
    QVector<QVector<int>> tabuList(m_n, QVector<int>(k, 0));

    for (int iter = 0; iter < maxIter && currentConf > 0; ++iter) {
        // Find best move (vertex recolor) reducing conflicts
        int bestV = -1, bestC = -1, bestDelta = 0;
        for (int v = 0; v < m_n; ++v) {
            int oldColor = current[v];
            for (int c = 0; c < k; ++c) {
                if (c == oldColor) continue;
                // Evaluate delta: how many conflicts change
                int delta = 0;
                for (int nb : m_adj[v]) {
                    if (current[nb] == oldColor) delta--;
                    if (current[nb] == c) delta++;
                }
                bool isTabu = (tabuList[v][c] > 0);
                if ((!isTabu && delta < bestDelta) ||
                    (isTabu && currentConf + delta == 0 && delta < bestDelta)) {
                    bestDelta = delta;
                    bestV = v;
                    bestC = c;
                }
            }
        }

        if (bestV >= 0) {
            current[bestV] = bestC;
            currentConf += bestDelta;
            tabuList[bestV][bestC] = m_tabuTenure;
        }

        // Decay tabu tenures
        for (int v = 0; v < m_n; ++v)
            for (int c = 0; c < k; ++c)
                if (tabuList[v][c] > 0) tabuList[v][c]--;
    }

    return current;
}

/* ---- Crossover ---- */

QVector<int> GraphColoring12::crossover(const QVector<int>& p1, const QVector<int>& p2,
                                         int k, unsigned int& seed) const
{
    QVector<int> child(m_n);
    for (int i = 0; i < m_n; ++i) {
        // Pick color from parent with fewer local conflicts
        int conf1 = 0, conf2 = 0;
        for (int nb : m_adj[i]) {
            if (p1[i] == p1[nb]) conf1++;
            if (p2[i] == p2[nb]) conf2++;
        }
        child[i] = (conf1 <= conf2) ? p1[i] : p2[i];
    }
    return child;
}

/* ---- Mutation ---- */

void GraphColoring12::mutate(QVector<int>& colors, int k, unsigned int& seed) const
{
    for (int i = 0; i < m_n; ++i) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        double r = static_cast<double>(seed >> 33) / static_cast<double>(1ULL << 31);
        if (r < m_mutationRate) {
            seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
            colors[i] = static_cast<int>(seed % static_cast<unsigned int>(k));
        }
    }
}

/* ---- Main coloring ---- */

GraphColoring12::ColoringResult GraphColoring12::color()
{
    QElapsedTimer timer;
    timer.start();

    ColoringResult result;
    if (m_n == 0) return result;

    // Start with DSATUR upper bound
    QVector<int> dsaturColors = dsaturColoring();
    int maxColor = 0;
    for (int c : dsaturColors) maxColor = qMax(maxColor, c);
    int upperK = maxColor + 1;

    // Evolutionary + Tabu search
    unsigned int seed = 42;
    QVector<QVector<int>> population(m_popSize);
    population[0] = tabuImprove(dsaturColors, upperK, m_maxIter / 2);
    for (int i = 1; i < m_popSize; ++i)
        population[i] = tabuImprove(randomColoring(upperK, seed), upperK, m_maxIter / 4);

    int bestConf = countConflicts(population[0]);
    QVector<int> bestColors = population[0];

    for (int gen = 0; gen < m_maxIter && bestConf > 0; ++gen) {
        // Evaluate and sort by conflicts
        QVector<QPair<int, int>> scored(m_popSize);
        for (int i = 0; i < m_popSize; ++i)
            scored[i] = qMakePair(countConflicts(population[i]), i);
        std::sort(scored.begin(), scored.end());

        if (scored[0].first < bestConf) {
            bestConf = scored[0].first;
            bestColors = population[scored[0].second];
        }

        // Create next generation
        QVector<QVector<int>> nextPop(m_popSize);
        nextPop[0] = population[scored[0].second]; // Elitism
        for (int i = 1; i < m_popSize; ++i) {
            seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
            int p1 = static_cast<int>(seed % static_cast<unsigned int>(m_popSize));
            seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
            int p2 = static_cast<int>(seed % static_cast<unsigned int>(m_popSize));
            nextPop[i] = crossover(population[p1], population[p2], upperK, seed);
            mutate(nextPop[i], upperK, seed);
            nextPop[i] = tabuImprove(nextPop[i], upperK, m_maxIter / 20);
        }
        population = nextPop;
    }

    result.colors = bestColors;
    result.conflicts = bestConf;
    result.valid = (bestConf == 0);
    result.iterations = m_maxIter;

    // Remap colors to 0..k-1
    QVector<int> colorMap(m_n, -1);
    int nextColor = 0;
    for (int i = 0; i < m_n; ++i) {
        if (colorMap[bestColors[i]] < 0)
            colorMap[bestColors[i]] = nextColor++;
        result.colors[i] = colorMap[bestColors[i]];
    }
    result.chromaticNum = nextColor;

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = 0;
    for (auto& adj : m_adj) m_stats.numEdges += adj.size();
    m_stats.numEdges /= 2;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit coloringDone(result.chromaticNum, result.conflicts, elapsed);

    return result;
}

/* ---- Color with k colors ---- */

GraphColoring12::ColoringResult GraphColoring12::colorWithK(int k)
{
    QElapsedTimer timer;
    timer.start();
    ColoringResult result;
    if (m_n == 0 || k <= 0) return result;

    unsigned int seed = 42;
    QVector<int> bestColors;
    int bestConf = m_n * m_n;

    for (int trial = 0; trial < m_popSize; ++trial) {
        QVector<int> c = randomColoring(k, seed);
        c = tabuImprove(c, k, m_maxIter);
        int conf = countConflicts(c);
        if (conf < bestConf) {
            bestConf = conf;
            bestColors = c;
        }
        if (conf == 0) break;
    }

    result.colors = bestColors;
    result.chromaticNum = k;
    result.conflicts = bestConf;
    result.valid = (bestConf == 0);
    result.iterations = m_maxIter;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Find minimum coloring ---- */

GraphColoring12::ColoringResult GraphColoring12::findMinColoring()
{
    // Binary search on k
    ColoringResult upper = color();
    int lo = 1, hi = upper.chromaticNum;
    ColoringResult best = upper;

    while (lo < hi) {
        int mid = (lo + hi) / 2;
        ColoringResult res = colorWithK(mid);
        if (res.valid) {
            best = res;
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return best;
}

/* ---- Verify coloring ---- */

bool GraphColoring12::verifyColoring(const QVector<int>& colors) const
{
    if (colors.size() != m_n) return false;
    for (int i = 0; i < m_n; ++i)
        for (int j : m_adj[i])
            if (j > i && colors[i] == colors[j])
                return false;
    return true;
}

/* ---- Reset ---- */

void GraphColoring12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_adj.clear();
    m_adjMatrix.clear();
    m_n = 0;
}
