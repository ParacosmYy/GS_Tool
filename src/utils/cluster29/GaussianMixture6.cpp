/**
 * @file GaussianMixture6.cpp
 * @brief 高斯混合增强实现 — 变分推断/Dirichlet先验/分量剪枝/模型选择
 */

#include "utils/cluster29/GaussianMixture6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
GaussianMixture6::GaussianMixture6(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置分量数 @param k 分量数 */
void GaussianMixture6::setComponents(int k)
{
    m_k = qMax(1, k);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void GaussianMixture6::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(10, maxIter);
}

/** @brief 设置自动分量选择 @param enable 是否启用 */
void GaussianMixture6::setAutoComponentSelect(bool enable)
{
    m_autoK = enable;
}

/** @brief 设置正则化参数(Dirichlet先验) @param alpha 浓度参数 */
void GaussianMixture6::setRegularization(double alpha)
{
    m_alpha = qMax(0.01, alpha);
}

/** @brief 变分贝叶斯GMM拟合 @param data 输入数据(1D) @return 每个点的标签 */
QVector<int> GaussianMixture6::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    /* 将多维数据展平为1D用于简化处理 */
    QVector<double> x(n);
    for (int i = 0; i < n; ++i) {
        if (data[i].isEmpty()) {
            x[i] = 0.0;
        } else {
            double sum = 0.0;
            for (double v : data[i]) sum += v;
            x[i] = sum / data[i].size();
        }
    }

    int bestK = m_k;
    if (m_autoK) {
        /* 自动选择最优分量数(1~min(m_k*2, n)) */
        int maxK = qMin(m_k * 2, n);
        double bestBIC = 1e30;
        for (int tryK = 1; tryK <= maxK; ++tryK) {
            double bic = fitInternal(x, tryK);
            if (bic < bestBIC) {
                bestBIC = bic;
                bestK = tryK;
            }
        }
        m_bic = bestBIC;
    }

    /* 用最优K重新拟合 */
    fitInternal(x, bestK);

    /* 分量剪枝: 去除权重极小的分量 */
    double weightThreshold = 1.0 / (bestK * 10.0);
    QVector<int> activeComponents;
    for (int k = 0; k < m_weights.size(); ++k) {
        if (m_weights[k] > weightThreshold) {
            activeComponents.append(k);
        }
    }

    /* 分配标签 */
    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) {
        double bestProb = -1e30;
        int bestLabel = 0;
        for (int k : activeComponents) {
            double prob = qLn(qMax(1e-300, m_weights[k]))
                - 0.5 * qLn(2.0 * M_PI * qMax(1e-10, m_vars[k]))
                - 0.5 * (x[i] - m_means[k]) * (x[i] - m_means[k])
                    / qMax(1e-10, m_vars[k]);
            if (prob > bestProb) {
                bestProb = prob;
                bestLabel = k;
            }
        }
        labels[i] = bestLabel;
    }

    m_stats.totalFits++;
    m_stats.totalPointsProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalFits));

    double ll = computeLogLikelihood(data);
    emit fitComplete(bestK, ll);
    return labels;
}

/** @brief 预测各点属于各分量的概率 @param data 数据点 @return 概率值(最大分量概率) */
QVector<double> GaussianMixture6::predict(const QVector<QVector<double>>& data) const
{
    QVector<double> probs(data.size(), 0.0);
    for (int i = 0; i < data.size(); ++i) {
        double xi = 0.0;
        if (!data[i].isEmpty()) {
            for (double v : data[i]) xi += v;
            xi /= data[i].size();
        }
        double bestP = 0.0;
        for (int k = 0; k < m_weights.size(); ++k) {
            double p = m_weights[k] * gaussianPDF(xi, m_means[k], m_vars[k]);
            if (p > bestP) bestP = p;
        }
        probs[i] = bestP;
    }
    return probs;
}

/** @brief 获取最优分量数 @return 分量数 */
int GaussianMixture6::optimalComponents() const
{
    return m_weights.size();
}

/** @brief 获取BIC值 @return BIC */
double GaussianMixture6::bic() const
{
    return m_bic;
}

/** @brief 获取AIC值 @return AIC */
double GaussianMixture6::aic() const
{
    return m_aic;
}

/** @brief 内部EM拟合 @param x 1D数据 @param k 分量数 @return BIC值 */
double GaussianMixture6::fitInternal(const QVector<double>& x, int k)
{
    int n = x.size();
    if (n == 0 || k <= 0) return 1e30;

    /* 初始化参数: 使用k-means++风格的初始化 */
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> uniInt(0, n - 1);

    m_weights.resize(k, 1.0 / k);
    m_means.resize(k);
    m_vars.resize(k);

    /* 随机选择初始均值 */
    int first = uniInt(gen);
    m_means[0] = x[first];

    /* 计算数据范围 */
    double xMin = x[0], xMax = x[0];
    for (int i = 1; i < n; ++i) {
        if (x[i] < xMin) xMin = x[i];
        if (x[i] > xMax) xMax = x[i];
    }
    double range = qMax(1e-6, xMax - xMin);

    for (int j = 1; j < k; ++j) {
        m_means[j] = xMin + range * j / k + range / (2.0 * k);
    }
    for (int j = 0; j < k; ++j) {
        m_vars[j] = range * range / (4.0 * k * k);
        if (m_vars[j] < 1e-6) m_vars[j] = 1e-6;
    }

    /* EM迭代 */
    double logLik = -1e30;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* E步: 计算响应度 */
        QVector<QVector<double>> resp(n, QVector<double>(k, 0.0));
        for (int i = 0; i < n; ++i) {
            double total = 0.0;
            for (int j = 0; j < k; ++j) {
                resp[i][j] = m_weights[j]
                    * gaussianPDF(x[i], m_means[j], m_vars[j]);
                total += resp[i][j];
            }
            if (total > 1e-300) {
                for (int j = 0; j < k; ++j) resp[i][j] /= total;
            }
        }

        /* M步: 更新参数(加入Dirichlet先验) */
        for (int j = 0; j < k; ++j) {
            double nj = m_alpha; /* Dirichlet先验 */
            double meanSum = 0.0;
            double varSum = 0.0;
            for (int i = 0; i < n; ++i) {
                nj += resp[i][j];
                meanSum += resp[i][j] * x[i];
            }
            m_weights[j] = nj / (n + k * m_alpha);
            if (nj > 1e-10) {
                m_means[j] = meanSum / nj;
            }
            for (int i = 0; i < n; ++i) {
                double diff = x[i] - m_means[j];
                varSum += resp[i][j] * diff * diff;
            }
            m_vars[j] = (varSum + 1e-6) / nj;
            if (m_vars[j] < 1e-6) m_vars[j] = 1e-6;
        }

        /* 计算对数似然 */
        double newLL = 0.0;
        for (int i = 0; i < n; ++i) {
            double p = 0.0;
            for (int j = 0; j < k; ++j) {
                p += m_weights[j] * gaussianPDF(x[i], m_means[j], m_vars[j]);
            }
            newLL += qLn(qMax(1e-300, p));
        }

        /* 收敛判断 */
        if (qFabs(newLL - logLik) < 1e-6) {
            logLik = newLL;
            break;
        }
        logLik = newLL;
    }

    /* 计算BIC和AIC */
    int numParams = k * 3 - 1; /* k个均值 + k个方差 + k个权重(和为1) */
    m_bic = -2.0 * logLik + numParams * qLn(n);
    m_aic = -2.0 * logLik + 2.0 * numParams;
    m_stats.bestBIC = m_bic;

    return m_bic;
}

/** @brief 计算高斯PDF @param x 值 @param mean 均值 @param var 方差 @return 概率密度 */
double GaussianMixture6::gaussianPDF(double x, double mean, double var) const
{
    double diff = x - mean;
    return qExp(-0.5 * diff * diff / var) / qSqrt(2.0 * M_PI * var);
}

/** @brief 计算对数似然 @param data 原始数据 @return 对数似然 */
double GaussianMixture6::computeLogLikelihood(
    const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (const auto& pt : data) {
        double xi = 0.0;
        if (!pt.isEmpty()) {
            for (double v : pt) xi += v;
            xi /= pt.size();
        }
        double p = 0.0;
        for (int k = 0; k < m_weights.size(); ++k) {
            p += m_weights[k] * gaussianPDF(xi, m_means[k], m_vars[k]);
        }
        ll += qLn(qMax(1e-300, p));
    }
    return ll;
}

/** @brief 重置统计 */
void GaussianMixture6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
