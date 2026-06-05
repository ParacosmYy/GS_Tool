/**
 * @file SpectralClustering3.cpp
 * @brief 谱聚类算法实现 — 归一化Laplacian + 特征分解 + K-Means
 */

#include "utils/cluster12/SpectralClustering3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <numeric>

/* ========== 构造/配置 ========== */

SpectralClustering3::SpectralClustering3(QObject* parent)
    : QObject(parent), m_k(3), m_sigma(1.0), m_knn(5), m_maxIterations(100),
      m_timeSum(0.0) {}

void SpectralClustering3::setK(int k)               { m_k = qMax(2, k); }
void SpectralClustering3::setSigma(double sigma)     { m_sigma = qMax(0.01, sigma); }
void SpectralClustering3::setKNN(int knn)            { m_knn = qMax(1, knn); }
void SpectralClustering3::setMaxIterations(int maxI) { m_maxIterations = qMax(1, maxI); }

/* ========== 核心聚类 ========== */

SpectralClustering3::ClusterResult SpectralClustering3::cluster(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = data.size();
    if (n < 2 || m_k < 2) return result;

    int k = qMin(m_k, n);

    /* 1) 构建相似度矩阵 */
    QVector<QVector<double>> W = (m_knn > 0)
        ? buildKNNAffinity(data) : buildRBFAffinity(data);

    /* 2) 计算度矩阵D和归一化Laplacian L_sym = I - D^{-1/2} W D^{-1/2} */
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            d[i] += W[i][j];

    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        double di = (d[i] > 1e-15) ? 1.0 / qSqrt(d[i]) : 0.0;
        for (int j = 0; j < n; ++j) {
            double dj = (d[j] > 1e-15) ? 1.0 / qSqrt(d[j]) : 0.0;
            L[i][j] = (i == j) ? 1.0 : -di * W[i][j] * dj;
        }
    }

    /* 3) 幂迭代提取前k个最小特征向量 */
    QVector<QVector<double>> eigvecs = computeEigenvectors(L, k);
    emit eigenDecomposed(n, k);

    /* 4) 行归一化 */
    for (int i = 0; i < n; ++i) normalizeVec(eigvecs[i]);

    /* 5) K-Means聚类 */
    auto pair = runKMeans(eigvecs, k);
    result.labels = pair.first;
    result.iterations = pair.second;

    /* 6) 计算归一化切割值 */
    result.normalizedCut = computeNCut(W, result.labels, k);

    /* 计算特征间隙(第k和第k+1个特征值的差) */
    result.eigenGap = 0.0; // 幂迭代无法直接获得，此处用0占位

    /* 统计更新 */
    ++m_stats.totalClusterings;
    ++m_stats.totalEigenComputations;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringComplete(k, result.normalizedCut);
    return result;
}

int SpectralClustering3::autoSelectK(
    const QVector<QVector<double>>& data, int maxK) const
{
    if (data.size() < 4) return 2;
    maxK = qMin(maxK, data.size() / 2);
    if (maxK < 2) return 2;

    /* 对小规模Laplacian做全特征值估计 */
    QVector<QVector<double>> W = buildRBFAffinity(data);
    int n = data.size();

    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            d[i] += W[i][j];

    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        double di = (d[i] > 1e-15) ? 1.0 / qSqrt(d[i]) : 0.0;
        for (int j = 0; j < n; ++j) {
            double dj = (d[j] > 1e-15) ? 1.0 / qSqrt(d[j]) : 0.0;
            L[i][j] = (i == j) ? 1.0 : -di * W[i][j] * dj;
        }
    }

    /* 提取maxK+1个最小特征向量以估计间隙 */
    QVector<QVector<double>> eigvecs = const_cast<SpectralClustering3*>(this)
        ->computeEigenvectors(L, maxK + 1);

    /* 通过瑞利商估计特征值 */
    QVector<double> eigenvalues;
    for (int idx = 0; idx < eigvecs[0].size() && idx < maxK + 1; ++idx) {
        /* 提取第idx列 */
        QVector<double> v(n);
        for (int i = 0; i < n; ++i) v[i] = eigvecs[i][idx];
        /* Rayleigh quotient: lambda = v^T L v / v^T v */
        double num = 0.0, den = 0.0;
        for (int i = 0; i < n; ++i) {
            den += v[i] * v[i];
            double Lv = 0.0;
            for (int j = 0; j < n; ++j) Lv += L[i][j] * v[j];
            num += v[i] * Lv;
        }
        eigenvalues.append((den > 1e-15) ? num / den : 0.0);
    }

    std::sort(eigenvalues.begin(), eigenvalues.end());

    /* 找最大间隙 */
    int bestK = 2;
    double maxGap = 0.0;
    for (int i = 1; i < eigenvalues.size() - 1; ++i) {
        double gap = qAbs(eigenvalues[i] - eigenvalues[i - 1]);
        if (gap > maxGap) { maxGap = gap; bestK = i + 1; }
    }
    return qBound(2, bestK, maxK);
}

/* ========== 相似度矩阵 ========== */

QVector<QVector<double>> SpectralClustering3::buildRBFAffinity(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));
    double negTwoSigmaSq = -2.0 * m_sigma * m_sigma;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double distSq = 0.0;
            int dim = qMin(data[i].size(), data[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                distSq += diff * diff;
            }
            double sim = qExp(distSq / negTwoSigmaSq);
            W[i][j] = W[j][i] = sim;
        }
        W[i][i] = 0.0; /* 对角线置零 */
    }
    return W;
}

QVector<QVector<double>> SpectralClustering3::buildKNNAffinity(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    /* 先用RBF算全距离 */
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dSq = 0.0;
            int dim = qMin(data[i].size(), data[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                dSq += diff * diff;
            }
            dist[i][j] = dist[j][i] = qSqrt(dSq);
        }
    }

    /* 对每个节点只保留KNN个最近邻的权重 */
    int knn = qMin(m_knn, n - 1);
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        QVector<QPair<double, int>> neighbors;
        neighbors.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (j != i) neighbors.append({dist[i][j], j});
        }
        std::sort(neighbors.begin(), neighbors.end());
        for (int k = 0; k < knn && k < neighbors.size(); ++k) {
            int j = neighbors[k].second;
            double sim = qExp(-dist[i][j] / (2.0 * m_sigma * m_sigma));
            W[i][j] = qMax(W[i][j], sim);
            W[j][i] = qMax(W[j][i], sim);
        }
    }
    return W;
}

/* ========== 特征分解(幂迭代) ========== */

QVector<QVector<double>> SpectralClustering3::computeEigenvectors(
    const QVector<QVector<double>>& laplacian, int k) const
{
    int n = laplacian.size();
    QVector<QVector<double>> eigvecs(n, QVector<double>(k, 0.0));

    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    QVector<QVector<double>> basis; /* 已找到的正交基 */

    for (int ev = 0; ev < k; ++ev) {
        /* 随机初始化 */
        QVector<double> v(n);
        for (int i = 0; i < n; ++i) v[i] = dist(rng);

        /* 正交化: 减去已找到特征向量方向的投影 */
        for (const auto& b : basis) {
            double proj = dotProduct(v, b);
            for (int i = 0; i < n; ++i) v[i] -= proj * b[i];
        }
        normalizeVec(v);

        /* 幂迭代(对Laplacian取反号找最小特征值 → 求 I-L 的最大) */
        for (int iter = 0; iter < 200; ++iter) {
            QVector<double> Lv(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    Lv[i] += laplacian[i][j] * v[j];

            /* 减去投影到已有基 */
            for (const auto& b : basis) {
                double proj = dotProduct(Lv, b);
                for (int i = 0; i < n; ++i) Lv[i] -= proj * b[i];
            }

            double norm = 0.0;
            for (double x : Lv) norm += x * x;
            norm = qSqrt(norm);
            if (norm < 1e-15) break;
            for (int i = 0; i < n; ++i) v[i] = Lv[i] / norm;
        }

        basis.append(v);
        for (int i = 0; i < n; ++i) eigvecs[i][ev] = v[i];
    }
    return eigvecs;
}

/* ========== K-Means子程序 ========== */

QPair<QVector<int>, int> SpectralClustering3::runKMeans(
    const QVector<QVector<double>>& points, int k) const
{
    int n = points.size();
    if (n == 0) return {QVector<int>(), 0};
    int dim = points[0].size();

    std::mt19937 rng(42);

    /* K-Means++ 初始化 */
    QVector<QVector<double>> centroids;
    centroids.append(points[rng() % n]);

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (const auto& cen : centroids) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd) d += (points[i][dd] - cen[dd]) * (points[i][dd] - cen[dd]);
                if (d < minD) minD = d;
            }
            dists[i] = minD;
            totalDist += minD;
        }
        if (totalDist < 1e-15) break;
        double r = static_cast<double>(rng()) / rng.max() * totalDist;
        double cumul = 0.0;
        for (int i = 0; i < n; ++i) {
            cumul += dists[i];
            if (cumul >= r) { centroids.append(points[i]); break; }
        }
    }

    k = centroids.size();
    QVector<int> labels(n, 0);

    int iter = 0;
    for (; iter < m_maxIterations; ++iter) {
        bool changed = false;
        /* 分配 */
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                for (int dd = 0; dd < dim; ++dd)
                    d += (points[i][dd] - centroids[c][dd]) * (points[i][dd] - centroids[c][dd]);
                if (d < minD) { minD = d; bestC = c; }
            }
            if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
        }
        if (!changed) break;

        /* 更新 */
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(dim, 0.0);
            int count = 0;
            for (int i = 0; i < n; ++i) {
                if (labels[i] == c) {
                    for (int dd = 0; dd < dim; ++dd) sum[dd] += points[i][dd];
                    ++count;
                }
            }
            if (count > 0)
                for (int dd = 0; dd < dim; ++dd) centroids[c][dd] = sum[dd] / count;
        }
    }
    return {labels, iter};
}

/* ========== 辅助 ========== */

double SpectralClustering3::computeNCut(
    const QVector<QVector<double>>& affinity,
    const QVector<int>& labels, int k) const
{
    int n = affinity.size();
    double ncut = 0.0;
    for (int c = 0; c < k; ++c) {
        double cut = 0.0;     /* 簇c到其他簇的权重和 */
        double assoc = 0.0;   /* 簇c到所有节点的权重和 */
        for (int i = 0; i < n; ++i) {
            if (labels[i] != c) continue;
            for (int j = 0; j < n; ++j) {
                assoc += affinity[i][j];
                if (labels[j] != c) cut += affinity[i][j];
            }
        }
        ncut += (assoc > 1e-15) ? cut / assoc : 0.0;
    }
    return ncut;
}

double SpectralClustering3::dotProduct(const QVector<double>& a,
                                       const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

void SpectralClustering3::normalizeVec(QVector<double>& v)
{
    double norm = 0.0;
    for (double x : v) norm += x * x;
    norm = qSqrt(norm);
    if (norm > 1e-15)
        for (double& x : v) x /= norm;
}

void SpectralClustering3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
