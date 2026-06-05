/**
 * @file AffinityCluster.cpp
 * @brief 亲和力传播聚类实现 — R/A消息传递+阻尼+收敛检测
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster22/AffinityCluster.h"

#include <QElapsedTimer>
#include <QMap>
#include <QSet>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
AffinityCluster::AffinityCluster(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 拟合数据 — 执行亲和力传播聚类
 *
 * 通过R(responsibility)和A(availability)消息迭代更新,
 * 自动确定簇数和exemplar。收敛条件为连续iter无标签变化。
 *
 * @param similarity N×N相似度矩阵(对称,sim[i][j]越大越相似)
 * @param preference 对角线偏好值(越大簇数越多,默认取相似度中位数)
 * @param maxIter 最大迭代次数
 * @param damping 阻尼系数[0,1),越大越稳定但收敛慢
 */
void AffinityCluster::fit(const QVector<QVector<double>>& similarity,
                           double preference, int maxIter, double damping)
{
    QElapsedTimer timer;
    timer.start();

    m_n = similarity.size();
    if (m_n == 0) {
        m_labels.clear();
        m_exemplars.clear();
        m_residual = 0.0;
        m_iterations = 0;
        emit fitCompleted(0);
        return;
    }

    /* 初始化偏好值:若未指定则取非对角线中位数 */
    double pref = preference;
    if (pref == 0.0) {
        QVector<double> offDiag;
        offDiag.reserve(m_n * (m_n - 1));
        for (int i = 0; i < m_n; ++i) {
            for (int j = 0; j < m_n; ++j) {
                if (i != j && j < similarity[i].size()) {
                    offDiag.append(similarity[i][j]);
                }
            }
        }
        if (!offDiag.isEmpty()) {
            std::sort(offDiag.begin(), offDiag.end());
            pref = offDiag[offDiag.size() / 2];
        }
    }

    /* 构建内部相似度矩阵S, 对角线置为偏好值 */
    QVector<QVector<double>> S(m_n, QVector<double>(m_n, 0.0));
    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j < m_n; ++j) {
            if (j < similarity[i].size()) {
                S[i][j] = (i == j) ? pref : similarity[i][j];
            }
        }
    }

    /* R和A消息矩阵 */
    QVector<QVector<double>> R(m_n, QVector<double>(m_n, 0.0));
    QVector<QVector<double>> A(m_n, QVector<double>(m_n, 0.0));
    QVector<int> labelsPrev(m_n, -1);

    m_iterations = 0;
    m_residual = 0.0;
    int convergenceCount = 0;
    const int convergenceWindow = 10;

    for (int iter = 0; iter < maxIter; ++iter) {
        m_iterations = iter + 1;

        /* 更新Responsibility: R(i,k) = S(i,k) - max_{j!=k}(A(i,j) + S(i,j)) */
        for (int i = 0; i < m_n; ++i) {
            for (int k = 0; k < m_n; ++k) {
                double maxVal = -std::numeric_limits<double>::infinity();
                for (int j = 0; j < m_n; ++j) {
                    if (j != k) {
                        double val = A[i][j] + S[i][j];
                        if (val > maxVal) {
                            maxVal = val;
                        }
                    }
                }
                double newR = S[i][k] - maxVal;
                R[i][k] = damping * R[i][k] + (1.0 - damping) * newR;
            }
        }

        /* 更新Availability: A(i,k) = min(0, R(k,k) + sum_{j!=i,k} max(0,R(j,k))) */
        for (int i = 0; i < m_n; ++i) {
            for (int k = 0; k < m_n; ++k) {
                if (i == k) {
                    /* 自可用性: A(k,k) = sum_{j!=k} max(0, R(j,k)) */
                    double sum = 0.0;
                    for (int j = 0; j < m_n; ++j) {
                        if (j != k) {
                            sum += std::max(0.0, R[j][k]);
                        }
                    }
                    A[i][k] = damping * A[i][k] + (1.0 - damping) * sum;
                } else {
                    /* 非自可用性 */
                    double sum = 0.0;
                    for (int j = 0; j < m_n; ++j) {
                        if (j != i && j != k) {
                            sum += std::max(0.0, R[j][k]);
                        }
                    }
                    double newA = std::min(0.0, R[k][k] + sum);
                    A[i][k] = damping * A[i][k] + (1.0 - damping) * newA;
                }
            }
        }

        /* 提取exemplar: 对每个i选择argmax_k(R(i,k)+A(i,k)) */
        m_labels.resize(m_n);
        m_exemplars.clear();
        for (int i = 0; i < m_n; ++i) {
            int bestK = 0;
            double bestVal = R[i][0] + A[i][0];
            for (int k = 1; k < m_n; ++k) {
                double val = R[i][k] + A[i][k];
                if (val > bestVal) {
                    bestVal = val;
                    bestK = k;
                }
            }
            m_labels[i] = bestK;
        }

        /* 收集exemplar(被自己或他人选为代表的点) */
        QSet<int> exSet;
        for (int i = 0; i < m_n; ++i) {
            if (m_labels[i] == i) {
                exSet.insert(i);
            }
        }
        /* 如果没有exemplar,则选R(k,k)+A(k,k)最大的点 */
        if (exSet.isEmpty()) {
            int bestK = 0;
            double bestVal = R[0][0] + A[0][0];
            for (int k = 1; k < m_n; ++k) {
                double val = R[k][k] + A[k][k];
                if (val > bestVal) {
                    bestVal = val;
                    bestK = k;
                }
            }
            exSet.insert(bestK);
        }
        m_exemplars = exSet.values().toVector();
        std::sort(m_exemplars.begin(), m_exemplars.end());

        /* 映射标签到连续编号 */
        QMap<int, int> exMap;
        for (int idx = 0; idx < m_exemplars.size(); ++idx) {
            exMap[m_exemplars[idx]] = idx;
        }
        for (int i = 0; i < m_n; ++i) {
            m_labels[i] = exMap.value(m_labels[i], 0);
        }

        /* 计算残差: 标签变化计数 */
        int changes = 0;
        if (labelsPrev[0] >= 0) {
            for (int i = 0; i < m_n; ++i) {
                if (m_labels[i] != labelsPrev[i]) {
                    ++changes;
                }
            }
        }
        m_residual = static_cast<double>(changes) / m_n;

        /* 收敛检测 */
        if (changes == 0 && labelsPrev[0] >= 0) {
            ++convergenceCount;
            if (convergenceCount >= convergenceWindow) {
                break;
            }
        } else {
            convergenceCount = 0;
        }

        labelsPrev = m_labels;
        emit iterationCompleted(iter + 1, m_residual);
    }

    /* 更新统计信息 */
    m_stats.totalFits++;
    m_stats.totalIterations += m_iterations;
    m_stats.totalExemplars += m_exemplars.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(m_exemplars.size());
}

/**
 * @brief 重置统计信息
 */
void AffinityCluster::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
