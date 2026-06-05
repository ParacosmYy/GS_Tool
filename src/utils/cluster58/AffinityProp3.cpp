/**
 * @file AffinityProp3.cpp
 * @brief 仿射传播聚类算法实现
 *
 * 实现基于消息传递的仿射传播(Affinity Propagation)聚类算法。
 * 通过在数据点之间传递"责任"(responsibility)和"可用性"(availability)
 * 消息来识别代表性点(exemplar)，自动确定聚类数量。
 * 支持阻尼因子防止振荡，以及收敛检测。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/cluster58/AffinityProp3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化仿射传播聚类器
 * @param parent 父QObject指针
 */
AffinityProp3::AffinityProp3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置阻尼因子
 * @param damp 阻尼因子，范围 [0.5, 1.0)，默认 0.5
 *
 * 较高的阻尼因子减少振荡但降低收敛速度
 */
void AffinityProp3::setDamping(double damp)
{
    m_damping = qBound(0.5, damp, 0.99);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数，默认 200
 */
void AffinityProp3::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

/**
 * @brief 设置收敛判定迭代数
 * @param convIter 连续多少次迭代exemplar不变即认为收敛，默认 10
 */
void AffinityProp3::setConvergence(int convIter)
{
    m_convIter = qMax(1, convIter);
}

/**
 * @brief 执行仿射传播聚类
 *
 * 算法流程:
 * 1. 初始化责任矩阵R和可用性矩阵A为零
 * 2. 迭代更新:
 *    a. R(i,k) = s(i,k) - max_{k'!=k} { A(i,k') + s(i,k') }
 *    b. A(i,k) = min(0, R(k,k) + sum_{i'!=i,k} max(0, R(i',k)))
 *    c. 应用阻尼: msg_new = damp * msg_old + (1-damp) * msg_new
 * 3. 当exemplar连续convIter次不变时判定收敛
 * 4. 根据最终R+A选择exemplar并分配聚类
 *
 * @param similarities 相似度矩阵，N*N，s(i,k)为点i与点k的相似度
 *                      对角线s(k,k)为点的"偏好度"，值越大越可能成为exemplar
 * @return 每个数据点的exemplar索引
 */
QVector<int> AffinityProp3::cluster(const QVector<QVector<double>>& similarities)
{
    QElapsedTimer timer;
    timer.start();

    const int N = similarities.size();
    QVector<int> labels(N, 0);
    m_exemplars.clear();

    if (N == 0) {
        emit clusteringCompleted(0, 0);
        return labels;
    }

    /* 初始化责任矩阵R和可用性矩阵A */
    QVector<QVector<double>> R(N, QVector<double>(N, 0.0));
    QVector<QVector<double>> A(N, QVector<double>(N, 0.0));

    /* 记录上一轮的exemplar用于收敛检测 */
    QVector<int> prevExemplars(N, -1);
    int convergenceCount = 0;
    int totalIter = 0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        totalIter = iter + 1;

        /* 步骤1: 更新责任矩阵 R(i,k) */
        for (int i = 0; i < N; ++i) {
            for (int k = 0; k < N; ++k) {
                /* 计算 max_{k'!=k} { A(i,k') + s(i,k') } */
                double maxVal = -std::numeric_limits<double>::max();
                double secondMax = -std::numeric_limits<double>::max();

                for (int kk = 0; kk < N; ++kk) {
                    if (kk == k) continue;
                    double val = A[i][kk] + similarities[i][kk];
                    if (val > maxVal) {
                        secondMax = maxVal;
                        maxVal = val;
                    } else if (val > secondMax) {
                        secondMax = val;
                    }
                }

                /* R(i,k) = s(i,k) - max_{k'!=k} { A(i,k') + s(i,k') } */
                double newR = similarities[i][k] - maxVal;

                /* 应用阻尼 */
                R[i][k] = m_damping * R[i][k] + (1.0 - m_damping) * newR;
            }
        }

        /* 步骤2: 更新可用性矩阵 A(i,k) */
        for (int i = 0; i < N; ++i) {
            for (int k = 0; k < N; ++k) {
                if (i == k) {
                    /* A(k,k) = sum_{i'!=k} max(0, R(i',k)) */
                    double sum = 0.0;
                    for (int ii = 0; ii < N; ++ii) {
                        if (ii != k) {
                            sum += qMax(0.0, R[ii][k]);
                        }
                    }
                    A[i][k] = m_damping * A[i][k] + (1.0 - m_damping) * sum;
                } else {
                    /* A(i,k) = min(0, R(k,k) + sum_{i'!=i,k} max(0, R(i',k))) */
                    double sum = R[k][k];
                    for (int ii = 0; ii < N; ++ii) {
                        if (ii != i && ii != k) {
                            sum += qMax(0.0, R[ii][k]);
                        }
                    }
                    double newA = qMin(0.0, sum);
                    A[i][k] = m_damping * A[i][k] + (1.0 - m_damping) * newA;
                }
            }
        }

        /* 步骤3: 检查收敛 - 找到当前exemplar */
        QVector<int> currentExemplars(N, 0);
        for (int i = 0; i < N; ++i) {
            double maxVal = R[i][0] + A[i][0];
            currentExemplars[i] = 0;
            for (int k = 1; k < N; ++k) {
                double val = R[i][k] + A[i][k];
                if (val > maxVal) {
                    maxVal = val;
                    currentExemplars[i] = k;
                }
            }
        }

        /* 比较与上一轮是否相同 */
        if (currentExemplars == prevExemplars) {
            convergenceCount++;
            if (convergenceCount >= m_convIter) {
                break;
            }
        } else {
            convergenceCount = 0;
        }
        prevExemplars = currentExemplars;
    }

    /* 步骤4: 确定最终exemplar和标签 */
    for (int i = 0; i < N; ++i) {
        double maxVal = R[i][0] + A[i][0];
        labels[i] = 0;
        for (int k = 1; k < N; ++k) {
            double val = R[i][k] + A[i][k];
            if (val > maxVal) {
                maxVal = val;
                labels[i] = k;
            }
        }
    }

    /* 提取唯一的exemplar */
    QVector<int> uniqueExemplars;
    for (int ex : labels) {
        if (!uniqueExemplars.contains(ex)) {
            uniqueExemplars.append(ex);
        }
    }
    m_exemplars = uniqueExemplars;

    /* 重新编号标签为连续的0~K-1 */
    for (int i = 0; i < N; ++i) {
        int idx = m_exemplars.indexOf(labels[i]);
        labels[i] = (idx >= 0) ? idx : 0;
    }

    /* 更新统计 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += N;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(m_exemplars.size(), totalIter);
    return labels;
}

/**
 * @brief 重置所有统计数据
 */
void AffinityProp3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_exemplars.clear();
}
