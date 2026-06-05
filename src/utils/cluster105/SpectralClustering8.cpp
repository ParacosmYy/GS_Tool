#include "SpectralClustering8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化谱聚类引擎
 * @param parent 父对象指针
 */
SpectralClustering8::SpectralClustering8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SpectralClustering8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_embeddings.clear();
}

/**
 * @brief 从距离矩阵构建相似度矩阵
 *
 * 使用高斯核函数将距离转换为相似度：exp(-d^2 / (2 * sigma^2))
 * 对角线置零去除自环，输出对称亲和矩阵。
 *
 * @param distanceMatrix n×n距离矩阵（对称）
 * @param sigma 高斯核宽度参数
 * @return 相似度矩阵
 */
QVector<QVector<double>> SpectralClustering8::buildSimilarityMatrix(
    const QVector<QVector<double>>& distanceMatrix, double sigma) const
{
    const int n = distanceMatrix.size();
    if (n == 0) return {};

    const double twoSigmaSq = 2.0 * sigma * sigma;
    if (twoSigmaSq < 1e-15) return QVector<QVector<double>>(n, QVector<double>(n, 0.0));

    QVector<QVector<double>> sim(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                sim[i][j] = 0.0;
            } else {
                double d = (j < distanceMatrix[i].size()) ? distanceMatrix[i][j] : 0.0;
                sim[i][j] = qExp(-(d * d) / twoSigmaSq);
            }
        }
    }
    return sim;
}

/**
 * @brief 幂迭代提取前k个特征向量
 *
 * 通过随机初始化和正交化逐步提取拉普拉斯矩阵的
 * 前k个最大特征值对应的特征向量。
 *
 * @param mat 对称矩阵
 * @param k 目标特征向量数
 * @return 特征向量矩阵(n×k)，每行为一个特征向量
 */
static QVector<QVector<double>> powerEigvecs(
    const QVector<QVector<double>>& mat, int k)
{
    const int n = mat.size();
    QVector<QVector<double>> vecs(k, QVector<double>(n));

    for (int ev = 0; ev < k; ++ev) {
        QVector<double> v(n);
        for (int i = 0; i < n; ++i) {
            v[i] = qSin(double(i * 7 + ev * 13 + 1)) * 0.5;
        }

        for (int iter = 0; iter < 200; ++iter) {
            /* 矩阵乘法 */
            QVector<double> mv(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    mv[i] += mat[i][j] * v[j];
                }
            }

            /* 正交化：减去已找到的特征向量分量 */
            for (int p = 0; p < ev; ++p) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += mv[i] * vecs[p][i];
                for (int i = 0; i < n; ++i) mv[i] -= dot * vecs[p][i];
            }

            /* 归一化 */
            double norm = 0.0;
            for (int i = 0; i < n; ++i) norm += mv[i] * mv[i];
            norm = qSqrt(qMax(norm, 1e-15));
            for (int i = 0; i < n; ++i) v[i] = mv[i] / norm;
        }
        vecs[ev] = v;
    }
    return vecs;
}

/**
 * @brief 在嵌入空间上执行K-Means聚类
 *
 * 标准K-Means迭代：分配步骤 + 更新步骤，最多80轮。
 *
 * @param pts 数据点集合(n×dim)
 * @param k 目标簇数
 * @return 每个点的簇标签
 */
static QVector<int> localKMeans(const QVector<QVector<double>>& pts, int k)
{
    const int n = pts.size();
    if (n == 0) return {};
    const int dim = pts[0].size();
    QVector<int> labels(n, 0);
    QVector<QVector<double>> centers(k);

    for (int c = 0; c < k; ++c) centers[c] = pts[c % n];

    for (int it = 0; it < 80; ++it) {
        bool changed = false;
        /* 分配步骤 */
        for (int i = 0; i < n; ++i) {
            double best = 1e18;
            int bc = 0;
            for (int c = 0; c < k; ++c) {
                double dsq = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = pts[i][d] - centers[c][d];
                    dsq += diff * diff;
                }
                if (dsq < best) { best = dsq; bc = c; }
            }
            if (labels[i] != bc) { labels[i] = bc; changed = true; }
        }
        if (!changed) break;

        /* 更新质心 */
        QVector<QVector<double>> nc(k, QVector<double>(dim, 0.0));
        QVector<int> cnt(k, 0);
        for (int i = 0; i < n; ++i) {
            cnt[labels[i]]++;
            for (int d = 0; d < dim; ++d) nc[labels[i]][d] += pts[i][d];
        }
        for (int c = 0; c < k; ++c) {
            if (cnt[c] > 0) {
                for (int d = 0; d < dim; ++d) nc[c][d] /= cnt[c];
                centers[c] = nc[c];
            }
        }
    }
    return labels;
}

/**
 * @brief 执行谱聚类
 *
 * 算法步骤：
 * 1. 计算度向量 d[i] = sum_j(W[i][j])
 * 2. 构建归一化拉普拉斯 L_sym = I - D^{-1/2} W D^{-1/2}
 * 3. 幂迭代提取前k个特征向量
 * 4. 行归一化特征向量矩阵得到低维嵌入
 * 5. K-Means聚类得到最终标签
 *
 * @param similarityMatrix n×n相似度矩阵（对称）
 * @param k 目标簇数
 * @return 每个样本的簇标签
 */
QVector<int> SpectralClustering8::fit(
    const QVector<QVector<double>>& similarityMatrix, int k)
{
    QElapsedTimer timer;
    timer.start();

    const int n = similarityMatrix.size();
    QVector<int> labels(n, 0);
    m_embeddings.clear();

    if (n == 0 || k <= 0) {
        emit clusteringCompleted(0);
        return labels;
    }

    k = qMin(k, n);
    if (n < 2) {
        emit clusteringCompleted(1);
        return labels;
    }

    /* 计算度向量 */
    QVector<double> deg(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            deg[i] += (j < similarityMatrix[i].size()) ? similarityMatrix[i][j] : 0.0;
        }
    }

    /* 构建归一化拉普拉斯 L_sym = I - D^{-1/2} W D^{-1/2} */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double di = (deg[i] > 1e-12) ? 1.0 / qSqrt(deg[i]) : 0.0;
            double dj = (deg[j] > 1e-12) ? 1.0 / qSqrt(deg[j]) : 0.0;
            double w = (j < similarityMatrix[i].size()) ? similarityMatrix[i][j] : 0.0;
            L[i][j] = (i == j ? 1.0 : 0.0) - di * w * dj;
        }
    }

    /* 特征分解：幂迭代求前k个特征向量 */
    auto evecs = powerEigvecs(L, k);

    /* 构建嵌入矩阵并逐行归一化 */
    m_embeddings.resize(n);
    for (int i = 0; i < n; ++i) {
        m_embeddings[i].resize(k);
        for (int ev = 0; ev < k; ++ev) {
            m_embeddings[i][ev] = evecs[ev][i];
        }
        double norm = 0.0;
        for (int ev = 0; ev < k; ++ev) {
            norm += m_embeddings[i][ev] * m_embeddings[i][ev];
        }
        norm = qSqrt(qMax(norm, 1e-15));
        for (int ev = 0; ev < k; ++ev) {
            m_embeddings[i][ev] /= norm;
        }
    }

    /* 在嵌入空间上执行K-Means */
    labels = localKMeans(m_embeddings, k);

    /* 更新统计信息 */
    m_stats.totalClusteringRuns++;
    m_stats.embeddingDimensions = k;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusteringRuns;

    emit clusteringCompleted(k);
    return labels;
}
