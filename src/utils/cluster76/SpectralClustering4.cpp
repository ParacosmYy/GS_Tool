/**
 * @file SpectralClustering4.cpp
 * @brief 谱聚类算法实现 — 图拉普拉斯特征分解 + K-Means后处理
 */

#include "utils/cluster76/SpectralClustering4.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SpectralClustering4::SpectralClustering4(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置相似度矩阵构建方式 @param type rbf或knn @param param 对应参数 */
void SpectralClustering4::setAffinity(const QString& type, double param)
{
    m_affinityType = type;
    m_affinityParam = param;
}

/** @brief 重置统计数据 */
void SpectralClustering4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_embedding.clear();
}

/**
 * @brief 构建RBF相似度矩阵
 * @param data 输入数据
 * @return 相似度矩阵W
 */
static QVector<QVector<double>> buildRBFAffinity(
    const QVector<QVector<double>>& data, double gamma)
{
    int n = data.size();
    int dim = data[0].size();
    QVector<QVector<double>> W(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double sumSq = 0.0;
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - data[j][d];
                sumSq += diff * diff;
            }
            double val = qExp(-gamma * sumSq);
            W[i][j] = W[j][i] = val;
        }
    }
    return W;
}

/**
 * @brief 幂迭代法求前k个特征向量
 * @param mat 对称矩阵
 * @param k 特征向量数
 * @return 特征向量矩阵(n×k)
 */
static QVector<QVector<double>> powerIteration(
    const QVector<QVector<double>>& mat, int k)
{
    int n = mat.size();
    QVector<QVector<double>> eigenVecs(k, QVector<double>(n, 0.0));

    for (int ev = 0; ev < k; ++ev) {
        /* 随机初始化 */
        QVector<double> v(n);
        for (int i = 0; i < n; ++i) {
            v[i] = QRandomGenerator::global()->bounded(1000) / 1000.0 - 0.5;
        }

        for (int iter = 0; iter < 200; ++iter) {
            QVector<double> mv(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    mv[i] += mat[i][j] * v[j];
                }
            }

            /* 减去已知特征向量投影(正交化) */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) {
                    dot += mv[i] * eigenVecs[prev][i];
                }
                for (int i = 0; i < n; ++i) {
                    mv[i] -= dot * eigenVecs[prev][i];
                }
            }

            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += mv[i] * mv[i];
            }
            norm = qSqrt(norm);
            if (norm < 1e-12) break;

            for (int i = 0; i < n; ++i) {
                v[i] = mv[i] / norm;
            }
        }
        eigenVecs[ev] = v;
    }
    return eigenVecs;
}

/**
 * @brief 简单K-Means用于嵌入空间聚类
 * @param points 数据点(k×n矩阵转置为n×k)
 * @param k 簇数
 * @return 标签
 */
static QVector<int> simpleKMeans(const QVector<QVector<double>>& points, int k)
{
    int n = points.size();
    int dim = points[0].size();
    QVector<int> labels(n, 0);

    /* 初始化中心 */
    QVector<QVector<double>> centers(k);
    for (int c = 0; c < k; ++c) {
        centers[c] = points[c % n];
    }

    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;

        /* 分配 */
        for (int i = 0; i < n; ++i) {
            double minDist = 1e18;
            int best = 0;
            for (int c = 0; c < k; ++c) {
                double distSq = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = points[i][d] - centers[c][d];
                    distSq += diff * diff;
                }
                if (distSq < minDist) {
                    minDist = distSq;
                    best = c;
                }
            }
            if (labels[i] != best) {
                labels[i] = best;
                changed = true;
            }
        }

        if (!changed) break;

        /* 更新中心 */
        QVector<QVector<double>> newCenters(k, QVector<double>(dim, 0.0));
        QVector<int> counts(k, 0);
        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            counts[c]++;
            for (int d = 0; d < dim; ++d) {
                newCenters[c][d] += points[i][d];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < dim; ++d) {
                    newCenters[c][d] /= counts[c];
                }
                centers[c] = newCenters[c];
            }
        }
    }
    return labels;
}

/**
 * @brief 执行谱聚类
 * @param data 输入数据
 * @param numClusters 目标簇数
 * @return 簇标签
 *
 * 构建相似度矩阵→计算归一化拉普拉斯→特征分解→K-Means。
 */
QVector<int> SpectralClustering4::fit(
    const QVector<QVector<double>>& data, int numClusters)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<int> labels(n, 0);
    m_embedding.clear();

    if (n < 2 || numClusters < 1) return labels;
    numClusters = std::min(numClusters, n);

    /* 构建相似度矩阵 */
    QVector<QVector<double>> W = buildRBFAffinity(data, m_affinityParam);

    /* 计算度矩阵D和归一化拉普拉斯 L_sym = I - D^{-1/2} W D^{-1/2} */
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            d[i] += W[i][j];
        }
    }

    QVector<QVector<double>> Lsym(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double diInv = (d[i] > 1e-12) ? 1.0 / qSqrt(d[i]) : 0.0;
            double djInv = (d[j] > 1e-12) ? 1.0 / qSqrt(d[j]) : 0.0;
            Lsym[i][j] = (i == j ? 1.0 : 0.0) - diInv * W[i][j] * djInv;
        }
    }

    /* 求前k个特征向量 */
    int k = numClusters;
    QVector<QVector<double>> eigenVecs = powerIteration(Lsym, k);

    /* 构建嵌入矩阵 (n×k) */
    m_embedding.resize(n);
    for (int i = 0; i < n; ++i) {
        m_embedding[i].resize(k);
        for (int ev = 0; ev < k; ++ev) {
            m_embedding[i][ev] = eigenVecs[ev][i];
        }
        /* 行归一化 */
        double norm = 0.0;
        for (int ev = 0; ev < k; ++ev) {
            norm += m_embedding[i][ev] * m_embedding[i][ev];
        }
        norm = qSqrt(norm);
        if (norm > 1e-12) {
            for (int ev = 0; ev < k; ++ev) {
                m_embedding[i][ev] /= norm;
            }
        }
    }

    /* K-Means聚类 */
    labels = simpleKMeans(m_embedding, numClusters);

    m_stats.totalClusterings++;
    m_stats.totalEigenvalues += k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / std::max(1, m_stats.totalClusterings);

    double cutVal = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (labels[i] != labels[j]) {
                cutVal += W[i][j];
            }
        }
    }
    emit clusteringCompleted(numClusters, cutVal);
    return labels;
}

/** @brief 获取谱嵌入向量 */
QVector<QVector<double>> SpectralClustering4::spectralEmbedding(int dimensions) const
{
    int d = std::min(dimensions, (int)m_embedding.size());
    QVector<QVector<double>> result(m_embedding.size());
    for (int i = 0; i < m_embedding.size(); ++i) {
        int cols = std::min(d, (int)m_embedding[i].size());
        result[i] = m_embedding[i].mid(0, cols);
    }
    return result;
}
