/**
 * @file SpectralClustering3.cpp
 * @brief 谱聚类算法实现 — 基于核矩阵特征分解的无监督聚类
 *
 * 支持RBF/多项式/线性核函数，通过拉普拉斯矩阵特征向量
 * 嵌入后使用K-Means离散化完成聚类，并计算轮廓系数评估质量。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/cluster60/SpectralClustering3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
SpectralClustering3::SpectralClustering3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置聚类数量
 * @param k 目标聚类数，必须 >= 2
 */
void SpectralClustering3::setNumClusters(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 设置核函数类型
 * @param type 核函数名称: "rbf"(默认), "poly", "linear"
 */
void SpectralClustering3::setKernelType(const QString& type)
{
    if (type == "rbf" || type == "poly" || type == "linear") {
        m_kernel = type;
    }
}

/**
 * @brief 设置核函数参数
 * @param param RBF的gamma值或多项式核的度数
 */
void SpectralClustering3::setKernelParam(double param)
{
    m_param = qMax(1e-9, param);
}

/**
 * @brief 对给定数据点执行谱聚类
 *
 * 流程: 构建核矩阵 -> 归一化拉普拉斯 -> 取前k个特征向量 ->
 *       行归一化 -> K-Means离散化 -> 计算轮廓系数
 *
 * @param points 输入数据点集合，每个点为特征向量
 * @return 聚类标签向量，labels[i]表示第i个点的簇编号
 */
QVector<int> SpectralClustering3::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    if (n == 0) {
        return {};
    }

    /* 步骤1: 构建核矩阵 (相似度矩阵) */
    QVector<QVector<double>> K = buildKernelMatrix(points);

    /* 步骤2: 构建归一化拉普拉斯矩阵 L = D^{-1/2} K D^{-1/2} */
    QVector<double> d(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            d[i] += K[i][j];
        }
        d[i] = qMax(1e-12, d[i]);
    }

    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        double invSqrtDi = 1.0 / qSqrt(d[i]);
        for (int j = 0; j < n; ++j) {
            double invSqrtDj = 1.0 / qSqrt(d[j]);
            L[i][j] = K[i][j] * invSqrtDi * invSqrtDj;
        }
    }

    /* 步骤3: 幂迭代法提取前k个特征向量 */
    const int dim = qMin(m_k, n);
    m_embedding.resize(n);
    for (int i = 0; i < n; ++i) {
        m_embedding[i].resize(dim);
    }

    /* 使用随机初始化 + 幂迭代近似特征向量 */
    for (int ev = 0; ev < dim; ++ev) {
        QVector<double> v(n, 0.0);
        for (int i = 0; i < n; ++i) {
            v[i] = qSin(static_cast<double>(i + 1) * (ev + 1) * 0.618);
        }

        /* 正交化: 减去之前特征向量的投影 */
        for (int prev = 0; prev < ev; ++prev) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) {
                dot += v[i] * m_embedding[i][prev];
            }
            for (int i = 0; i < n; ++i) {
                v[i] -= dot * m_embedding[i][prev];
            }
        }

        /* 幂迭代 */
        for (int iter = 0; iter < 100; ++iter) {
            QVector<double> vNew(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    vNew[i] += L[i][j] * v[j];
                }
            }
            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += vNew[i] * vNew[i];
            }
            norm = qSqrt(qMax(1e-12, norm));
            for (int i = 0; i < n; ++i) {
                v[i] = vNew[i] / norm;
            }
        }

        /* 存储特征向量 */
        for (int i = 0; i < n; ++i) {
            m_embedding[i][ev] = v[i];
        }
    }

    /* 步骤4: 行归一化嵌入向量 */
    for (int i = 0; i < n; ++i) {
        double norm = 0.0;
        for (int j = 0; j < dim; ++j) {
            norm += m_embedding[i][j] * m_embedding[i][j];
        }
        norm = qSqrt(qMax(1e-12, norm));
        for (int j = 0; j < dim; ++j) {
            m_embedding[i][j] /= norm;
        }
    }

    /* 步骤5: K-Means离散化 */
    QVector<int> labels = discretize(m_embedding, m_k);

    /* 步骤6: 计算轮廓系数 */
    m_silhouette = 0.0;
    if (n >= 2) {
        double silSum = 0.0;
        /* 计算嵌入空间中的距离矩阵 */
        QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double d2 = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = m_embedding[i][dd] - m_embedding[j][dd];
                    d2 += diff * diff;
                }
                dist[i][j] = dist[j][i] = qSqrt(d2);
            }
        }

        for (int i = 0; i < n; ++i) {
            int myCluster = labels[i];
            /* 计算簇内平均距离 a(i) */
            double a = 0.0;
            int clusterSize = 0;
            for (int j = 0; j < n; ++j) {
                if (j != i && labels[j] == myCluster) {
                    a += dist[i][j];
                    ++clusterSize;
                }
            }
            a = (clusterSize > 0) ? a / clusterSize : 0.0;

            /* 计算最近簇的平均距离 b(i) */
            double b = 1e18;
            for (int c = 0; c < m_k; ++c) {
                if (c == myCluster) continue;
                double avgDist = 0.0;
                int cnt = 0;
                for (int j = 0; j < n; ++j) {
                    if (labels[j] == c) {
                        avgDist += dist[i][j];
                        ++cnt;
                    }
                }
                if (cnt > 0) {
                    avgDist /= cnt;
                    b = qMin(b, avgDist);
                }
            }
            if (b > 1e17) b = 0.0;
            double sil = (b + a < 1e-12) ? 0.0 : (b - a) / qMax(a, b);
            silSum += sil;
        }
        m_silhouette = silSum / n;
    }

    /* 更新统计信息 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_k, m_silhouette);
    return labels;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralClustering3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 构建核矩阵
 *
 * 根据m_kernel类型计算点对之间的核函数值:
 * - rbf: exp(-gamma * ||x-y||^2)
 * - poly: (gamma * x^T y + 1)^d
 * - linear: x^T y
 *
 * @param pts 输入数据点
 * @return n x n 核矩阵
 */
QVector<QVector<double>> SpectralClustering3::buildKernelMatrix(
    const QVector<QVector<double>>& pts)
{
    const int n = pts.size();
    const int d = (n > 0) ? pts[0].size() : 0;
    QVector<QVector<double>> K(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            double val = 0.0;
            if (m_kernel == "rbf") {
                /* RBF核: exp(-gamma * ||x-y||^2) */
                double dist2 = 0.0;
                for (int dd = 0; dd < d; ++dd) {
                    double diff = pts[i][dd] - pts[j][dd];
                    dist2 += diff * diff;
                }
                val = qExp(-m_param * dist2);
            } else if (m_kernel == "poly") {
                /* 多项式核: (gamma * x^T y + 1)^d */
                double dot = 0.0;
                for (int dd = 0; dd < d; ++dd) {
                    dot += pts[i][dd] * pts[j][dd];
                }
                int degree = qMax(1, static_cast<int>(m_param));
                val = qPow(m_param * dot + 1.0, degree);
            } else {
                /* 线性核: x^T y */
                for (int dd = 0; dd < d; ++dd) {
                    val += pts[i][dd] * pts[j][dd];
                }
            }
            K[i][j] = val;
            K[j][i] = val;
        }
    }
    return K;
}

/**
 * @brief K-Means离散化，将嵌入向量分配到k个簇
 *
 * @param emb 嵌入向量矩阵
 * @param k 目标聚类数
 * @return 聚类标签
 */
QVector<int> SpectralClustering3::discretize(const QVector<QVector<double>>& emb, int k)
{
    const int n = emb.size();
    const int dim = (n > 0) ? emb[0].size() : 0;
    if (n == 0 || dim == 0) return {};

    /* 使用前k个点作为初始质心 (避免空簇) */
    QVector<QVector<double>> centroids(k);
    for (int c = 0; c < k && c < n; ++c) {
        centroids[c] = emb[c];
    }
    for (int c = n; c < k; ++c) {
        centroids[c] = emb[0]; /* 兜底 */
    }

    QVector<int> labels(n, 0);
    const int maxIter = 50;

    for (int iter = 0; iter < maxIter; ++iter) {
        bool changed = false;

        /* 分配步骤: 将每个点分配到最近质心 */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestC = 0;
            for (int c = 0; c < k; ++c) {
                double dist = 0.0;
                for (int dd = 0; dd < dim; ++dd) {
                    double diff = emb[i][dd] - centroids[c][dd];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestC = c;
                }
            }
            if (labels[i] != bestC) {
                labels[i] = bestC;
                changed = true;
            }
        }

        if (!changed) break;

        /* 更新步骤: 重新计算质心 */
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(dim, 0.0);
            int cnt = 0;
            for (int i = 0; i < n; ++i) {
                if (labels[i] == c) {
                    for (int dd = 0; dd < dim; ++dd) {
                        sum[dd] += emb[i][dd];
                    }
                    ++cnt;
                }
            }
            if (cnt > 0) {
                for (int dd = 0; dd < dim; ++dd) {
                    centroids[c][dd] = sum[dd] / cnt;
                }
            }
        }
    }
    return labels;
}
