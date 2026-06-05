/**
 * @file SpectralCluster2.cpp
 * @brief 谱聚类增强实现 — 归一化割/随机游走/特征向量选择/k-means后处理
 */

#include "utils/cluster26/SpectralCluster2.h"

#include <QtMath>
#include <algorithm>
#include <random>

SpectralCluster2::SpectralCluster2(int k, NormalizeMode mode, QObject* parent)
    : QObject(parent), m_k(k), m_mode(mode)
{
}

QVector<int> SpectralCluster2::fit(const QVector<double>& similarity, int n)
{
    m_timing.start();
    ++m_stats.totalClusterOps;

    /* 1. 构建拉普拉斯矩阵 */
    QVector<double> lap = buildLaplacian(similarity, n, m_mode);

    /* 2. 确定聚类数 */
    int k = m_k;
    if (k <= 0 || k >= n) {
        eigenDecompose(lap, n, qMin(n - 1, 20));
        k = autoSelectK(m_eigenvalues);
        m_k = k;
    }

    /* 3. 求前k个特征向量 */
    eigenDecompose(lap, n, k);
    emit eigenDecompositionComplete(k);

    /* 4. k-means后处理 */
    QVector<int> labels = kmeansPostProcess(m_eigenvectors, n, k, k);
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterOps;
    emit clusteringComplete(k, m_stats.avgProcessingTimeMs);
    return labels;
}

QVector<int> SpectralCluster2::fitPoints(const QVector<double>& points, double sigma)
{
    int n = points.size() / 2;
    if (n < 2) return {0};

    /* 构建RBF相似度矩阵 */
    QVector<double> W(n * n, 0.0);
    double sigmaSq = sigma;
    if (sigmaSq <= 0.0) {
        /* 自动估计sigma: 取所有点对距离的中位数 */
        QVector<double> dists;
        dists.reserve(n * (n - 1) / 2);
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double dx = points[2 * i] - points[2 * j];
                double dy = points[2 * i + 1] - points[2 * j + 1];
                dists.append(qSqrt(dx * dx + dy * dy));
            }
        }
        std::sort(dists.begin(), dists.end());
        sigmaSq = dists[dists.size() / 2];
    }
    sigmaSq = sigmaSq * sigmaSq * 2.0;
    if (sigmaSq < 1e-12) sigmaSq = 1.0;

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            double dx = points[2 * i] - points[2 * j];
            double dy = points[2 * i + 1] - points[2 * j + 1];
            double d2 = dx * dx + dy * dy;
            double val = qExp(-d2 / sigmaSq);
            W[i * n + j] = val;
            W[j * n + i] = val;
        }
    }
    return fit(W, n);
}

QVector<double> SpectralCluster2::buildDegree(const QVector<double>& W, int n) const
{
    QVector<double> deg(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            deg[i] += W[i * n + j];
    return deg;
}

QVector<double> SpectralCluster2::buildLaplacian(const QVector<double>& W, int n,
                                                  NormalizeMode mode) const
{
    QVector<double> deg = buildDegree(W, n);
    QVector<double> L(n * n, 0.0);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double val = (i == j) ? deg[i] : 0.0;
            val -= W[i * n + j];
            /* 归一化 */
            if (mode == NormalizeMode::NormalizedCut) {
                double di = qSqrt(qMax(deg[i], 1e-15));
                double dj = qSqrt(qMax(deg[j], 1e-15));
                val /= (di * dj);
            } else if (mode == NormalizeMode::RandomWalk) {
                val /= qMax(deg[i], 1e-15);
            }
            L[i * n + j] = val;
        }
    }
    return L;
}

void SpectralCluster2::eigenDecompose(const QVector<double>& mat, int n, int k)
{
    ++m_stats.totalEigenDecomps;
    m_eigenvalues.resize(k);
    m_eigenvectors.resize(n * k);

    /* 随机初始化 */
    std::mt19937 gen(42);
    std::normal_distribution<double> dist(0.0, 1.0);

    /* 幂迭代求前k个最小特征向量(因拉普拉斯半正定) */
    for (int ev = 0; ev < k; ++ev) {
        QVector<double> v(n), w(n);
        for (int i = 0; i < n; ++i) v[i] = dist(gen);
        double norm = 0.0;
        for (double x : v) norm += x * x;
        norm = qSqrt(qMax(norm, 1e-15));
        for (double& x : v) x /= norm;

        for (int iter = 0; iter < 200; ++iter) {
            /* w = L * v */
            for (int i = 0; i < n; ++i) {
                w[i] = 0.0;
                for (int j = 0; j < n; ++j)
                    w[i] += mat[i * n + j] * v[j];
            }
            /* 减去已求特征向量投影(正交化) */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i)
                    dot += w[i] * m_eigenvectors[i * k + prev];
                for (int i = 0; i < n; ++i)
                    w[i] -= dot * m_eigenvectors[i * k + prev];
            }
            norm = 0.0;
            for (double x : w) norm += x * x;
            norm = qSqrt(qMax(norm, 1e-15));
            bool converged = true;
            for (int i = 0; i < n; ++i) {
                double nv = w[i] / norm;
                if (qAbs(nv - v[i]) > 1e-10) converged = false;
                v[i] = nv;
            }
            if (converged) break;
        }
        /* 计算特征值: lambda = v^T L v */
        double lambda = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                lambda += v[i] * mat[i * n + j] * v[j];
        m_eigenvalues[ev] = lambda;
        for (int i = 0; i < n; ++i)
            m_eigenvectors[i * k + ev] = v[i];
    }
}

int SpectralCluster2::autoSelectK(const QVector<double>& evals) const
{
    /* 特征间隙法: 找最大间隙 */
    int bestK = 1;
    double maxGap = 0.0;
    for (int i = 1; i < evals.size() - 1; ++i) {
        double gap = evals[i + 1] - evals[i];
        if (gap > maxGap) { maxGap = gap; bestK = i + 1; }
    }
    return qMax(2, qMin(bestK, static_cast<int>(evals.size())));
}

QVector<int> SpectralCluster2::kmeansPostProcess(const QVector<double>& vectors,
                                                  int n, int dim, int k)
{
    QVector<int> labels(n, 0);
    if (n <= k || k <= 0) return labels;

    std::mt19937 gen(123);
    /* k-means++初始化 */
    QVector<QVector<double>> centers(k);
    int first = gen() % n;
    for (int d = 0; d < dim; ++d)
        centers[0].append(vectors[first * dim + d]);

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n);
        double total = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = 1e30;
            for (int p = 0; p < c; ++p) {
                double d2 = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = vectors[i * dim + dd] - centers[p][dd];
                    d2 += diff * diff;
                }
                minD = qMin(minD, d2);
            }
            dists[i] = minD;
            total += minD;
        }
        double r = std::uniform_real_distribution<>(0, total)(gen);
        double cum = 0.0;
        int pick = 0;
        for (int i = 0; i < n; ++i) {
            cum += dists[i];
            if (cum >= r) { pick = i; break; }
        }
        for (int d = 0; d < dim; ++d)
            centers[c].append(vectors[pick * dim + d]);
    }

    /* 迭代 */
    for (int iter = 0; iter < 100; ++iter) {
        ++m_stats.totalKmeansIters;
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double minD = 1e30;
            int best = 0;
            for (int c = 0; c < k; ++c) {
                double d2 = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = vectors[i * dim + d] - centers[c][d];
                    d2 += diff * diff;
                }
                if (d2 < minD) { minD = d2; best = c; }
            }
            if (labels[i] != best) { labels[i] = best; changed = true; }
        }
        if (!changed) break;
        /* 更新中心 */
        QVector<int> counts(k, 0);
        for (auto& ctr : centers) ctr.fill(0.0);
        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            ++counts[c];
            for (int d = 0; d < dim; ++d)
                centers[c][d] += vectors[i * dim + d];
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < dim; ++d)
                    centers[c][d] /= counts[c];
            }
        }
    }
    return labels;
}

void SpectralCluster2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
