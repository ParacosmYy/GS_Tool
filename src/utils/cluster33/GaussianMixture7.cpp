/**
 * @file GaussianMixture7.cpp
 * @brief GMM7 半监督高斯混合模型实现 — EM算法与约束标签传播
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现标准EM聚类与半监督约束EM聚类，支持必须链接/不可链接约束。
 */

#include "utils/cluster33/GaussianMixture7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
GaussianMixture7::GaussianMixture7(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("GaussianMixture7"));
}

/**
 * @brief 设置高斯分量数量
 * @param k 分量数，必须 >= 2
 */
void GaussianMixture7::setComponents(int k)
{
    m_k = qMax(2, k);
}

/**
 * @brief 设置最大EM迭代次数
 * @param maxIter 最大迭代数
 */
void GaussianMixture7::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(10, maxIter);
}

/**
 * @brief 设置半监督约束
 *
 * mustLink 表示两个数据点必须属于同一聚类，
 * cannotLink 表示两个数据点必须属于不同聚类。
 *
 * @param mustLink 必须链接的点对列表
 * @param cannotLink 不可链接的点对列表
 */
void GaussianMixture7::setConstraints(const QVector<QPair<int, int>> &mustLink,
                                       const QVector<QPair<int, int>> &cannotLink)
{
    m_mustLink = mustLink;
    m_cannotLink = cannotLink;
}

/**
 * @brief 计算多维高斯概率密度
 *
 * 使用对数形式避免数值下溢：log N(x|mu,Sigma)。
 * 对角协方差矩阵情况下简化计算。
 *
 * @param x 数据点
 * @param mean 均值向量
 * @param diagVar 对角方差向量
 * @return 概率密度值
 */
static double gaussianPdf(const QVector<double> &x,
                           const QVector<double> &mean,
                           const QVector<double> &diagVar)
{
    const int d = x.size();
    double logDet = 0.0;
    double mahal = 0.0;
    for (int i = 0; i < d; ++i) {
        const double v = qMax(diagVar[i], 1e-12);
        logDet += qLn(v);
        const double diff = x[i] - mean[i];
        mahal += diff * diff / v;
    }
    const double logPdf = -0.5 * (d * qLn(2.0 * M_PI) + logDet + mahal);
    return qExp(logPdf);
}

/**
 * @brief 重置统计信息
 */
void GaussianMixture7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 标准 EM 聚类拟合
 *
 * 步骤：
 * 1. K-Means++ 初始化均值
 * 2. E步：计算每个点属于每个分量的后验概率
 * 3. M步：根据后验重新估计均值、方差、权重
 * 4. 计算对数似然判断收敛
 *
 * @param data 输入数据，每个元素为一个多维样本
 * @return 每个样本的聚类标签
 */
QVector<int> GaussianMixture7::fit(const QVector<QVector<double>> &data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return {};

    const int d = data[0].size();
    const int k = qMin(m_k, n);

    // K-Means++ 初始化均值
    QVector<QVector<double>> means(k, QVector<double>(d, 0.0));
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, n - 1);
    means[0] = data[uni(rng)];

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n, 0.0);
        double distSum = 0.0;
        for (int i = 0; i < n; ++i) {
            double minDist = std::numeric_limits<double>::max();
            for (int j = 0; j < c; ++j) {
                double dist = 0.0;
                for (int dim = 0; dim < d; ++dim) {
                    const double diff = data[i][dim] - means[j][dim];
                    dist += diff * diff;
                }
                minDist = qMin(minDist, dist);
            }
            dists[i] = minDist;
            distSum += minDist;
        }
        std::uniform_real_distribution<double> rDist(0.0, distSum);
        double threshold = rDist(rng);
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= threshold) {
                means[c] = data[i];
                break;
            }
        }
    }

    // 初始化对角方差和权重
    QVector<QVector<double>> diagVars(k, QVector<double>(d, 1.0));
    QVector<double> weights(k, 1.0 / k);

    // 后验概率矩阵 (n x k)
    QVector<QVector<double>> resp(n, QVector<double>(k, 0.0));

    double prevLogLik = -std::numeric_limits<double>::max();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // E步：计算后验概率
        for (int i = 0; i < n; ++i) {
            double sumPdf = 0.0;
            for (int c = 0; c < k; ++c) {
                resp[i][c] = weights[c] * gaussianPdf(data[i], means[c], diagVars[c]);
                sumPdf += resp[i][c];
            }
            if (sumPdf > 1e-300) {
                for (int c = 0; c < k; ++c) {
                    resp[i][c] /= sumPdf;
                }
            }
        }

        // M步：更新参数
        for (int c = 0; c < k; ++c) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) {
                nk += resp[i][c];
            }
            nk = qMax(nk, 1e-12);

            // 更新权重
            weights[c] = nk / n;

            // 更新均值
            for (int dim = 0; dim < d; ++dim) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i) {
                    sum += resp[i][c] * data[i][dim];
                }
                means[c][dim] = sum / nk;
            }

            // 更新对角方差
            for (int dim = 0; dim < d; ++dim) {
                double sumVar = 0.0;
                for (int i = 0; i < n; ++i) {
                    const double diff = data[i][dim] - means[c][dim];
                    sumVar += resp[i][c] * diff * diff;
                }
                diagVars[c][dim] = qMax(sumVar / nk, 1e-12);
            }
        }

        // 计算对数似然
        double logLik = 0.0;
        for (int i = 0; i < n; ++i) {
            double sumPdf = 0.0;
            for (int c = 0; c < k; ++c) {
                sumPdf += weights[c] * gaussianPdf(data[i], means[c], diagVars[c]);
            }
            if (sumPdf > 1e-300) {
                logLik += qLn(sumPdf);
            }
        }

        // 收敛判断
        if (qAbs(logLik - prevLogLik) < 1e-6) {
            prevLogLik = logLik;
            break;
        }
        prevLogLik = logLik;
    }

    // 分配标签：取最大后验概率对应的分量
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        int bestC = 0;
        double bestResp = resp[i][0];
        for (int c = 1; c < k; ++c) {
            if (resp[i][c] > bestResp) {
                bestResp = resp[i][c];
                bestC = c;
            }
        }
        labels[i] = bestC;
    }

    // 更新统计信息
    m_stats.totalFits++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitComplete(k, prevLogLik);
    return labels;
}

/**
 * @brief 半监督 EM 拟合，结合部分标签和约束
 *
 * 在 E 步中强制已标注样本的后验概率为 one-hot，
 * 并通过约束传播更新未标注样本的后验。
 *
 * @param data 输入数据
 * @param partialLabels 部分标签，-1 表示未标注
 * @return 所有样本的聚类标签
 */
QVector<int> GaussianMixture7::fitSemiSupervised(const QVector<QVector<double>> &data,
                                                   const QVector<int> &partialLabels)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return {};

    const int d = data[0].size();
    const int k = qMin(m_k, n);

    // 复用标准EM进行初始拟合，获取参数
    QVector<QVector<double>> means(k, QVector<double>(d, 0.0));
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, n - 1);
    for (int c = 0; c < k; ++c) {
        means[c] = data[uni(rng)];
    }

    QVector<QVector<double>> diagVars(k, QVector<double>(d, 1.0));
    QVector<double> weights(k, 1.0 / k);
    QVector<QVector<double>> resp(n, QVector<double>(k, 0.0));

    // 迭代EM + 约束传播
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // E步：标准后验计算
        for (int i = 0; i < n; ++i) {
            double sumPdf = 0.0;
            for (int c = 0; c < k; ++c) {
                resp[i][c] = weights[c] * gaussianPdf(data[i], means[c], diagVars[c]);
                sumPdf += resp[i][c];
            }
            if (sumPdf > 1e-300) {
                for (int c = 0; c < k; ++c) {
                    resp[i][c] /= sumPdf;
                }
            }
        }

        // 强制已标注样本为 one-hot
        for (int i = 0; i < n && i < partialLabels.size(); ++i) {
            if (partialLabels[i] >= 0 && partialLabels[i] < k) {
                for (int c = 0; c < k; ++c) {
                    resp[i][c] = (c == partialLabels[i]) ? 1.0 : 0.0;
                }
            }
        }

        // Must-link 约束传播：强制两点的后验相同
        for (const auto &link : m_mustLink) {
            if (link.first < n && link.second < n) {
                for (int c = 0; c < k; ++c) {
                    const double avg = (resp[link.first][c] + resp[link.second][c]) / 2.0;
                    resp[link.first][c] = avg;
                    resp[link.second][c] = avg;
                }
            }
        }

        // Cannot-link 约束传播：限制两点不能同时属于同一分量
        for (const auto &link : m_cannotLink) {
            if (link.first < n && link.second < n) {
                for (int c = 0; c < k; ++c) {
                    if (resp[link.first][c] > 0.5 && resp[link.second][c] > 0.5) {
                        resp[link.second][c] *= 0.1;
                    }
                }
                // 重新归一化
                double sum = 0.0;
                for (int c = 0; c < k; ++c) sum += resp[link.second][c];
                if (sum > 1e-300) {
                    for (int c = 0; c < k; ++c) resp[link.second][c] /= sum;
                }
            }
        }

        // M步
        for (int c = 0; c < k; ++c) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) nk += resp[i][c];
            nk = qMax(nk, 1e-12);
            weights[c] = nk / n;

            for (int dim = 0; dim < d; ++dim) {
                double sum = 0.0;
                for (int i = 0; i < n; ++i) sum += resp[i][c] * data[i][dim];
                means[c][dim] = sum / nk;

                double sumVar = 0.0;
                for (int i = 0; i < n; ++i) {
                    const double diff = data[i][dim] - means[c][dim];
                    sumVar += resp[i][c] * diff * diff;
                }
                diagVars[c][dim] = qMax(sumVar / nk, 1e-12);
            }
        }
    }

    // 生成最终标签
    QVector<int> labels(n, 0);
    for (int i = 0; i < n; ++i) {
        int bestC = 0;
        double bestResp = resp[i][0];
        for (int c = 1; c < k; ++c) {
            if (resp[i][c] > bestResp) {
                bestResp = resp[i][c];
                bestC = c;
            }
        }
        labels[i] = bestC;
    }

    m_stats.totalFits++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    double logLik = 0.0;
    for (int i = 0; i < n; ++i) {
        double sumPdf = 0.0;
        for (int c = 0; c < k; ++c) {
            sumPdf += weights[c] * gaussianPdf(data[i], means[c], diagVars[c]);
        }
        if (sumPdf > 1e-300) logLik += qLn(sumPdf);
    }

    emit fitComplete(k, logLik);
    return labels;
}
