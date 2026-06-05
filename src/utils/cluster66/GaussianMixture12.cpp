/**
 * @file GaussianMixture12.cpp
 * @brief 高斯混合模型(GMM)聚类算法实现
 *
 * 实现基于期望最大化(EM)算法的高斯混合模型，支持多分量拟合、
 * 预测、BIC/AIC模型选择。用于数据聚类和概率密度估计。
 */

#include "utils/cluster66/GaussianMixture12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
GaussianMixture12::GaussianMixture12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置高斯分量数量
 * @param k 分量数量，必须大于0
 */
void GaussianMixture12::setNumComponents(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置EM算法最大迭代次数
 * @param iter 最大迭代次数
 */
void GaussianMixture12::setMaxIterations(int iter)
{
    m_maxIter = qMax(10, iter);
}

/**
 * @brief 设置协方差类型
 * @param type 协方差类型："full"（全协方差）或 "diag"（对角协方差）
 */
void GaussianMixture12::setCovarianceType(const QString& type)
{
    if (type == "full" || type == "diag") {
        m_covType = type;
    }
}

/**
 * @brief 对数据进行GMM拟合（EM算法）
 * @param data 输入数据，每行一个样本，每列一个特征
 * @return 拟合是否成功
 */
bool GaussianMixture12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_k || data.isEmpty()) {
        return false;
    }

    const int N = data.size();
    const int D = data[0].size();

    // 初始化均值：使用K-Means++策略选取初始中心
    std::mt19937 rng(42);
    m_means.clear();
    m_weights.clear();
    m_weights.resize(m_k, 1.0 / m_k);

    // 选取第一个中心
    std::uniform_int_distribution<int> uni(0, N - 1);
    m_means.append(data[uni(rng)]);

    // K-Means++选取后续中心
    QVector<double> minDist(N, 1e18);
    for (int c = 1; c < m_k; ++c) {
        double totalDist = 0.0;
        for (int i = 0; i < N; ++i) {
            double d = 0.0;
            for (int dd = 0; dd < D; ++dd) {
                double diff = data[i][dd] - m_means[c - 1][dd];
                d += diff * diff;
            }
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }
        // 轮盘赌选择下一个中心
        std::uniform_real_distribution<double> prob(0, totalDist);
        double r = prob(rng);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < N; ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) { chosen = i; break; }
        }
        m_means.append(data[chosen]);
    }

    // 确保均值维度一致
    for (auto& mu : m_means) {
        if (mu.size() != D) mu.resize(D, 0.0);
    }

    // 初始化协方差矩阵为单位矩阵的倍数
    QVector<QVector<QVector<double>>> covariances(m_k);
    for (int c = 0; c < m_k; ++c) {
        covariances[c].resize(D);
        for (int d = 0; d < D; ++d) {
            covariances[c][d].resize(D, 0.0);
            covariances[c][d][d] = 1.0;
        }
    }

    // EM迭代
    QVector<QVector<double>> resp(N, QVector<double>(m_k, 0.0));
    double prevLLH = -1e18;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // E步：计算每个样本对每个分量的响应度
        for (int i = 0; i < N; ++i) {
            double sumResp = 0.0;
            for (int c = 0; c < m_k; ++c) {
                // 计算多元高斯概率密度
                double det = 1.0;
                QVector<double> diff(D);
                for (int d = 0; d < D; ++d) {
                    diff[d] = data[i][d] - m_means[c][d];
                    det *= (m_covType == "diag") ? covariances[c][d][d] : 1.0;
                }
                if (m_covType == "full") {
                    det = 1.0; // 简化：使用对角近似
                    for (int d = 0; d < D; ++d) det *= covariances[c][d][d];
                }
                det = qMax(det, 1e-300);

                double mahal = 0.0;
                if (m_covType == "diag") {
                    for (int d = 0; d < D; ++d) {
                        double v = diff[d] / qMax(covariances[c][d][d], 1e-300);
                        mahal += v * v;
                    }
                } else {
                    for (int d = 0; d < D; ++d) {
                        mahal += diff[d] * diff[d] / qMax(covariances[c][d][d], 1e-300);
                    }
                }

                double logProb = -0.5 * D * qLn(2.0 * M_PI) - 0.5 * qLn(det) - 0.5 * mahal;
                resp[i][c] = m_weights[c] * qExp(logProb);
                sumResp += resp[i][c];
            }
            if (sumResp > 0.0) {
                for (int c = 0; c < m_k; ++c) resp[i][c] /= sumResp;
            }
        }

        // M步：更新参数
        for (int c = 0; c < m_k; ++c) {
            double Nc = 0.0;
            for (int i = 0; i < N; ++i) Nc += resp[i][c];
            Nc = qMax(Nc, 1e-300);

            // 更新权重
            m_weights[c] = Nc / N;

            // 更新均值
            for (int d = 0; d < D; ++d) {
                double sum = 0.0;
                for (int i = 0; i < N; ++i) sum += resp[i][c] * data[i][d];
                m_means[c][d] = sum / Nc;
            }

            // 更新协方差
            for (int d = 0; d < D; ++d) {
                double var = 0.0;
                for (int i = 0; i < N; ++i) {
                    double diff = data[i][d] - m_means[c][d];
                    var += resp[i][c] * diff * diff;
                }
                covariances[c][d][d] = qMax(var / Nc, 1e-6);
            }
        }

        // 计算对数似然
        double llh = 0.0;
        for (int i = 0; i < N; ++i) {
            double sumP = 0.0;
            for (int c = 0; c < m_k; ++c) {
                double det = 1.0;
                for (int d = 0; d < D; ++d) det *= covariances[c][d][d];
                double mahal = 0.0;
                for (int d = 0; d < D; ++d) {
                    double diff = data[i][d] - m_means[c][d];
                    mahal += diff * diff / qMax(covariances[c][d][d], 1e-300);
                }
                double logP = -0.5 * D * qLn(2.0 * M_PI) - 0.5 * qLn(qMax(det, 1e-300)) - 0.5 * mahal;
                sumP += m_weights[c] * qExp(logP);
            }
            llh += qLn(qMax(sumP, 1e-300));
        }

        // 收敛检测
        if (qAbs(llh - prevLLH) < 1e-6) break;
        prevLLH = llh;

        // 计算BIC和AIC
        int numParams = m_k * (D + D + 1) - 1; // 均值+方差+权重
        m_bic = -2.0 * llh + numParams * qLn(N);
        m_aic = -2.0 * llh + 2.0 * numParams;
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalFits++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(m_k, prevLLH);
    return true;
}

/**
 * @brief 预测每个样本所属的聚类标签
 * @param data 输入数据
 * @return 聚类标签向量
 */
QVector<int> GaussianMixture12::predict(const QVector<QVector<double>>& data)
{
    QVector<int> labels;
    if (m_means.isEmpty()) return labels;

    const int D = m_means[0].size();
    labels.reserve(data.size());

    for (const auto& sample : data) {
        int bestC = 0;
        double bestResp = -1e18;
        for (int c = 0; c < m_k; ++c) {
            double mahal = 0.0;
            for (int d = 0; d < D && d < sample.size(); ++d) {
                double diff = sample[d] - m_means[c][d];
                mahal += diff * diff;
            }
            double score = qLn(qMax(m_weights[c], 1e-300)) - 0.5 * mahal;
            if (score > bestResp) {
                bestResp = score;
                bestC = c;
            }
        }
        labels.append(bestC);
    }
    return labels;
}

/**
 * @brief 重置统计信息
 */
void GaussianMixture12::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
