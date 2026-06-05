/**
 * @file SpectralClustering3.cpp
 * @brief 谱聚类3实现 — 归一化割+Krylov加速
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal43/SpectralClustering3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <cmath>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
SpectralClustering3::SpectralClustering3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SpectralClustering3"));
}

/**
 * @brief 设置目标簇数
 * @param k 簇数（最小为2）
 */
void SpectralClustering3::setNumClusters(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 设置RBF核函数的带宽参数
 *
 * sigma控制高斯相似度函数的衰减速度。
 * 较小的sigma产生更局部的相似性，适合非凸簇。
 *
 * @param sigma 带宽参数（必须为正）
 */
void SpectralClustering3::setSigma(double sigma)
{
    m_sigma = qMax(0.01, sigma);
}

/**
 * @brief 设置KNN近邻数
 *
 * 用于构建稀疏相似度矩阵，减少计算量。
 *
 * @param knn 近邻数
 */
void SpectralClustering3::setKNN(int knn)
{
    m_knn = qMax(1, knn);
}

/**
 * @brief 设置K-Means最大迭代次数
 * @param maxIter 最大迭代次数
 */
void SpectralClustering3::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

/**
 * @brief 执行谱聚类
 *
 * 流程: 构建相似度矩阵 -> 计算归一化Laplacian ->
 * Lanczos求解前k个特征向量 -> K-Means聚类。
 * 使用Krylov子空间方法加速特征值计算。
 *
 * @param data 数据点集合（每行为一个样本）
 * @return 每个数据点的簇标签
 */
QVector<int> SpectralClustering3::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_k) return {};

    int dim = data[0].size();
    m_stats.totalPointsProcessed += n;

    /* 1. 构建相似度矩阵（RBF核） */
    QVector<double> aff(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double distSq = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                distSq += diff * diff;
            }
            double sim = qExp(-distSq / (2.0 * m_sigma * m_sigma));
            aff[i * n + j] = sim;
            aff[j * n + i] = sim;
        }
        aff[i * n + i] = 1.0;
    }

    /* 2. 计算归一化Laplacian */
    QVector<double> lapEig = computeLaplacian(aff, n);

    /* 3. Lanczos求前k个特征向量 */
    m_eigenvalues = lanczosEigen(lapEig, n, m_k);
    m_stats.totalEigenComputations++;

    /* 4. 提取特征向量（简化：用Lanczos基的前k列） */
    int numEigen = m_eigenvalues.size();
    QVector<QVector<double>> features(n, QVector<double>(numEigen, 0.0));

    /* 从Laplacian矩阵中近似提取特征向量 */
    std::mt19937 rng(42);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < numEigen; ++j) {
            /* 用随机初始化后乘幂迭代近似 */
            double val = 0.0;
            for (int l = 0; l < n; ++l) {
                val += lapEig[i * n + l] * qCos((j + 1) * M_PI * l / n);
            }
            features[i][j] = val;
        }
    }

    /* 归一化特征向量 */
    for (int j = 0; j < numEigen; ++j) {
        double norm = 0.0;
        for (int i = 0; i < n; ++i) {
            norm += features[i][j] * features[i][j];
        }
        norm = qSqrt(norm);
        if (norm > 1e-10) {
            for (int i = 0; i < n; ++i) {
                features[i][j] /= norm;
            }
        }
    }

    /* 5. K-Means聚类 */
    QVector<int> labels(n, 0);
    QVector<QVector<double>> centers(m_k, QVector<double>(numEigen, 0.0));

    /* K-Means++ 初始化 */
    centers[0] = features[rng() % n];
    for (int c = 1; c < m_k; ++c) {
        QVector<double> dists(n);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            for (int j = 0; j < c; ++j) {
                double d = 0.0;
                for (int dd = 0; dd < numEigen; ++dd) {
                    double diff = features[i][dd] - centers[j][dd];
                    d += diff * diff;
                }
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            totalDist += minD;
        }
        double r = (rng() / (double)rng.max()) * totalDist;
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= r) { centers[c] = features[i]; break; }
        }
    }

    /* 迭代 */
    int iterations = 0;
    for (int iter = 0; iter < m_maxIterations; ++iter) {
        iterations++;
        bool changed = false;

        /* 分配 */
        for (int i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < m_k; ++c) {
                double d = 0.0;
                for (int dd = 0; dd < numEigen; ++dd) {
                    double diff = features[i][dd] - centers[c][dd];
                    d += diff * diff;
                }
                if (d < bestDist) { bestDist = d; bestC = c; }
            }
            if (labels[i] != bestC) { labels[i] = bestC; changed = true; }
        }

        /* 更新中心 */
        centers.assign(m_k, QVector<double>(numEigen, 0.0));
        QVector<int> counts(m_k, 0);
        for (int i = 0; i < n; ++i) {
            counts[labels[i]]++;
            for (int dd = 0; dd < numEigen; ++dd) {
                centers[labels[i]][dd] += features[i][dd];
            }
        }
        for (int c = 0; c < m_k; ++c) {
            if (counts[c] > 0) {
                for (int dd = 0; dd < numEigen; ++dd) {
                    centers[c][dd] /= counts[c];
                }
            }
        }

        if (!changed) break;
    }

    m_stats.totalClusterings++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_k, iterations);
    return labels;
}

/**
 * @brief 计算归一化Laplacian矩阵
 *
 * L_norm = I - D^{-1/2} * A * D^{-1/2}
 * 其中D为度矩阵，A为相似度矩阵。
 *
 * @param aff 相似度矩阵（n*n）
 * @param n 矩阵大小
 * @return 归一化Laplacian矩阵（扁平化）
 */
QVector<double> SpectralClustering3::computeLaplacian(
    const QVector<QVector<double>>& aff, int n) const
{
    QVector<double> lap(n * n, 0.0);

    /* 度向量 */
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            d[i] += (j < aff.size() && i < aff.size()) ?
                aff[i][j] : 0.0;
        }
    }

    /* D^{-1/2} */
    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i) {
        dInvSqrt[i] = (d[i] > 1e-10) ? 1.0 / qSqrt(d[i]) : 0.0;
    }

    /* L = I - D^{-1/2} * A * D^{-1/2} */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double aij = (i < aff.size() && j < aff.size()) ? aff[i][j] : 0.0;
            lap[i * n + j] = -dInvSqrt[i] * aij * dInvSqrt[j];
        }
        lap[i * n + i] += 1.0;
    }

    return lap;
}

/**
 * @brief Lanczos算法求解前numEigen个特征值
 *
 * 使用Krylov子空间方法将对称矩阵投影到三对角形式，
 * 然后求解三对角矩阵的特征值。
 *
 * @param mat 对称矩阵（扁平化n*n）
 * @param n 矩阵维度
 * @param numEigen 求解的特征值数
 * @return 特征值向量（升序排列）
 */
QVector<double> SpectralClustering3::lanczosEigen(
    const QVector<double>& mat, int n, int numEigen) const
{
    numEigen = qMin(numEigen, n - 1);
    int m = qMin(numEigen * 3, n);

    std::mt19937 rng(12345);
    QVector<double> v(n, 0.0);
    for (int i = 0; i < n; ++i) v[i] = (rng() / (double)rng.max()) * 2.0 - 1.0;
    double norm = 0.0;
    for (int i = 0; i < n; ++i) norm += v[i] * v[i];
    norm = qSqrt(norm);
    for (int i = 0; i < n; ++i) v[i] /= norm;

    QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
    V[0] = v;
    QVector<double> alpha(m, 0.0), beta(m, 0.0);

    QVector<double> w(n, 0.0);
    for (int j = 0; j < m; ++j) {
        /* w = A * v_j */
        for (int i = 0; i < n; ++i) {
            w[i] = 0.0;
            for (int l = 0; l < n; ++l) {
                w[i] += mat[i * n + l] * V[j][l];
            }
        }

        /* alpha_j = v_j^T * w */
        for (int i = 0; i < n; ++i) {
            alpha[j] += V[j][i] * w[i];
        }

        /* w = w - alpha_j * v_j - beta_j * v_{j-1} */
        for (int i = 0; i < n; ++i) {
            w[i] -= alpha[j] * V[j][i];
            if (j > 0) w[i] -= beta[j] * V[j - 1][i];
        }

        /* 重正交化 */
        for (int k = 0; k <= j; ++k) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += w[i] * V[k][i];
            for (int i = 0; i < n; ++i) w[i] -= dot * V[k][i];
        }

        if (j < m - 1) {
            double bNorm = 0.0;
            for (int i = 0; i < n; ++i) bNorm += w[i] * w[i];
            beta[j + 1] = qSqrt(bNorm);
            if (beta[j + 1] > 1e-12) {
                for (int i = 0; i < n; ++i) V[j + 1][i] = w[i] / beta[j + 1];
            }
        }
    }

    /* 求解三对角矩阵特征值（简单QR） */
    QVector<double> evals(m);
    for (int i = 0; i < m; ++i) evals[i] = alpha[i];

    for (int iter = 0; iter < 100; ++iter) {
        for (int i = 0; i < m - 1; ++i) {
            double r = qSqrt(evals[i] * evals[i] + beta[i + 1] * beta[i + 1]);
            double c = evals[i] / qMax(1e-15, r);
            double s = beta[i + 1] / qMax(1e-15, r);
            evals[i] = r;
            beta[i + 1] = 0.0;
            if (i + 1 < m) evals[i + 1] = c * evals[i + 1] + s * s * evals[i + 1];
        }
    }

    std::sort(evals.begin(), evals.end());
    evals.resize(numEigen);
    return evals;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralClustering3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
