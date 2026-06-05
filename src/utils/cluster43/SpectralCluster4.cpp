/**
 * @file SpectralCluster4.cpp
 * @brief 谱聚类4实现 — 多流形+自适应核宽
 *
 * 基于图拉普拉斯特征分解的多流形谱聚类算法实现。
 * 支持自适应核宽计算和多种流形结构发现。
 */

#include "utils/cluster43/SpectralCluster4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SpectralCluster4::SpectralCluster4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置聚类数目
 * @param k 目标聚类数，必须 >= 2
 */
void SpectralCluster4::setNumClusters(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 启用/禁用自适应核宽
 * @param adaptive true启用自适应，false使用固定带宽
 */
void SpectralCluster4::setAdaptiveBandwidth(bool adaptive)
{
    m_adaptive = adaptive;
}

/**
 * @brief 设置流形数量
 * @param manifolds 流形数量，用于多流形聚类
 */
void SpectralCluster4::setManifoldCount(int manifolds)
{
    m_manifolds = qMax(1, manifolds);
}

/**
 * @brief 执行谱聚类拟合
 * @param data 输入数据点集合，每个QVector<double>代表一个数据点
 * @return 聚类标签向量，每个元素为对应数据点的簇编号
 *
 * 流程: 构建亲和矩阵 -> 特征分解 -> KMeans投影
 */
QVector<int> SpectralCluster4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) {
        return QVector<int>();
    }

    /* ---- 第一步: 构建自适应亲和矩阵 ---- */
    if (m_adaptive) {
        buildAdaptiveAffinity(data);
    } else {
        /* 固定核宽方式构建亲和矩阵 */
        const double sigma = 1.0;
        m_affinity.resize(n);
        for (int i = 0; i < n; ++i) {
            m_affinity[i].resize(n);
            for (int j = 0; j < n; ++j) {
                double distSq = 0.0;
                for (int d = 0; d < data[i].size() && d < data[j].size(); ++d) {
                    double diff = data[i][d] - data[j][d];
                    distSq += diff * diff;
                }
                m_affinity[i][j] = qExp(-distSq / (2.0 * sigma * sigma));
            }
        }
    }

    /* ---- 第二步: 归一化拉普拉斯矩阵 L = D^{-1/2} W D^{-1/2} ---- */
    QVector<double> dInvSqrt(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += m_affinity[i][j];
        }
        dInvSqrt[i] = (sum > 1e-12) ? 1.0 / qSqrt(sum) : 0.0;
    }

    /* L_norm = D^{-1/2} W D^{-1/2} */
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            L[i][j] = dInvSqrt[i] * m_affinity[i][j] * dInvSqrt[j];
        }
    }

    /* ---- 第三步: 幂迭代法求前k个特征向量 ---- */
    const int numEigenvectors = qMin(m_k * m_manifolds, n);
    QVector<QVector<double>> eigenvectors(numEigenvectors, QVector<double>(n, 0.0));

    for (int ev = 0; ev < numEigenvectors; ++ev) {
        /* 随机初始化 */
        eigenvectors[ev].resize(n);
        for (int i = 0; i < n; ++i) {
            eigenvectors[ev][i] = qSin(static_cast<double>(i + 1) * (ev + 1) * 0.1);
        }

        /* 幂迭代 */
        for (int iter = 0; iter < 100; ++iter) {
            QVector<double> newVec(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    newVec[i] += L[i][j] * eigenvectors[ev][j];
                }
            }

            /* 正交化 (Gram-Schmidt) */
            for (int prev = 0; prev < ev; ++prev) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) {
                    dot += newVec[i] * eigenvectors[prev][i];
                }
                for (int i = 0; i < n; ++i) {
                    newVec[i] -= dot * eigenvectors[prev][i];
                }
            }

            /* 归一化 */
            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += newVec[i] * newVec[i];
            }
            norm = qSqrt(norm);
            if (norm < 1e-12) break;

            for (int i = 0; i < n; ++i) {
                eigenvectors[ev][i] = newVec[i] / norm;
            }
        }
    }

    m_stats.totalEigenDecomps += numEigenvectors;

    /* ---- 第四步: 行归一化特征向量矩阵 ---- */
    QVector<QVector<double>> embedding(n, QVector<double>(numEigenvectors, 0.0));
    for (int i = 0; i < n; ++i) {
        double rowNorm = 0.0;
        for (int ev = 0; ev < numEigenvectors; ++ev) {
            rowNorm += eigenvectors[ev][i] * eigenvectors[ev][i];
        }
        rowNorm = qSqrt(rowNorm);
        if (rowNorm > 1e-12) {
            for (int ev = 0; ev < numEigenvectors; ++ev) {
                embedding[i][ev] = eigenvectors[ev][i] / rowNorm;
            }
        }
    }

    /* ---- 第五步: KMeans聚类 ---- */
    QVector<int> labels(n, 0);
    QVector<QVector<double>> centers(m_k, QVector<double>(numEigenvectors, 0.0));

    /* 初始化聚类中心 (KMeans++) */
    centers[0] = embedding[0];
    for (int c = 1; c < m_k; ++c) {
        QVector<double> dists(n, 0.0);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minDist = 1e18;
            for (int j = 0; j < c; ++j) {
                double d = 0.0;
                for (int dim = 0; dim < numEigenvectors; ++dim) {
                    double diff = embedding[i][dim] - centers[j][dim];
                    d += diff * diff;
                }
                minDist = qMin(minDist, d);
            }
            dists[i] = minDist;
            totalDist += minDist;
        }
        /* 按距离加权选择下一个中心 */
        double threshold = totalDist * qSin(static_cast<double>(c) * 1.234 + 0.567);
        threshold = qFabs(threshold);
        if (totalDist > 1e-12) {
            threshold = fmod(threshold, totalDist);
        }
        double accum = 0.0;
        for (int i = 0; i < n; ++i) {
            accum += dists[i];
            if (accum >= threshold) {
                centers[c] = embedding[i];
                break;
            }
        }
    }

    /* KMeans迭代 */
    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;

        /* 分配步骤 */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestLabel = 0;
            for (int c = 0; c < m_k; ++c) {
                double dist = 0.0;
                for (int dim = 0; dim < numEigenvectors; ++dim) {
                    double diff = embedding[i][dim] - centers[c][dim];
                    dist += diff * diff;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestLabel = c;
                }
            }
            if (labels[i] != bestLabel) {
                labels[i] = bestLabel;
                changed = true;
            }
        }

        if (!changed) break;

        /* 更新中心 */
        QVector<int> counts(m_k, 0);
        for (int c = 0; c < m_k; ++c) {
            centers[c].fill(0.0);
        }
        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            counts[c]++;
            for (int dim = 0; dim < numEigenvectors; ++dim) {
                centers[c][dim] += embedding[i][dim];
            }
        }
        for (int c = 0; c < m_k; ++c) {
            if (counts[c] > 0) {
                for (int dim = 0; dim < numEigenvectors; ++dim) {
                    centers[c][dim] /= counts[c];
                }
            }
        }
    }

    /* ---- 更新统计信息 ---- */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_k, n);
    return labels;
}

/**
 * @brief 计算单个点的自适应核宽
 * @param data 数据集
 * @param pointIdx 目标点索引
 * @param knn 近邻数目
 * @return 自适应核宽值sigma
 */
QVector<double> SpectralCluster4::adaptiveSigma(const QVector<QVector<double>>& data,
                                                  int pointIdx, int knn) const
{
    const int n = data.size();
    const int dim = data[pointIdx].size();

    /* 计算到所有点的距离 */
    QVector<QPair<double, int>> distances;
    distances.reserve(n);
    for (int j = 0; j < n; ++j) {
        double distSq = 0.0;
        for (int d = 0; d < dim && d < data[j].size(); ++d) {
            double diff = data[pointIdx][d] - data[j][d];
            distSq += diff * diff;
        }
        distances.append({distSq, j});
    }

    /* 排序取前knn个近邻的平均距离 */
    std::sort(distances.begin(), distances.end());
    double avgDist = 0.0;
    int actualKnn = qMin(knn, n - 1);
    for (int i = 1; i <= actualKnn; ++i) {
        avgDist += qSqrt(distances[i].first);
    }
    avgDist /= actualKnn;

    /* 返回自适应sigma值 */
    QVector<double> result(1);
    result[0] = (avgDist > 1e-12) ? avgDist : 1.0;
    return result;
}

/**
 * @brief 构建自适应亲和矩阵
 * @param data 输入数据集
 *
 * 使用局部自适应核宽，对每个点独立计算带宽参数。
 */
void SpectralCluster4::buildAdaptiveAffinity(const QVector<QVector<double>>& data)
{
    const int n = data.size();
    const int knn = qMin(7, n - 1);

    m_affinity.resize(n);
    for (int i = 0; i < n; ++i) {
        m_affinity[i].resize(n, 0.0);
    }

    for (int i = 0; i < n; ++i) {
        QVector<double> sigmaI = adaptiveSigma(data, i, knn);
        for (int j = i; j < n; ++j) {
            QVector<double> sigmaJ = adaptiveSigma(data, j, knn);
            double distSq = 0.0;
            for (int d = 0; d < data[i].size() && d < data[j].size(); ++d) {
                double diff = data[i][d] - data[j][d];
                distSq += diff * diff;
            }

            /* 对称自适应核: exp(-d^2 / (sigma_i * sigma_j)) */
            double sigmaProd = sigmaI[0] * sigmaJ[0];
            double val = (sigmaProd > 1e-24) ? qExp(-distSq / sigmaProd) : 0.0;
            m_affinity[i][j] = val;
            m_affinity[j][i] = val;
        }
    }
}

/**
 * @brief 重置所有统计信息
 */
void SpectralCluster4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
