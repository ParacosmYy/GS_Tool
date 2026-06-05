#include "SpectralClustering9.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化谱聚类引擎
 * @param parent 父对象指针
 */
SpectralClustering9::SpectralClustering9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SpectralClustering9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行谱聚类
 *
 * 完整流程：构建K近邻亲和矩阵 → 计算归一化拉普拉斯矩阵 →
 * 提取前k个最小特征向量 → 对特征向量矩阵行向量做K-Means聚类。
 *
 * @param dataPoints 输入数据点集合 (n×d)
 * @param k 目标聚类数
 * @return 各数据点的聚类标签
 */
QVector<int> SpectralClustering9::fit(const QVector<QVector<double>>& dataPoints, int k)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    QVector<int> labels(n, -1);
    if (n == 0 || k <= 0) {
        emit clusteringCompleted(0);
        return labels;
    }
    if (k >= n) {
        for (int i = 0; i < n; ++i) labels[i] = i;
        emit clusteringCompleted(n);
        return labels;
    }

    /* 1. 构建亲和矩阵 */
    int knnK = qMin(7, n - 1);
    double sigma = 1.0;
    QVector<QVector<double>> affinity = buildKNNAffinity(dataPoints, knnK, sigma);

    /* 2. 计算归一化拉普拉斯矩阵 */
    QVector<QVector<double>> laplacian = normalizedLaplacian(affinity);

    /* 3. 幂迭代法提取前k个最小特征向量（简化实现） */
    const int dims = n;
    QVector<QVector<double>> eigenVectors(k, QVector<double>(n, 0.0));

    QVector<double> prevEigenvector;
    for (int ev = 0; ev < k; ++ev) {
        QVector<double> v(n, 0.0);
        /* 初始化：小随机值 + 偏移 */
        for (int i = 0; i < n; ++i)
            v[i] = 0.1 / (i + 1 + ev);

        for (int iter = 0; iter < 200; ++iter) {
            /* 矩阵-向量乘法 L * v */
            QVector<double> Lv(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j)
                    Lv[i] += laplacian[i][j] * v[j];
            }

            /* 减去之前特征向量方向的分量（Gram-Schmidt正交化） */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i)
                    dot += Lv[i] * eigenVectors[prev][i];
                for (int i = 0; i < n; ++i)
                    Lv[i] -= dot * eigenVectors[prev][i];
            }

            /* 归一化 */
            double norm = 0.0;
            for (int i = 0; i < n; ++i)
                norm += Lv[i] * Lv[i];
            norm = qSqrt(norm);
            if (norm < 1e-15) break;

            for (int i = 0; i < n; ++i)
                v[i] = Lv[i] / norm;
        }
        eigenVectors[ev] = v;
    }

    /* 4. 构建行向量矩阵 T (n×k)，每行用于K-Means */
    QVector<QVector<double>> T(n, QVector<double>(k, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            T[i][j] = eigenVectors[j][i];

    /* 5. 简单K-Means聚类 */
    QVector<QVector<double>> centroids(k, QVector<double>(k, 0.0));
    for (int c = 0; c < k; ++c) {
        int idx = (c * n) / k;
        if (idx < n) centroids[c] = T[idx];
    }

    for (int iter = 0; iter < 50; ++iter) {
        /* 分配步骤 */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                for (int dim = 0; dim < k; ++dim) {
                    double diff = T[i][dim] - centroids[c][dim];
                    d += diff * diff;
                }
                if (d < bestDist) { bestDist = d; bestC = c; }
            }
            labels[i] = bestC;
        }

        /* 更新步骤 */
        QVector<QVector<double>> newCentroids(k, QVector<double>(k, 0.0));
        QVector<int> counts(k, 0);
        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            if (c < 0 || c >= k) continue;
            counts[c]++;
            for (int dim = 0; dim < k; ++dim)
                newCentroids[c][dim] += T[i][dim];
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int dim = 0; dim < k; ++dim)
                    newCentroids[c][dim] /= counts[c];
            }
        }
        centroids = newCentroids;
    }

    m_stats.totalClusterOps++;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterOps;

    emit clusteringCompleted(k);
    return labels;
}

/**
 * @brief 构建K近邻亲和矩阵
 *
 * 对每个数据点找到k个最近邻，用高斯核函数将距离转换为亲和度。
 * W(i,j) = exp(-||xi - xj||^2 / (2 * sigma^2))，仅保留KNN连接。
 *
 * @param dataPoints 数据点集合
 * @param kNeighbors 近邻数
 * @param sigma 高斯核宽度参数
 * @return 亲和矩阵（对称）
 */
QVector<QVector<double>> SpectralClustering9::buildKNNAffinity(
    const QVector<QVector<double>>& dataPoints, int kNeighbors, double sigma)
{
    const int n = dataPoints.size();
    QVector<QVector<double>> affinity(n, QVector<double>(n, 0.0));
    if (n == 0) return affinity;

    double sigma2 = 2.0 * sigma * sigma;
    if (sigma2 < 1e-15) sigma2 = 1.0;

    /* 计算所有成对距离并排序找KNN */
    for (int i = 0; i < n; ++i) {
        QVector<QPair<double, int>> dists;
        dists.reserve(n);
        for (int j = 0; j < n; ++j) {
            if (i == j) { dists.append({0.0, j}); continue; }
            double sqDist = 0.0;
            int dims = qMin(dataPoints[i].size(), dataPoints[j].size());
            for (int d = 0; d < dims; ++d) {
                double diff = dataPoints[i][d] - dataPoints[j][d];
                sqDist += diff * diff;
            }
            dists.append({sqDist, j});
        }
        std::sort(dists.begin(), dists.end());

        int knn = qMin(kNeighbors + 1, n);
        for (int rank = 0; rank < knn; ++rank) {
            int j = dists[rank].second;
            if (j == i) continue;
            double w = qExp(-dists[rank].first / sigma2);
            affinity[i][j] = w;
            affinity[j][i] = w;
        }
    }
    return affinity;
}

/**
 * @brief 计算归一化图拉普拉斯矩阵
 *
 * L_norm = I - D^{-1/2} * W * D^{-1/2}
 * 其中D为度矩阵，W为亲和矩阵。
 *
 * @param affinityMatrix 亲和矩阵
 * @return 归一化拉普拉斯矩阵
 */
QVector<QVector<double>> SpectralClustering9::normalizedLaplacian(
    const QVector<QVector<double>>& affinityMatrix)
{
    const int n = affinityMatrix.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    if (n == 0) return L;

    /* 计算度向量 */
    QVector<double> degree(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            degree[i] += affinityMatrix[i][j];

    /* D^{-1/2} */
    QVector<double> invSqrtD(n, 0.0);
    for (int i = 0; i < n; ++i)
        invSqrtD[i] = (degree[i] > 1e-15) ? 1.0 / qSqrt(degree[i]) : 0.0;

    /* L_norm = I - D^{-1/2} * W * D^{-1/2} */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double val = invSqrtD[i] * affinityMatrix[i][j] * invSqrtD[j];
            L[i][j] = (i == j) ? 1.0 - val : -val;
        }
    }
    return L;
}

/**
 * @brief 基于特征间隙启发式自动选择聚类数
 *
 * 计算拉普拉斯矩阵特征值，找到相邻特征值差值最大的间隙位置，
 * 以此作为最优聚类数。
 *
 * @param dataPoints 数据点集合
 * @param maxK 最大候选聚类数
 * @return 推荐的聚类数
 */
int SpectralClustering9::autoSelectK(const QVector<QVector<double>>& dataPoints, int maxK)
{
    const int n = dataPoints.size();
    if (n == 0) return 1;

    int knnK = qMin(7, n - 1);
    QVector<QVector<double>> affinity = buildKNNAffinity(dataPoints, knnK, 1.0);
    QVector<QVector<double>> L = normalizedLaplacian(affinity);

    int numEigenvalues = qMin(maxK + 1, n);

    /* 通过Rayleigh商迭代估计前几个最小特征值 */
    QVector<double> eigenvalues(numEigenvalues, 0.0);
    QVector<QVector<double>> eigenVectors(numEigenvalues, QVector<double>(n, 0.0));

    for (int ev = 0; ev < numEigenvalues; ++ev) {
        QVector<double> v(n, 0.01 / (ev + 1));
        double eigenval = 0.0;

        for (int iter = 0; iter < 100; ++iter) {
            QVector<double> Lv(n, 0.0);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    Lv[i] += L[i][j] * v[j];

            /* 正交化 */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i)
                    dot += Lv[i] * eigenVectors[prev][i];
                for (int i = 0; i < n; ++i)
                    Lv[i] -= dot * eigenVectors[prev][i];
            }

            double norm = 0.0;
            for (int i = 0; i < n; ++i)
                norm += Lv[i] * Lv[i];
            norm = qSqrt(norm);
            if (norm < 1e-15) break;

            for (int i = 0; i < n; ++i)
                v[i] = Lv[i] / norm;
        }

        /* Rayleigh商求特征值 */
        double num = 0.0, den = 0.0;
        QVector<double> Lv(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j)
                Lv[i] += L[i][j] * v[j];
            num += v[i] * Lv[i];
            den += v[i] * v[i];
        }
        eigenvalues[ev] = (den > 1e-15) ? num / den : 0.0;
        eigenVectors[ev] = v;
    }

    /* 寻找最大特征间隙 */
    double maxGap = 0.0;
    int bestK = 1;
    for (int i = 1; i < numEigenvalues; ++i) {
        double gap = qAbs(eigenvalues[i] - eigenvalues[i - 1]);
        if (gap > maxGap) {
            maxGap = gap;
            bestK = i;
        }
    }
    return qMax(1, qMin(bestK, maxK));
}
