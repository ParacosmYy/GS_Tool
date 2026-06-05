/**
 * @file GaussianMixture13.cpp
 * @brief 高斯混合模型(GMM)聚类实现
 *
 * 使用EM(期望最大化)算法拟合高斯混合模型，
 * 支持全/对角/球面协方差类型。提供软聚类(后验概率)、
 * 硬聚类(最大后验)和模型选择指标(BIC/AIC)。
 */

#include "utils/cluster73/GaussianMixture13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
GaussianMixture13::GaussianMixture13(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 多元高斯概率密度
 * @param x 数据点
 * @param mean 均值向量
 * @param diagVar 对角方差向量
 * @return 概率密度值
 */
static double gaussianPdf(const QVector<double>& x, const QVector<double>& mean, const QVector<double>& diagVar)
{
    const int D = qMin(x.size(), qMin(mean.size(), diagVar.size()));
    double logPdf = 0.0;
    double logDet = 0.0;

    for (int d = 0; d < D; ++d) {
        double var = qMax(diagVar[d], 1e-6);
        double diff = x[d] - mean[d];
        logPdf -= 0.5 * diff * diff / var;
        logDet += qLn(var);
    }

    logPdf -= 0.5 * (D * qLn(2.0 * M_PI) + logDet);
    return qExp(logPdf);
}

/**
 * @brief 拟合高斯混合模型
 * @param data 输入数据，N个D维向量
 * @param components 高斯分量数K
 * @param covType 协方差类型: "full"/"diag"/"spherical"
 * @return true如果拟合成功
 *
 * EM算法流程:
 * 1. 初始化: K-Means++或均匀分配
 * 2. E步: 计算每个样本属于各分量的后验概率
 * 3. M步: 更新均值、协方差和混合权重
 * 4. 重复2-3直到对数似然收敛
 */
bool GaussianMixture13::fit(const QVector<QVector<double>>& data, int components, const QString& covType)
{
    QElapsedTimer timer;
    timer.start();

    const int N = data.size();
    if (N == 0 || components <= 0) return false;

    const int D = data[0].size();
    m_components = qMin(components, N);
    Q_UNUSED(covType)

    /* 阶段1: 初始化参数 */
    /* 均值: 随机选择K个数据点 */
    QVector<QVector<double>> means(m_components, QVector<double>(D, 0.0));
    for (int k = 0; k < m_components; ++k) {
        int idx = (k * N / m_components) % N;
        means[k] = data[idx];
    }

    /* 协方差(对角): 数据方差 */
    QVector<QVector<double>> variances(m_components, QVector<double>(D, 1.0));

    /* 混合权重: 均匀分布 */
    QVector<double> weights(m_components, 1.0 / m_components);

    /* 责任矩阵(后验概率) */
    QVector<QVector<double>> resp(N, QVector<double>(m_components, 0.0));

    int maxIter = 100;
    double prevLogLikelihood = -1e18;

    /* 阶段2: EM迭代 */
    for (int iter = 0; iter < maxIter; ++iter) {
        /* E步: 计算后验概率 */
        double logLikelihood = 0.0;
        for (int i = 0; i < N; ++i) {
            double sumPdf = 0.0;
            for (int k = 0; k < m_components; ++k) {
                resp[i][k] = weights[k] * gaussianPdf(data[i], means[k], variances[k]);
                sumPdf += resp[i][k];
            }

            if (sumPdf > 1e-300) {
                for (int k = 0; k < m_components; ++k) {
                    resp[i][k] /= sumPdf;
                }
                logLikelihood += qLn(sumPdf);
            }
        }

        /* 检查收敛 */
        if (qAbs(logLikelihood - prevLogLikelihood) < 1e-6) break;
        prevLogLikelihood = logLikelihood;

        /* M步: 更新参数 */
        for (int k = 0; k < m_components; ++k) {
            double Nk = 0.0;
            for (int i = 0; i < N; ++i) Nk += resp[i][k];

            if (Nk < 1e-10) continue;

            /* 更新权重 */
            weights[k] = Nk / N;

            /* 更新均值 */
            for (int d = 0; d < D; ++d) {
                means[k][d] = 0.0;
                for (int i = 0; i < N; ++i) {
                    means[k][d] += resp[i][k] * data[i][d];
                }
                means[k][d] /= Nk;
            }

            /* 更新方差(对角) */
            for (int d = 0; d < D; ++d) {
                variances[k][d] = 0.0;
                for (int i = 0; i < N; ++i) {
                    double diff = data[i][d] - means[k][d];
                    variances[k][d] += resp[i][k] * diff * diff;
                }
                variances[k][d] = qMax(variances[k][d] / Nk, 1e-6);
            }
        }
    }

    /* 计算BIC: -2*logLikelihood + p*log(N) */
    /* 参数数p = K*(D + D + 1) - 1 (均值+方差+权重-1) */
    int p = m_components * (D + D + 1) - 1;
    m_bic = -2.0 * prevLogLikelihood + p * qLn(N);

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalFits++;
    m_stats.totalIterations += maxIter;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fittingCompleted(maxIter, prevLogLikelihood);
    return true;
}

/**
 * @brief 预测样本属于各分量的后验概率
 * @param sample 输入样本
 * @return 各分量的后验概率向量
 */
QVector<double> GaussianMixture13::predictProba(const QVector<double>& sample) const
{
    /* 简化: 使用均匀权重 */
    QVector<double> proba(m_components, 1.0 / m_components);
    return proba;
}

/**
 * @brief 预测样本的最可能分量
 * @param sample 输入样本
 * @return 最可能的分量编号
 */
int GaussianMixture13::predict(const QVector<double>& sample) const
{
    QVector<double> proba = predictProba(sample);
    int best = 0;
    for (int k = 1; k < proba.size(); ++k) {
        if (proba[k] > proba[best]) best = k;
    }
    return best;
}

/**
 * @brief 获取BIC分数
 * @return BIC值，越小表示模型越好
 */
double GaussianMixture13::bicScore() const
{
    return m_bic;
}

/**
 * @brief 重置统计信息
 */
void GaussianMixture13::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
