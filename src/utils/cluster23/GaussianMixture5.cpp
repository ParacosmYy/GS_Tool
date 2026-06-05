/**
 * @file GaussianMixture5.cpp
 * @brief 变分贝叶斯高斯混合模型实现
 *
 * 使用变分推断自动确定高斯混合分量数。
 * 通过Dirichlet先验约束权重分布, E步计算责任度,
 * M步更新均值/协方差/权重, 当权重低于阈值时自动修剪分量。
 */

#include "utils/cluster23/GaussianMixture5.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <numeric>

/**
 * @brief 构造函数
 * @param maxComponents 最大分量数(默认10)
 * @param parent 父对象
 */
GaussianMixture5::GaussianMixture5(int maxComponents, QObject* parent)
    : QObject(parent)
    , m_maxComponents(maxComponents)
{
}

/**
 * @brief 拟合数据 — 执行变分贝叶斯GMM推理
 *
 * 算法流程:
 * 1. K-means++初始化均值
 * 2. E步: 计算每个样本对各分量的责任度(后验概率)
 * 3. M步: 根据责任度更新均值、协方差、Dirichlet参数
 * 4. 计算变分下界(ELBO), 检查收敛
 * 5. 修剪权重低于阈值的分量
 *
 * @param data 输入数据, N×D行优先展开的一维数组
 * @param dims 特征维度D
 * @param maxIter 最大迭代次数(默认200)
 * @return 拟合结果, 包含权重/均值/协方差/下界历史/活跃分量数
 */
GaussianMixture5::FitResult GaussianMixture5::fit(const QVector<double>& data, int dims, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    m_dims = dims;
    const int N = data.size() / dims;
    const int K = m_maxComponents;

    if (N < K || N < 2) {
        return m_result;
    }

    FitResult result;
    result.weights.resize(K, 1.0 / K);
    result.means.resize(K, QVector<double>(dims, 0.0));
    result.covariances.resize(K, QVector<QVector<double>>(dims, QVector<double>(dims, 0.0)));

    /* K-means++初始化: 依次选取距已有中心最远的点作为新中心 */
    QVector<int> assignment(N, 0);
    QVector<double> minDist(N, 1e18);
    int firstIdx = 0;
    for (int d = 0; d < dims; ++d) {
        result.means[0][d] = data[firstIdx * dims + d];
    }
    for (int k = 1; k < K; ++k) {
        double totalDist = 0.0;
        for (int i = 0; i < N; ++i) {
            double dist = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = data[i * dims + d] - result.means[k - 1][d];
                dist += diff * diff;
            }
            if (dist < minDist[i]) {
                minDist[i] = dist;
            }
            totalDist += minDist[i];
        }
        double threshold = totalDist * (static_cast<double>(qrand()) / RAND_MAX);
        double cumSum = 0.0;
        int chosen = N - 1;
        for (int i = 0; i < N; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) {
                chosen = i;
                break;
            }
        }
        for (int d = 0; d < dims; ++d) {
            result.means[k][d] = data[chosen * dims + d];
        }
    }

    /* 初始化协方差为单位矩阵×0.01, Dirichlet参数 */
    double dirichletAlpha = 1.0 / K;
    for (int k = 0; k < K; ++k) {
        for (int d = 0; d < dims; ++d) {
            result.covariances[k][d][d] = 0.01;
        }
    }

    /* 责任度矩阵 N×K */
    QVector<QVector<double>> resp(N, QVector<double>(K, 0.0));

    double prevLB = -1e18;
    const double weightThreshold = 1e-3;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* === E步: 计算责任度 === */
        for (int i = 0; i < N; ++i) {
            double sumResp = 0.0;
            for (int k = 0; k < K; ++k) {
                double logProb = 0.0;
                /* 计算log N(x_i | mu_k, Sigma_k) */
                for (int d = 0; d < dims; ++d) {
                    double diff = data[i * dims + d] - result.means[k][d];
                    logProb += diff * diff / (result.covariances[k][d][d] + 1e-10);
                }
                logProb *= -0.5;
                /* log det(Sigma) 简化: 仅对角项 */
                double logDet = 0.0;
                for (int d = 0; d < dims; ++d) {
                    logDet += qLn(qFabs(result.covariances[k][d][d]) + 1e-10);
                }
                logProb -= 0.5 * (dims * qLn(2.0 * M_PI) + logDet);
                /* 加上log权重 + Dirichlet先验贡献 */
                double alphaK = result.weights[k] * N + dirichletAlpha;
                double alphaSum = N + K * dirichletAlpha;
                resp[i][k] = qExp(logProb) * (alphaK / alphaSum);
                sumResp += resp[i][k];
            }
            if (sumResp > 0.0) {
                for (int k = 0; k < K; ++k) {
                    resp[i][k] /= sumResp;
                }
            }
        }

        /* === M步: 更新参数 === */
        for (int k = 0; k < K; ++k) {
            double Nk = 0.0;
            for (int i = 0; i < N; ++i) {
                Nk += resp[i][k];
            }

            if (Nk < 1e-10) {
                /* 死分量: 重置到随机数据点 */
                int idx = qrand() % N;
                for (int d = 0; d < dims; ++d) {
                    result.means[k][d] = data[idx * dims + d];
                }
                result.weights[k] = 0.0;
                continue;
            }

            /* 更新权重 */
            result.weights[k] = (Nk + dirichletAlpha) / (N + K * dirichletAlpha);

            /* 更新均值 */
            for (int d = 0; d < dims; ++d) {
                double sum = 0.0;
                for (int i = 0; i < N; ++i) {
                    sum += resp[i][k] * data[i * dims + d];
                }
                result.means[k][d] = sum / Nk;
            }

            /* 更新协方差(简化为对角) */
            for (int d = 0; d < dims; ++d) {
                double var = 0.0;
                for (int i = 0; i < N; ++i) {
                    double diff = data[i * dims + d] - result.means[k][d];
                    var += resp[i][k] * diff * diff;
                }
                result.covariances[k][d][d] = var / Nk + 1e-6;
            }
        }

        /* === 计算变分下界(简化ELBO) === */
        double lb = 0.0;
        for (int i = 0; i < N; ++i) {
            for (int k = 0; k < K; ++k) {
                if (resp[i][k] > 1e-30) {
                    lb += resp[i][k] * qLn(result.weights[k] + 1e-30);
                    double quad = 0.0;
                    for (int d = 0; d < dims; ++d) {
                        double diff = data[i * dims + d] - result.means[k][d];
                        quad += diff * diff / (result.covariances[k][d][d] + 1e-10);
                    }
                    lb -= 0.5 * resp[i][k] * quad;
                    lb -= resp[i][k] * qLn(resp[i][k]);
                }
            }
        }
        result.lowerBounds.push_back(lb);

        /* 检查收敛: ELBO变化小于阈值 */
        if (iter > 0 && qFabs(lb - prevLB) < 1e-6 * qFabs(prevLB + 1.0)) {
            result.iterations = iter + 1;
            break;
        }
        prevLB = lb;
        result.iterations = iter + 1;

        emit fitProgress(iter, lb, K);
    }

    /* === 修剪低权重分量 === */
    int active = 0;
    for (int k = 0; k < K; ++k) {
        if (result.weights[k] >= weightThreshold) {
            ++active;
        }
    }
    result.activeComponents = active;

    m_result = result;

    /* 更新统计信息 */
    m_stats.totalFits++;
    m_stats.totalIterations += result.iterations;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(active);
    return result;
}

/**
 * @brief 预测每个样本的后验分量概率
 *
 * 基于已拟合的模型参数, 对每个输入样本计算其属于各分量的后验概率。
 * 返回N×K矩阵, 每行元素之和为1。
 *
 * @param data 输入数据, N×D行优先
 * @param dims 特征维度
 * @return N×K责任度矩阵
 */
QVector<QVector<double>> GaussianMixture5::predict(const QVector<double>& data, int dims) const
{
    const int N = data.size() / dims;
    const int K = m_result.weights.size();
    QVector<QVector<double>> resp(N, QVector<double>(K, 0.0));

    for (int i = 0; i < N; ++i) {
        double sumResp = 0.0;
        for (int k = 0; k < K; ++k) {
            if (m_result.weights[k] < 1e-3) {
                continue;
            }
            double logProb = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = data[i * dims + d] - m_result.means[k][d];
                logProb += diff * diff / (m_result.covariances[k][d][d] + 1e-10);
            }
            logProb *= -0.5;
            double logDet = 0.0;
            for (int d = 0; d < dims; ++d) {
                logDet += qLn(qFabs(m_result.covariances[k][d][d]) + 1e-10);
            }
            logProb -= 0.5 * (dims * qLn(2.0 * M_PI) + logDet);
            resp[i][k] = qExp(logProb) * m_result.weights[k];
            sumResp += resp[i][k];
        }
        if (sumResp > 0.0) {
            for (int k = 0; k < K; ++k) {
                resp[i][k] /= sumResp;
            }
        }
    }

    return resp;
}

/**
 * @brief 重置统计计数器
 * 将总拟合次数、总迭代数和平均处理时间归零
 */
void GaussianMixture5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
