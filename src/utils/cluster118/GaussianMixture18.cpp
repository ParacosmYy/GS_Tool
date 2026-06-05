#include "GaussianMixture18.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- 内部GMM参数存储 ---- */
static QVector<double> g_weights;           /* 混合权重 */
static QVector<QVector<double>> g_means;    /* 各分量均值 */
static QVector<QVector<QVector<double>>> g_covs; /* 各分量协方差矩阵 */
static int g_dim = 0;                       /* 数据维度 */
static int g_numComp = 0;                   /* 分量数 */

/**
 * @brief 计算多元高斯概率密度（内部辅助）
 *
 * 使用对角近似计算 N(x | μ, Σ)，避免完整矩阵求逆：
 * 对角元素取自协方差矩阵主对角线。
 *
 * @param x 数据点
 * @param mean 均值向量
 * @param cov 协方差矩阵
 * @return 概率密度值
 */
static double computeGaussian(const QVector<double>& x,
                               const QVector<double>& mean,
                               const QVector<QVector<double>>& cov)
{
    const int dim = qMin(x.size(), qMin(mean.size(), cov.size()));
    if (dim == 0) return 0.0;

    /* 对角近似：仅使用主对角线 */
    double logDet = 0.0;
    double mahal = 0.0;
    for (int d = 0; d < dim; ++d) {
        double var = (d < cov[d].size()) ? qMax(cov[d][d], 1e-8) : 1.0;
        logDet += qLn(var);
        double diff = (d < x.size()) ? x[d] - mean[d] : 0.0;
        mahal += diff * diff / var;
    }

    double logProb = -0.5 * (dim * qLn(2.0 * M_PI) + logDet + mahal);
    return qExp(logProb);
}

/**
 * @brief 构造函数，初始化高斯混合模型引擎
 * @param parent 父对象指针
 */
GaussianMixture18::GaussianMixture18(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GaussianMixture18::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    g_weights.clear();
    g_means.clear();
    g_covs.clear();
    g_dim = 0;
    g_numComp = 0;
}

/**
 * @brief 使用EM算法拟合GMM参数
 *
 * EM算法迭代流程：
 * E步：计算每个数据点对各分量的后验概率（软分配）
 * M步：根据后验概率更新均值、协方差和混合权重
 * 重复直到收敛或达到最大迭代次数。
 *
 * @param dataPoints 输入数据点集合
 * @param numComponents 高斯分量数
 * @param maxIterations 最大迭代次数
 * @return 各数据点的软分配概率
 */
QVector<QVector<double>> GaussianMixture18::fit(
    const QVector<QVector<double>>& dataPoints, int numComponents, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    const int K = qMax(1, numComponents);
    QVector<QVector<double>> responsibilities(n, QVector<double>(K, 1.0 / K));

    if (n == 0 || n < K) {
        emit fittingCompleted(0);
        return responsibilities;
    }

    g_dim = dataPoints[0].size();
    g_numComp = K;

    /* 初始化：均匀权重，随机选择均值，单位协方差 */
    g_weights = QVector<double>(K, 1.0 / K);
    g_means.clear();
    g_covs.clear();

    /* 使用均匀间隔选择初始均值 */
    for (int k = 0; k < K; ++k) {
        int idx = (n * k) / K;
        g_means.append(dataPoints[qMin(idx, n - 1)]);
        /* 初始化为单位矩阵 * 缩放因子 */
        QVector<QVector<double>> cov(g_dim, QVector<double>(g_dim, 0.0));
        for (int d = 0; d < g_dim; ++d) cov[d][d] = 1.0;
        g_covs.append(cov);
    }

    double prevLogLik = -1e18;
    int iterCount = 0;

    for (int iter = 0; iter < maxIterations; ++iter) {
        iterCount = iter + 1;

        /* === E步：计算后验概率 === */
        for (int i = 0; i < n; ++i) {
            double sumResp = 0.0;
            for (int k = 0; k < K; ++k) {
                responsibilities[i][k] = g_weights[k]
                    * computeGaussian(dataPoints[i], g_means[k], g_covs[k]);
                sumResp += responsibilities[i][k];
            }
            /* 归一化 */
            if (sumResp > 1e-300) {
                for (int k = 0; k < K; ++k) {
                    responsibilities[i][k] /= sumResp;
                }
            }
        }

        /* === M步：更新参数 === */
        for (int k = 0; k < K; ++k) {
            double Nk = 0.0;
            for (int i = 0; i < n; ++i) Nk += responsibilities[i][k];

            if (Nk < 1e-300) continue;

            /* 更新权重 */
            g_weights[k] = Nk / n;

            /* 更新均值 */
            QVector<double> newMean(g_dim, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int d = 0; d < g_dim && d < dataPoints[i].size(); ++d) {
                    newMean[d] += responsibilities[i][k] * dataPoints[i][d];
                }
            }
            for (int d = 0; d < g_dim; ++d) newMean[d] /= Nk;
            g_means[k] = newMean;

            /* 更新协方差 */
            QVector<QVector<double>> newCov(g_dim, QVector<double>(g_dim, 0.0));
            for (int i = 0; i < n; ++i) {
                const int dMax = qMin(g_dim, dataPoints[i].size());
                for (int d1 = 0; d1 < dMax; ++d1) {
                    double diff1 = dataPoints[i][d1] - newMean[d1];
                    for (int d2 = 0; d2 < dMax; ++d2) {
                        double diff2 = dataPoints[i][d2] - newMean[d2];
                        newCov[d1][d2] += responsibilities[i][k] * diff1 * diff2;
                    }
                }
            }
            for (int d1 = 0; d1 < g_dim; ++d1) {
                for (int d2 = 0; d2 < g_dim; ++d2) {
                    newCov[d1][d2] /= Nk;
                }
                /* 正则化：防止奇异矩阵 */
                newCov[d1][d1] += 1e-6;
            }
            g_covs[k] = newCov;
        }

        /* 计算对数似然，判断收敛 */
        double logLik = 0.0;
        for (int i = 0; i < n; ++i) {
            double pTotal = 0.0;
            for (int k = 0; k < K; ++k) {
                pTotal += g_weights[k]
                    * computeGaussian(dataPoints[i], g_means[k], g_covs[k]);
            }
            if (pTotal > 1e-300) logLik += qLn(pTotal);
        }

        if (qAbs(logLik - prevLogLik) < 1e-6) break;
        prevLogLik = logLik;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFitOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitOps;

    emit fittingCompleted(iterCount);
    return responsibilities;
}

/**
 * @brief 计算给定数据点的概率密度
 *
 * 使用混合高斯模型计算联合概率密度：
 * p(x) = Σ_k w_k * N(x | μ_k, Σ_k)
 *
 * @param point 数据点
 * @return 概率密度值
 */
double GaussianMixture18::probabilityDensity(const QVector<double>& point)
{
    if (g_numComp == 0 || g_means.isEmpty()) return 0.0;

    double density = 0.0;
    for (int k = 0; k < g_numComp; ++k) {
        density += g_weights[k] * computeGaussian(point, g_means[k], g_covs[k]);
    }
    return density;
}

/**
 * @brief 预测数据点最可能属于的分量
 *
 * 对每个数据点计算各分量的后验概率，
 * 返回后验概率最大的分量索引作为标签。
 *
 * @param dataPoints 数据点集合
 * @return 各点的分量标签
 */
QVector<int> GaussianMixture18::predict(const QVector<QVector<double>>& dataPoints)
{
    const int n = dataPoints.size();
    QVector<int> labels(n, 0);

    for (int i = 0; i < n; ++i) {
        double bestProb = -1.0;
        int bestK = 0;
        for (int k = 0; k < g_numComp; ++k) {
            double prob = g_weights[k] * computeGaussian(dataPoints[i], g_means[k], g_covs[k]);
            if (prob > bestProb) {
                bestProb = prob;
                bestK = k;
            }
        }
        labels[i] = bestK;
    }
    return labels;
}

/**
 * @brief 使用BIC准则选择最优分量数
 *
 * 对每个候选分量数拟合GMM，计算BIC值：
 * BIC = -2 * ln(L) + p * ln(n)
 * 其中p为自由参数数，n为样本数。
 * 选择BIC最小的分量数。
 *
 * @param dataPoints 数据点集合
 * @param maxComponents 最大候选分量数
 * @return 最优分量数
 */
int GaussianMixture18::selectComponentsBIC(
    const QVector<QVector<double>>& dataPoints, int maxComponents)
{
    const int n = dataPoints.size();
    if (n == 0) return 1;
    maxComponents = qMin(maxComponents, n);
    if (maxComponents <= 1) return 1;

    const int dim = dataPoints[0].size();
    double bestBIC = 1e18;
    int bestK = 1;

    for (int K = 1; K <= maxComponents; ++K) {
        /* 拟合K个分量 */
        auto resps = fit(dataPoints, K, 50);

        /* 计算对数似然 */
        double logLik = 0.0;
        for (int i = 0; i < n; ++i) {
            double pTotal = 0.0;
            for (int k = 0; k < K; ++k) {
                pTotal += g_weights[k]
                    * computeGaussian(dataPoints[i], g_means[k], g_covs[k]);
            }
            if (pTotal > 1e-300) logLik += qLn(pTotal);
        }

        /* 计算自由参数数：权重(K-1) + 均值(K*dim) + 协方差(K*dim*(dim+1)/2) */
        int numParams = (K - 1) + K * dim + K * dim * (dim + 1) / 2;
        double bic = -2.0 * logLik + numParams * qLn(n);

        if (bic < bestBIC) {
            bestBIC = bic;
            bestK = K;
        }
    }

    return bestK;
}

/**
 * @brief 在线更新GMM参数（单点增量学习）
 *
 * 使用学习率对新数据点进行增量参数更新：
 * 1. 计算新点对各分量的后验概率
 * 2. 以学习率混合更新权重、均值和协方差
 *
 * @param newPoint 新数据点
 * @param learningRate 学习率
 */
void GaussianMixture18::onlineUpdate(const QVector<double>& newPoint, double learningRate)
{
    if (g_numComp == 0 || g_means.isEmpty()) return;
    const int dim = qMin(g_dim, newPoint.size());
    if (dim == 0) return;

    /* 计算后验概率 */
    QVector<double> resp(g_numComp, 0.0);
    double sumResp = 0.0;
    for (int k = 0; k < g_numComp; ++k) {
        resp[k] = g_weights[k] * computeGaussian(newPoint, g_means[k], g_covs[k]);
        sumResp += resp[k];
    }
    if (sumResp > 1e-300) {
        for (int k = 0; k < g_numComp; ++k) resp[k] /= sumResp;
    }

    /* 增量更新各分量参数 */
    for (int k = 0; k < g_numComp; ++k) {
        double lr = learningRate * resp[k];
        if (lr < 1e-12) continue;

        /* 更新均值 */
        auto& mean = g_means[k];
        for (int d = 0; d < dim && d < mean.size(); ++d) {
            mean[d] = (1.0 - lr) * mean[d] + lr * newPoint[d];
        }

        /* 更新协方差 */
        auto& cov = g_covs[k];
        for (int d1 = 0; d1 < dim && d1 < cov.size(); ++d1) {
            double diff1 = newPoint[d1] - mean[d1];
            for (int d2 = 0; d2 < dim && d2 < cov[d1].size(); ++d2) {
                double diff2 = newPoint[d2] - mean[d2];
                cov[d1][d2] = (1.0 - lr) * cov[d1][d2] + lr * diff1 * diff2;
            }
            cov[d1][d1] += 1e-8; /* 正则化 */
        }

        /* 更新权重 */
        g_weights[k] = (1.0 - learningRate) * g_weights[k] + learningRate * resp[k];
    }

    /* 归一化权重 */
    double wSum = 0.0;
    for (int k = 0; k < g_numComp; ++k) wSum += g_weights[k];
    if (wSum > 1e-300) {
        for (int k = 0; k < g_numComp; ++k) g_weights[k] /= wSum;
    }
}
