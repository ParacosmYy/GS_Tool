/**
 * @file SpectralCluster7.cpp
 * @brief SpectralCluster7 实现
 *
 * 实现谱聚类：归一化拉普拉斯、特征间隙自动选K、Nystrom近似、K-means嵌入。
 */

#include "utils/cluster177/SpectralCluster7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SpectralCluster7::SpectralCluster7(QObject *parent) : QObject(parent) {}
SpectralCluster7::~SpectralCluster7() = default;

/* ---- Configuration ---- */

void SpectralCluster7::setNumClusters(int k) { m_numClusters = qMax(2, k); }
void SpectralCluster7::setAutoK(bool enabled) { m_autoK = enabled; }
void SpectralCluster7::setMaxK(int maxK) { m_maxK = qMax(2, maxK); }
void SpectralCluster7::setSigma(double sigma) { m_sigma = qMax(0.01, sigma); }
void SpectralCluster7::setNystromRatio(double ratio) { m_nystromRatio = qBound(0.01, ratio, 1.0); }
void SpectralCluster7::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }

/* ---- Gaussian affinity matrix ---- */

QVector<QVector<double>> SpectralCluster7::buildAffinity(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    double neg2s2 = -1.0 / (2.0 * m_sigma * m_sigma);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dist2 = 0.0;
            for (int d = 0; d < m_dims; ++d) {
                double diff = data[i][d] - data[j][d];
                dist2 += diff * diff;
            }
            double val = qExp(dist2 * neg2s2);
            W[i][j] = val;
            W[j][i] = val;
        }
    }
    return W;
}

/* ---- Normalized Laplacian: D^{-1/2} W D^{-1/2} ---- */

void SpectralCluster7::normalizedLaplacian(QVector<QVector<double>>& W) const
{
    int n = W.size();
    QVector<double> d(n);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) sum += W[i][j];
        d[i] = qSqrt(qMax(sum, 1e-12));
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            W[i][j] /= (d[i] * d[j] + 1e-15);
}

/* ---- Power iteration for top-K eigenvectors ---- */

void SpectralCluster7::powerIteration(const QVector<QVector<double>>& mat,
                                        int k, QVector<QVector<double>>& eigvecs,
                                        QVector<double>& eigvals)
{
    int n = mat.size();
    if (n == 0) return;
    eigvecs.resize(k);
    eigvals.resize(k);

    QVector<double> deflated(n, 0.0); // cumulative deflation

    for (int ev = 0; ev < k; ++ev) {
        QVector<double> v(n);
        for (int i = 0; i < n; ++i) v[i] = static_cast<double>(qrand()) / RAND_MAX - 0.5;
        // Remove components of previous eigenvectors
        for (int prev = 0; prev < ev; ++prev) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += v[i] * eigvecs[prev][i];
            for (int i = 0; i < n; ++i) v[i] -= dot * eigvecs[prev][i];
        }

        for (int iter = 0; iter < 200; ++iter) {
            // Matrix-vector multiply
            QVector<double> mv(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    mv[i] += mat[i][j] * v[j];
            // Subtract deflation
            for (int i = 0; i < n; ++i) mv[i] -= deflated[i];

            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += mv[i] * mv[i];
            norm = qSqrt(qMax(norm, 1e-15));
            for (int i = 0; i < n; ++i) v[i] = mv[i] / norm;

            if (iter > 0) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += v[i] * mv[i];
                if (qAbs(qAbs(dot / norm) - 1.0) < 1e-8) break;
            }
        }
        // Eigenvalue estimate
        double lambda = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                lambda += v[i] * mat[i][j] * v[j];

        eigvecs[ev] = v;
        eigvals[ev] = lambda;
        // Update deflation
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                deflated[i] += lambda * v[i] * v[j];
    }
}

/* ---- Nystrom approximation ---- */

void SpectralCluster7::nystromApproximation(const QVector<QVector<double>>& data,
                                              QVector<QVector<double>>& eigvecs,
                                              QVector<double>& eigvals)
{
    int n = data.size();
    int m = qBound(10, static_cast<int>(n * m_nystromRatio), n);

    // Sample landmark points
    QVector<int> indices(m);
    for (int i = 0; i < m; ++i) indices[i] = (i * n) / m;

    // Build small affinity between landmarks
    QVector<QVector<double>> Wmm(m, QVector<double>(m, 0.0));
    double neg2s2 = -1.0 / (2.0 * m_sigma * m_sigma);
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j) {
            double d2 = 0.0;
            for (int dd = 0; dd < m_dims; ++dd) {
                double diff = data[indices[i]][dd] - data[indices[j]][dd];
                d2 += diff * diff;
            }
            Wmm[i][j] = qExp(d2 * neg2s2);
        }

    // Eigendecompose Wmm via power iteration
    int k = qMin(m_numClusters + 2, m);
    QVector<QVector<double>> evecs;
    QVector<double> evals;
    powerIteration(Wmm, k, evecs, evals);

    // Build cross-affinity Wnm (n x m)
    QVector<QVector<double>> Wnm(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j) {
            double d2 = 0.0;
            for (int dd = 0; dd < m_dims; ++dd) {
                double diff = data[i][dd] - data[indices[j]][dd];
                d2 += diff * diff;
            }
            Wnm[i][j] = qExp(d2 * neg2s2);
        }

    // Nystrom extension: V = Wnm * evecs * diag(1/evals)
    int useK = qMin(m_numClusters, k);
    eigvecs.resize(useK);
    eigvals.resize(useK);
    for (int ev = 0; ev < useK; ++ev) {
        eigvecs[ev].resize(n);
        for (int i = 0; i < n; ++i) {
            double val = 0.0;
            for (int j = 0; j < m; ++j)
                val += Wnm[i][j] * evecs[ev][j];
            eigvecs[ev][i] = val / (qAbs(evals[ev]) + 1e-12);
        }
        eigvals[ev] = evals[ev];
    }
}

/* ---- Eigengap heuristic for auto-K ---- */

int SpectralCluster7::autoSelectK(const QVector<double>& eigenvalues) const
{
    int maxK = qMin(m_maxK, eigenvalues.size() - 1);
    if (maxK < 2) return 2;

    double maxGap = 0.0;
    int bestK = 2;
    for (int k = 1; k < maxK; ++k) {
        double gap = qAbs(eigenvalues[k] - eigenvalues[k - 1]);
        if (gap > maxGap) {
            maxGap = gap;
            bestK = k + 1;
        }
    }
    return qBound(2, bestK, m_maxK);
}

/* ---- Normalize rows of embedding matrix ---- */

void SpectralCluster7::normalizeRows(QVector<QVector<double>>& mat)
{
    int rows = mat[0].size();
    int cols = mat.size();
    for (int i = 0; i < rows; ++i) {
        double norm = 0.0;
        for (int j = 0; j < cols; ++j) norm += mat[j][i] * mat[j][i];
        norm = qSqrt(qMax(norm, 1e-15));
        for (int j = 0; j < cols; ++j) mat[j][i] /= norm;
    }
}

/* ---- K-means on embedding space ---- */

QVector<int> SpectralCluster7::kmeansEmbed(const QVector<QVector<double>>& embedded, int k)
{
    int n = embedded.size();
    int d = embedded.isEmpty() ? 0 : embedded[0].size();
    if (n == 0 || d == 0) return {};

    // Initialize centers from random samples
    QVector<QVector<double>> ctrs(k);
    for (int c = 0; c < k; ++c)
        ctrs[c] = embedded[(c * n) / k];

    QVector<int> labels(n, 0);
    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool changed = false;
        // Assign
        for (int i = 0; i < n; ++i) {
            double bestD = 1e30;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double dist = 0.0;
                for (int dd = 0; dd < d; ++dd) {
                    double diff = embedded[i][dd] - ctrs[c][dd];
                    dist += diff * diff;
                }
                if (dist < bestD) { bestD = dist; bestC = c; }
            }
            if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
        }
        if (!changed) break;

        // Update centers
        for (int c = 0; c < k; ++c) {
            ctrs[c].fill(0.0);
            int cnt = 0;
            for (int i = 0; i < n; ++i) {
                if (labels[i] == c) {
                    for (int dd = 0; dd < d; ++dd) ctrs[c][dd] += embedded[i][dd];
                    ++cnt;
                }
            }
            if (cnt > 0)
                for (int dd = 0; dd < d; ++dd) ctrs[c][dd] /= cnt;
        }
    }
    m_centers = ctrs;
    return labels;
}

/* ---- Main fit ---- */

QVector<int> SpectralCluster7::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return {};
    m_dims = data[0].size();

    QVector<QVector<double>> eigvecs;
    QVector<double> eigvals;

    if (m_nystromRatio < 1.0 && m_n > 100) {
        // Use Nystrom approximation for large datasets
        nystromApproximation(data, eigvecs, eigvals);
    } else {
        // Full spectral decomposition
        auto W = buildAffinity(data);
        normalizedLaplacian(W);
        int k = qMin(m_numClusters + 4, m_n);
        powerIteration(W, k, eigvecs, eigvals);
    }

    // Auto-K via eigengap
    int useK = m_numClusters;
    double gap = 0.0;
    if (m_autoK && eigvals.size() > 2) {
        useK = autoSelectK(eigvals);
        if (useK >= 2 && useK < eigvals.size())
            gap = qAbs(eigvals[useK - 1] - eigvals[useK - 2]);
    }

    // Select top-K eigenvectors and transpose to row-major embedding
    int dims = qMin(useK, eigvecs.size());
    QVector<QVector<double>> embedded(m_n, QVector<double>(dims, 0.0));
    for (int ev = 0; ev < dims; ++ev)
        for (int i = 0; i < m_n; ++i)
            embedded[i][ev] = eigvecs[ev][i];

    // Normalize rows
    for (int i = 0; i < m_n; ++i) {
        double norm = 0.0;
        for (int d = 0; d < dims; ++d) norm += embedded[i][d] * embedded[i][d];
        norm = qSqrt(qMax(norm, 1e-15));
        for (int d = 0; d < dims; ++d) embedded[i][d] /= norm;
    }

    m_labels = kmeansEmbed(embedded, useK);
    m_eigvecs = eigvecs;

    m_stats.totalRuns++;
    m_stats.numClusters = useK;
    m_stats.numSamples = m_n;
    m_stats.eigenDims = dims;
    m_stats.eigengap = gap;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit clusteringCompleted(useK, gap);
    return m_labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> SpectralCluster7::centers() const { return m_centers; }
QVector<QVector<double>> SpectralCluster7::eigenvectors() const { return m_eigvecs; }

void SpectralCluster7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
