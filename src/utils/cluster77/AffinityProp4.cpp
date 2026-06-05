/**
 * @file AffinityProp4.cpp
 * @brief 近邻传播聚类算法实现 — Responsibility/Availability消息传递
 */

#include "utils/cluster77/AffinityProp4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
AffinityProp4::AffinityProp4(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置偏好值 @param preference 偏好值，越大产生越多簇 */
void AffinityProp4::setPreference(double preference)
{
    m_preference = preference;
}

/** @brief 设置阻尼系数 @param damping 0.5~1.0，防止震荡 */
void AffinityProp4::setDamping(double damping)
{
    m_damping = qBound(0.5, damping, 0.99);
}

/** @brief 重置统计数据 */
void AffinityProp4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_exemplars.clear();
}

/**
 * @brief 执行近邻传播聚类
 * @param similarity 相似度矩阵(n×n)
 * @param maxIterations 最大迭代次数
 * @return 簇标签(每个样本)
 *
 * 通过R和A消息交替更新，收敛后提取代表点。
 */
QVector<int> AffinityProp4::fit(const QVector<QVector<double>>& similarity,
                                int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    int n = similarity.size();
    QVector<int> labels(n, -1);
    m_exemplars.clear();

    if (n < 2) {
        if (n == 1) labels[0] = 0;
        return labels;
    }

    /* 对角线设为偏好值 */
    QVector<QVector<double>> S = similarity;
    for (int i = 0; i < n; ++i) {
        S[i][i] = m_preference;
    }

    /* R和A消息矩阵 */
    QVector<QVector<double>> R(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));

    int convergenceIter = 0;
    const int convergenceWindow = 15;
    QVector<int> prevExemplars;

    for (int iter = 0; iter < maxIterations; ++iter) {
        /* 更新Responsibility: R(i,k) = S(i,k) - max_{k'!=k}{A(i,k') + S(i,k')} */
        QVector<QVector<double>> newR(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                double maxVal = -1e18;
                for (int kk = 0; kk < n; ++kk) {
                    if (kk != k) {
                        maxVal = std::max(maxVal, A[i][kk] + S[i][kk]);
                    }
                }
                newR[i][k] = S[i][k] - maxVal;
            }
        }

        /* 阻尼更新R */
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                R[i][k] = m_damping * R[i][k] + (1.0 - m_damping) * newR[i][k];
            }
        }

        /* 更新Availability: A(i,k) = min(0, R(k,k) + sum_{j!=i,k} max(0, R(j,k))) */
        QVector<QVector<double>> newA(n, QVector<double>(n, 0.0));
        for (int k = 0; k < n; ++k) {
            double sumRkk = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != k) sumRkk += std::max(0.0, R[j][k]);
            }

            for (int i = 0; i < n; ++i) {
                if (i == k) {
                    newA[i][k] = sumRkk;
                } else {
                    double sumExcl = sumRkk - std::max(0.0, R[i][k]);
                    newA[i][k] = std::min(0.0, R[k][k] + sumExcl);
                }
            }
        }

        /* 阻尼更新A */
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < n; ++k) {
                A[i][k] = m_damping * A[i][k] + (1.0 - m_damping) * newA[i][k];
            }
        }

        /* 每10步检查收敛 */
        if ((iter + 1) % 10 == 0) {
            QVector<int> curExemplars;
            for (int i = 0; i < n; ++i) {
                double maxVal = -1e18;
                int best = i;
                for (int k = 0; k < n; ++k) {
                    double val = R[i][k] + A[i][k];
                    if (val > maxVal) {
                        maxVal = val;
                        best = k;
                    }
                }
                if (best == i) curExemplars.append(i);
            }

            if (curExemplars == prevExemplars) {
                convergenceIter++;
                if (convergenceIter >= 3) break;
            } else {
                convergenceIter = 0;
                prevExemplars = curExemplars;
            }
        }
    }

    /* 提取代表点 */
    for (int i = 0; i < n; ++i) {
        double maxVal = -1e18;
        int best = i;
        for (int k = 0; k < n; ++k) {
            double val = R[i][k] + A[i][k];
            if (val > maxVal) {
                maxVal = val;
                best = k;
            }
        }
        if (best == i) m_exemplars.append(i);
    }

    /* 分配标签 */
    if (!m_exemplars.isEmpty()) {
        for (int i = 0; i < n; ++i) {
            double minDist = 1e18;
            int bestLabel = 0;
            for (int e = 0; e < m_exemplars.size(); ++e) {
                double dist = -S[i][m_exemplars[e]];
                if (dist < minDist) {
                    minDist = dist;
                    bestLabel = e;
                }
            }
            labels[i] = bestLabel;
        }
    }

    m_stats.totalIterations += maxIterations;
    m_stats.totalClusters += m_exemplars.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / std::max(1, m_stats.totalClusters);

    emit clusteringCompleted(m_exemplars.size(), maxIterations);
    return labels;
}

/** @brief 获取簇中心索引 */
QVector<int> AffinityProp4::exemplars() const
{
    return m_exemplars;
}
