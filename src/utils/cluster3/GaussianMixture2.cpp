/**
 * @file GaussianMixture2.cpp
 * @brief 一维高斯混合模型实现 — EM算法
 */

#include "utils/cluster3/GaussianMixture2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>
#include <random>

/** @brief 构造函数 @param numComponents 成分数 @param maxIterations 最大迭代数 @param parent 父对象 */
GaussianMixture2::GaussianMixture2(int numComponents, int maxIterations,
                                   QObject* parent)
    : QObject(parent)
    , m_numComponents(qMax(1, numComponents))
    , m_maxIterations(qMax(1, maxIterations))
    , m_fitted(false)
    , m_timeSum(0.0)
{
    m_means.resize(m_numComponents, 0.0);
    m_variances.resize(m_numComponents, 1.0);
    m_weights.resize(m_numComponents, 1.0 / m_numComponents);
}

/** @brief 高斯概率密度 @param x 输入值 @param mean 均值 @param variance 方差 @return 概率密度 */
double GaussianMixture2::gaussianPdf(double x, double mean, double variance) const
{
    if (variance <= 0.0) variance = 1e-10;
    double diff = x - mean;
    double exponent = -(diff * diff) / (2.0 * variance);
    return qExp(exponent) / (qSqrt(2.0 * M_PI * variance));
}

/** @brief 初始化参数 @param data 输入数据 */
void GaussianMixture2::initializeParameters(const QVector<double>& data)
{
    int n = data.size();
    if (n == 0) return;

    /* 排序后均匀分布初始均值 */
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    double dataMin = sorted.first();
    double dataMax = sorted.last();
    double range = dataMax - dataMin;
    if (range < 1e-10) range = 1.0;

    for (int k = 0; k < m_numComponents; ++k) {
        double frac = static_cast<double>(k + 1)
                      / (m_numComponents + 1);
        m_means[k] = dataMin + frac * range;
        m_variances[k] = range * range / (4.0 * m_numComponents);
        m_weights[k] = 1.0 / m_numComponents;
    }
}

/** @brief 拟合数据 @param data 一维数据 @return 是否成功 */
bool GaussianMixture2::fit(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_numComponents) {
        m_timeSum += timer.elapsed();
        return false;
    }

    /* 初始化参数 */
    initializeParameters(data);

    /* 责任度矩阵 gamma[n][k] */
    QVector<QVector<double>> gamma(n, QVector<double>(m_numComponents, 0.0));

    double prevLogLikelihood = -qInf();
    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIterations; ++iter) {
        /* E步: 计算责任度 */
        for (int i = 0; i < n; ++i) {
            double total = 0.0;
            for (int k = 0; k < m_numComponents; ++k) {
                gamma[i][k] = m_weights[k]
                              * gaussianPdf(data[i], m_means[k], m_variances[k]);
                total += gamma[i][k];
            }
            if (total > 0) {
                for (int k = 0; k < m_numComponents; ++k) {
                    gamma[i][k] /= total;
                }
            }
        }

        /* M步: 更新参数 */
        for (int k = 0; k < m_numComponents; ++k) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) {
                nk += gamma[i][k];
            }

            if (nk < 1e-10) continue;

            /* 更新均值 */
            double sumX = 0.0;
            for (int i = 0; i < n; ++i) {
                sumX += gamma[i][k] * data[i];
            }
            m_means[k] = sumX / nk;

            /* 更新方差 */
            double sumSq = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = data[i] - m_means[k];
                sumSq += gamma[i][k] * diff * diff;
            }
            m_variances[k] = qMax(sumSq / nk, 1e-10);

            /* 更新权重 */
            m_weights[k] = nk / static_cast<double>(n);
        }

        /* 计算对数似然 */
        double logLikelihood = 0.0;
        for (int i = 0; i < n; ++i) {
            double prob = 0.0;
            for (int k = 0; k < m_numComponents; ++k) {
                prob += m_weights[k]
                        * gaussianPdf(data[i], m_means[k], m_variances[k]);
            }
            if (prob > 0) {
                logLikelihood += qLn(prob);
            }
        }

        /* 收敛判断 */
        if (qAbs(logLikelihood - prevLogLikelihood) < 1e-6) {
            converged = true;
            break;
        }
        prevLogLikelihood = logLikelihood;
    }

    m_fitted = true;

    m_timeSum += timer.elapsed();
    ++m_stats.totalFits;
    quint64 totalOps = m_stats.totalFits + m_stats.totalPredictions;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit fitCompleted(iter, converged);
    return true;
}

/** @brief 预测样本所属成分 @param sample 样本值 @return 成分索引 */
int GaussianMixture2::predict(double sample)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_fitted) return 0;

    int bestK = 0;
    double bestProb = -1.0;
    for (int k = 0; k < m_numComponents; ++k) {
        double prob = m_weights[k]
                      * gaussianPdf(sample, m_means[k], m_variances[k]);
        if (prob > bestProb) {
            bestProb = prob;
            bestK = k;
        }
    }

    m_timeSum += timer.elapsed();
    ++m_stats.totalPredictions;
    quint64 totalOps = m_stats.totalFits + m_stats.totalPredictions;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    return bestK;
}

/** @brief 样本属于某成分的概率 @param sample 样本值 @param component 成分索引 @return 后验概率 */
double GaussianMixture2::probability(double sample, int component)
{
    if (!m_fitted || component < 0 || component >= m_numComponents) {
        return 0.0;
    }

    double total = 0.0;
    for (int k = 0; k < m_numComponents; ++k) {
        total += m_weights[k]
                 * gaussianPdf(sample, m_means[k], m_variances[k]);
    }

    if (total <= 0) return 0.0;
    double joint = m_weights[component]
                   * gaussianPdf(sample, m_means[component], m_variances[component]);
    return joint / total;
}

/** @brief 获取各成分均值 @return 均值向量 */
QVector<double> GaussianMixture2::means() const
{
    return m_means;
}

/** @brief 获取各成分方差 @return 方差向量 */
QVector<double> GaussianMixture2::variances() const
{
    return m_variances;
}

/** @brief 获取各成分混合权重 @return 权重向量 */
QVector<double> GaussianMixture2::weights() const
{
    return m_weights;
}

/** @brief 重置统计 */
void GaussianMixture2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
