/**
 * @file GaussianMixture11.cpp
 * @brief 高斯混合模型实现 — EM算法 + 高斯PDF + 对数似然
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现高斯混合模型（GMM）的 EM（期望最大化）算法。
 * 通过迭代更新各分量的均值、协方差和混合权重，
 * 最大化数据的对数似然，实现概率密度估计和软聚类。
 */

#include "utils/cluster56/GaussianMixture11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 GMM 参数
 * @param parent 父QObject对象
 */
GaussianMixture11::GaussianMixture11(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("GaussianMixture11"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置高斯分量数量
 * @param k 分量数，必须 >= 1
 */
void GaussianMixture11::setNumComponents(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置最大 EM 迭代次数
 * @param iter 最大迭代次数
 */
void GaussianMixture11::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

/**
 * @brief 设置收敛阈值
 *
 * 当对数似然变化小于此阈值时停止迭代。
 *
 * @param tol 收敛阈值，必须 > 0
 */
void GaussianMixture11::setConvergence(double tol)
{
    m_tol = qMax(1e-10, tol);
}

// ──────────────────────────────────────────────
// EM 拟合
// ──────────────────────────────────────────────

/**
 * @brief 使用 EM 算法拟合高斯混合模型
 *
 * EM 算法步骤：
 * 1. 初始化：使用 K-Means++ 或随机方式初始化均值、协方差和权重
 * 2. E步（期望）：计算每个数据点属于每个分量的后验概率（责任）
 * 3. M步（最大化）：根据责任更新均值、协方差和权重
 * 4. 计算对数似然，检查收敛
 *
 * @param data 输入数据点集合
 * @return true 拟合成功
 */
bool GaussianMixture11::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return false;
    const int dim = data[0].size();
    if (dim == 0) return false;

    // 初始化参数
    m_means.resize(m_k, QVector<double>(dim, 0.0));
    m_covs.resize(m_k, QVector<QVector<double>>(dim, QVector<double>(dim, 0.0)));
    m_weights.resize(m_k, 1.0 / m_k);

    // 初始化均值：使用等间距采样
    for (int k = 0; k < m_k; ++k) {
        int idx = k * n / m_k;
        m_means[k] = data[idx];
    }

    // 初始化协方差：单位矩阵 * 数据方差
    double totalVar = 0.0;
    for (int d = 0; d < dim; ++d) {
        double mean = 0.0;
        for (int i = 0; i < n; ++i) mean += data[i][d];
        mean /= n;
        double var = 0.0;
        for (int i = 0; i < n; ++i) {
            var += (data[i][d] - mean) * (data[i][d] - mean);
        }
        totalVar += var / n;
    }
    totalVar /= dim;

    for (int k = 0; k < m_k; ++k) {
        for (int d = 0; d < dim; ++d) {
            m_covs[k][d][d] = totalVar + 1e-6; // 防止奇异
        }
    }

    m_logLik.clear();

    // EM 迭代
    for (int iter = 0; iter < m_maxIter; ++iter) {
        // E步：计算责任
        QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));

        for (int i = 0; i < n; ++i) {
            double total = 0.0;
            for (int k = 0; k < m_k; ++k) {
                resp[i][k] = m_weights[k] * gaussianPdf(data[i], m_means[k], m_covs[k]);
                total += resp[i][k];
            }
            if (total > 1e-300) {
                for (int k = 0; k < m_k; ++k) {
                    resp[i][k] /= total;
                }
            }
        }

        // M步：更新参数
        QVector<double> Nk(m_k, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int k = 0; k < m_k; ++k) {
                Nk[k] += resp[i][k];
            }
        }

        // 更新均值
        for (int k = 0; k < m_k; ++k) {
            if (Nk[k] < 1e-10) continue;
            for (int d = 0; d < dim; ++d) {
                m_means[k][d] = 0.0;
                for (int i = 0; i < n; ++i) {
                    m_means[k][d] += resp[i][k] * data[i][d];
                }
                m_means[k][d] /= Nk[k];
            }
        }

        // 更新协方差
        for (int k = 0; k < m_k; ++k) {
            if (Nk[k] < 1e-10) continue;
            for (int d1 = 0; d1 < dim; ++d1) {
                for (int d2 = 0; d2 < dim; ++d2) {
                    double cov = 0.0;
                    for (int i = 0; i < n; ++i) {
                        cov += resp[i][k] * (data[i][d1] - m_means[k][d1])
                                           * (data[i][d2] - m_means[k][d2]);
                    }
                    m_covs[k][d1][d2] = cov / Nk[k] + 1e-6 * (d1 == d2 ? 1.0 : 0.0);
                }
            }
        }

        // 更新权重
        for (int k = 0; k < m_k; ++k) {
            m_weights[k] = Nk[k] / n;
        }

        // 计算对数似然
        double llh = 0.0;
        for (int i = 0; i < n; ++i) {
            double prob = 0.0;
            for (int k = 0; k < m_k; ++k) {
                prob += m_weights[k] * gaussianPdf(data[i], m_means[k], m_covs[k]);
            }
            llh += qLn(qMax(prob, 1e-300));
        }
        m_logLik.append(llh);

        // 检查收敛
        if (m_logLik.size() > 1) {
            double diff = qAbs(m_logLik.last() - m_logLik[m_logLik.size() - 2]);
            if (diff < m_tol) break;
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalFits++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    double finalLlh = m_logLik.isEmpty() ? 0.0 : m_logLik.last();
    emit fitCompleted(m_k, finalLlh);
    return true;
}

// ──────────────────────────────────────────────
// 预测
// ──────────────────────────────────────────────

/**
 * @brief 预测每个数据点最可能属于的分量
 *
 * @param data 输入数据点集合
 * @return 聚类标签数组（0-based）
 */
QVector<int> GaussianMixture11::predict(const QVector<QVector<double>>& data)
{
    const int n = data.size();
    QVector<int> labels(n, 0);

    for (int i = 0; i < n; ++i) {
        double maxResp = -1.0;
        int bestK = 0;
        for (int k = 0; k < m_k; ++k) {
            double resp = m_weights[k] * gaussianPdf(data[i], m_means[k], m_covs[k]);
            if (resp > maxResp) {
                maxResp = resp;
                bestK = k;
            }
        }
        labels[i] = bestK;
    }

    return labels;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含拟合次数、总采样数和平均耗时的Stats结构
 */
GaussianMixture11::Stats GaussianMixture11::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void GaussianMixture11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 高斯 PDF
// ──────────────────────────────────────────────

/**
 * @brief 计算多维高斯概率密度函数值
 *
 * p(x) = (2*pi)^(-d/2) * |cov|^(-1/2) * exp(-0.5 * (x-mu)^T * cov^(-1) * (x-mu))
 *
 * 简化实现：假定为对角协方差矩阵。
 *
 * @param x 数据点
 * @param mu 均值向量
 * @param cov 协方差矩阵
 * @return 概率密度值
 */
double GaussianMixture11::gaussianPdf(const QVector<double>& x,
                                        const QVector<double>& mu,
                                        const QVector<QVector<double>>& cov)
{
    const int dim = qMin(x.size(), mu.size());
    if (dim == 0) return 0.0;

    // 计算对角协方差情况的 Mahalanobis 距离
    double mahal = 0.0;
    double logDet = 0.0;

    for (int d = 0; d < dim; ++d) {
        double diff = x[d] - mu[d];
        double var = (d < cov.size() && d < cov[d].size()) ? cov[d][d] : 1.0;
        if (var < 1e-10) var = 1e-10;
        mahal += diff * diff / var;
        logDet += qLn(var);
    }

    // 对数 PDF
    double logPdf = -0.5 * dim * qLn(2.0 * M_PI) - 0.5 * logDet - 0.5 * mahal;

    // 防止下溢
    if (logPdf < -700.0) return 1e-300;

    return qExp(logPdf);
}
