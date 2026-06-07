/**
 * @file SpectralCluster8.cpp
 * @brief SpectralCluster8 实现
 *
 * 实现谱聚类：自适应kNN+epsilon相似图构建、比率割目标优化、Laplacian特征向量嵌入。
 */

#include "utils/cluster191/SpectralCluster8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

SpectralCluster8::SpectralCluster8(QObject *parent) : QObject(parent) {}
SpectralCluster8::~SpectralCluster8() = default;

/* ---- Configuration ---- */

void SpectralCluster8::setKnnK(int k) { m_knnK = qMax(1, k); }
void SpectralCluster8::setEpsilon(double eps) { m_epsilon = qMax(1e-10, eps); }
void SpectralCluster8::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void SpectralCluster8::setSigma(double sigma) { m_sigma = qMax(1e-10, sigma); }

/* ---- Gaussian kernel similarity ---- */

double SpectralCluster8::gaussianSim(const QVector<double>& a,
                                       const QVector<double>& b) const
{
    double sum = 0.0;
    int d = qMin(a.size(), b.size());
    for (int i = 0; i < d; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qExp(-sum / (2.0 * m_sigma * m_sigma));
}

/* ---- Build adaptive similarity graph ---- */

QVector<QVector<double>> SpectralCluster8::buildGraph(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));

    // Compute pairwise similarities
    QVector<QVector<double>> sim(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double s = gaussianSim(data[i], data[j]);
            sim[i][j] = s;
            sim[j][i] = s;
        }

    // Adaptive kNN + epsilon threshold
    int k = qMin(m_knnK, n - 1);
    for (int i = 0; i < n; ++i) {
        // Sort neighbors by similarity
        QVector<QPair<double, int>> neighbors;
        for (int j = 0; j < n; ++j) {
            if (j != i)
                neighbors.append({sim[i][j], j});
        }
        std::sort(neighbors.begin(), neighbors.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        // kNN: connect to top-k neighbors
        for (int t = 0; t < qMin(k, neighbors.size()); ++t) {
            int j = neighbors[t].second;
            double s = neighbors[t].first;
            if (s >= m_epsilon) {
                W[i][j] = qMax(W[i][j], s);
                W[j][i] = qMax(W[j][i], s);
            }
        }
    }
    return W;
}

/* ---- Compute normalized Laplacian ---- */

QVector<QVector<double>> SpectralCluster8::laplacian(
    const QVector<QVector<double>>& graph) const
{
    int n = graph.size();
    // Degree matrix D
    QVector<double> D(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            D[i] += graph[i][j];

    // Normalized Laplacian: L = I - D^{-1/2} W D^{-1/2}
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double di = (D[i] > 1e-15) ? 1.0 / qSqrt(D[i]) : 0.0;
            double dj = (D[j] > 1e-15) ? 1.0 / qSqrt(D[j]) : 0.0;
            L[i][j] = (i == j ? 1.0 : 0.0) - di * graph[i][j] * dj;
        }
    }
    return L;
}

/* ---- Power iteration for top eigenvectors ---- */

QVector<QVector<double>> SpectralCluster8::powerEigenvectors(
    const QVector<QVector<double>>& mat, int numVecs, int maxIter) const
{
    int n = mat.size();
    QVector<QVector<double>> eigvecs(numVecs, QVector<double>(n, 0.0));

    QVector<QVector<double>> residual = mat;

    for (int v = 0; v < numVecs; ++v) {
        // Random initial vector
        QVector<double> vec(n);
        for (int i = 0; i < n; ++i)
            vec[i] = (static_cast<double>(qrand()) / RAND_MAX) - 0.5;

        // Deflate from previous eigenvectors
        for (int pv = 0; pv < v; ++pv) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += vec[i] * eigvecs[pv][i];
            for (int i = 0; i < n; ++i) vec[i] -= dot * eigvecs[pv][i];
        }

        // Normalize
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += vec[i] * vec[i];
        norm = qSqrt(qMax(norm, 1e-15));
        for (int i = 0; i < n; ++i) vec[i] /= norm;

        // Power iteration
        for (int it = 0; it < maxIter; ++it) {
            QVector<double> newVec(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    newVec[i] += residual[i][j] * vec[j];

            // Deflate
            for (int pv = 0; pv < v; ++pv) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += newVec[i] * eigvecs[pv][i];
                for (int i = 0; i < n; ++i) newVec[i] -= dot * eigvecs[pv][i];
            }

            double nrm = 0.0;
            for (int i = 0; i < n; ++i) nrm += newVec[i] * newVec[i];
            nrm = qSqrt(qMax(nrm, 1e-15));
            for (int i = 0; i < n; ++i) vec[i] = newVec[i] / nrm;
        }
        eigvecs[v] = vec;
    }
    return eigvecs;
}

/* ---- Normalize rows ---- */

void SpectralCluster8::normalizeRows(QVector<QVector<double>>& mat) const
{
    for (auto& row : mat) {
        double nrm = 0.0;
        for (double v : row) nrm += v * v;
        nrm = qSqrt(qMax(nrm, 1e-15));
        for (double& v : row) v /= nrm;
    }
}

/* ---- Simple k-means on embedded vectors ---- */

QVector<int> SpectralCluster8::embeddedKMeans(
    const QVector<QVector<double>>& embedded, int k, int maxIter)
{
    int n = embedded.size();
    if (n == 0) return {};
    int dim = embedded[0].size();
    k = qMin(k, n);

    // Initialize centroids from random points
    QVector<QVector<double>> cents(k);
    for (int j = 0; j < k; ++j)
        cents[j] = embedded[qrand() % n];

    QVector<int> labels(n, 0);

    for (int it = 0; it < maxIter; ++it) {
        bool changed = false;

        // Assign
        for (int i = 0; i < n; ++i) {
            double bestD = std::numeric_limits<double>::max();
            int bestJ = 0;
            for (int j = 0; j < k; ++j) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = embedded[i][dd] - cents[j][dd];
                    d += diff * diff;
                }
                if (d < bestD) { bestD = d; bestJ = j; }
            }
            if (labels[i] != bestJ) { labels[i] = bestJ; changed = true; }
        }

        if (!changed) break;

        // Update centroids
        QVector<int> counts(k, 0);
        cents = QVector<QVector<double>>(k, QVector<double>(dim, 0.0));
        for (int i = 0; i < n; ++i) {
            counts[labels[i]]++;
            for (int dd = 0; dd < dim; ++dd)
                cents[labels[i]][dd] += embedded[i][dd];
        }
        for (int j = 0; j < k; ++j)
            if (counts[j] > 0)
                for (int dd = 0; dd < dim; ++dd)
                    cents[j][dd] /= counts[j];
    }
    return labels;
}

/* ---- Fit model ---- */

QVector<int> SpectralCluster8::fit(
    const QVector<QVector<double>>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0 || k <= 0) return {};
    k = qMin(k, n);

    // Build similarity graph
    QVector<QVector<double>> W = buildGraph(data);
    int edges = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (W[i][j] > 0) edges++;

    // Compute normalized Laplacian
    QVector<QVector<double>> L = laplacian(W);

    // Extract k smallest eigenvectors via power method on (I - L)
    // We want smallest eigenvalues of L, i.e. largest of (I - L)
    QVector<QVector<double>> shiftedL = L;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            shiftedL[i][j] = (i == j ? 1.0 : 0.0) - shiftedL[i][j];

    m_eigvecs = powerEigenvectors(shiftedL, k, m_maxIter);

    // Build embedding matrix: n x k
    QVector<QVector<double>> embedded(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i)
        for (int v = 0; v < k; ++v)
            embedded[i][v] = m_eigvecs[v][i];

    normalizeRows(embedded);

    // k-means on embedded space
    QVector<int> labels = embeddedKMeans(embedded, k, m_maxIter);

    m_stats.totalRuns++;
    m_stats.numPoints = n;
    m_stats.numClusters = k;
    m_stats.graphEdges = edges;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(k, edges, timer.elapsed());
    return labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> SpectralCluster8::eigenvectors() const
{
    return m_eigvecs;
}

/* ---- Reset ---- */

void SpectralCluster8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_eigvecs.clear();
}
