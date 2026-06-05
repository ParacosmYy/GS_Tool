/**
 * @file GaussianMixture3.cpp
 * @brief 高斯混合模型实现 — EM算法/BIC/AIC选择
 */

#include "utils/gmm/GaussianMixture3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
GaussianMixture3::GaussianMixture3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置高斯分量数 @param k 分量数 */
void GaussianMixture3::setComponentCount(int k)
{
    m_k = qBound(1, k, 10);
}

/** @brief 设置EM算法参数
 *  @param maxIterations 最大迭代次数
 *  @param tolerance 收敛阈值 */
void GaussianMixture3::setEmParameters(int maxIterations, double tolerance)
{
    m_maxIterations = qMax(1, maxIterations);
    m_tolerance = qMax(1e-10, tolerance);
}

/** @brief 拟合数据
 *  @param data 一维数据样本
 *  @return 拟合结果 */
GaussianMixture3::FitResult GaussianMixture3::fit(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;
    int n = data.size();
    if (n < m_k) {
        result.converged = false;
        return result;
    }

    /* 初始化分量参数 */
    result.components.resize(m_k);
    initializeComponents(data);
    for (int k = 0; k < m_k; ++k) {
        result.components[k].weight = 1.0 / m_k;
    }

    double prevLL = -1e18;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        /* E步: 计算后验概率 */
        for (int k = 0; k < m_k; ++k) {
            result.components[k].responsibilities.resize(n);
        }

        for (int i = 0; i < n; ++i) {
            double totalResp = 0.0;
            for (int k = 0; k < m_k; ++k) {
                double pdf = gaussianPdf(data[i],
                    result.components[k].mean,
                    result.components[k].variance);
                double resp = result.components[k].weight * pdf;
                result.components[k].responsibilities[i] = resp;
                totalResp += resp;
            }

            /* 归一化 */
            if (totalResp > 1e-300) {
                for (int k = 0; k < m_k; ++k) {
                    result.components[k].responsibilities[i] /= totalResp;
                }
            }
        }

        /* M步: 更新参数 */
        for (int k = 0; k < m_k; ++k) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) {
                nk += result.components[k].responsibilities[i];
            }

            if (nk < 1e-10) continue;

            /* 更新均值 */
            double newMean = 0.0;
            for (int i = 0; i < n; ++i) {
                newMean += result.components[k].responsibilities[i] * data[i];
            }
            newMean /= nk;

            /* 更新方差 */
            double newVar = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = data[i] - newMean;
                newVar += result.components[k].responsibilities[i]
                          * diff * diff;
            }
            newVar /= nk;
            newVar = qMax(newVar, 1e-6); /* 防止方差为0 */

            result.components[k].mean = newMean;
            result.components[k].variance = newVar;
            result.components[k].weight = nk / n;
        }

        /* 计算对数似然 */
        double ll = computeLogLikelihood(data);
        result.iterations = iter + 1;

        emit iterationProgress(iter + 1, ll);

        /* 收敛检查 */
        if (qAbs(ll - prevLL) < m_tolerance) {
            result.converged = true;
            break;
        }
        prevLL = ll;
    }

    result.logLikelihood = prevLL;
    result.bic = computeBIC(prevLL, n, m_k);
    result.aic = computeAIC(prevLL, m_k);

    m_currentResult = result;
    m_fitted = true;

    ++m_stats.totalFits;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    emit fitCompleted(m_k, result.logLikelihood);
    return result;
}

/** @brief 自动选择最佳分量数并拟合
 *  @param data 数据
 *  @param maxK 最大候选分量数
 *  @return 最佳拟合结果 */
GaussianMixture3::FitResult GaussianMixture3::fitAuto(
    const QVector<double>& data, int maxK)
{
    QElapsedTimer timer;
    timer.start();

    int originalK = m_k;
    FitResult bestResult;
    bestResult.bic = 1e18;

    for (int k = 1; k <= qMin(maxK, data.size()); ++k) {
        m_k = k;
        FitResult candidate = fit(data);
        if (candidate.bic < bestResult.bic) {
            bestResult = candidate;
        }
    }

    m_k = originalK;
    m_currentResult = bestResult;
    m_fitted = true;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);

    return bestResult;
}

/** @brief 预测样本所属聚类 @param sample 输入样本 @return 聚类ID */
int GaussianMixture3::predict(double sample) const
{
    if (!m_fitted) return -1;
    ++m_stats.totalPredictions;

    int bestK = 0;
    double bestProb = -1.0;
    for (int k = 0; k < m_currentResult.components.size(); ++k) {
        double pdf = gaussianPdf(sample,
            m_currentResult.components[k].mean,
            m_currentResult.components[k].variance);
        double prob = m_currentResult.components[k].weight * pdf;
        if (prob > bestProb) {
            bestProb = prob;
            bestK = k;
        }
    }
    return bestK;
}

/** @brief 计算概率密度 @param x 输入值 @return 混合概率密度 */
double GaussianMixture3::probabilityDensity(double x) const
{
    if (!m_fitted) return 0.0;
    double density = 0.0;
    for (const auto& comp : m_currentResult.components) {
        density += comp.weight * gaussianPdf(x, comp.mean, comp.variance);
    }
    return density;
}

/** @brief 获取各分量后验概率 @param x 输入值 @return 概率向量 */
QVector<double> GaussianMixture3::componentProbs(double x) const
{
    if (!m_fitted) return {};

    QVector<double> probs(m_currentResult.components.size());
    double total = 0.0;
    for (int k = 0; k < probs.size(); ++k) {
        probs[k] = m_currentResult.components[k].weight
                   * gaussianPdf(x, m_currentResult.components[k].mean,
                                 m_currentResult.components[k].variance);
        total += probs[k];
    }
    if (total > 1e-300) {
        for (double& p : probs) p /= total;
    }
    return probs;
}

/** @brief 生成随机样本 @param count 样本数 @return 生成的样本 */
QVector<double> GaussianMixture3::sample(int count) const
{
    if (!m_fitted || count <= 0) return {};

    std::mt19937 rng(42);
    QVector<double> samples;
    samples.reserve(count);

    for (int i = 0; i < count; ++i) {
        /* 轮盘赌选择分量 */
        double r = static_cast<double>(rng()) / rng.max();
        double cumSum = 0.0;
        int chosenK = 0;
        for (int k = 0; k < m_currentResult.components.size(); ++k) {
            cumSum += m_currentResult.components[k].weight;
            if (r <= cumSum) {
                chosenK = k;
                break;
            }
        }

        /* Box-Muller变换生成高斯样本 */
        const auto& comp = m_currentResult.components[chosenK];
        double u1 = static_cast<double>(rng()) / rng.max();
        double u2 = static_cast<double>(rng()) / rng.max();
        u1 = qMax(u1, 1e-10);
        double z = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
        samples.append(comp.mean + qSqrt(comp.variance) * z);
    }
    return samples;
}

/** @brief 获取当前拟合结果 */
GaussianMixture3::FitResult GaussianMixture3::currentResult() const
{
    return m_currentResult;
}

/** @brief 获取统计信息 */
GaussianMixture3::Stats GaussianMixture3::stats() const
{
    return m_stats;
}

/** @brief 重置统计 */
void GaussianMixture3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算单分量高斯概率 */
double GaussianMixture3::gaussianPdf(double x, double mean,
                                     double variance) const
{
    double diff = x - mean;
    double exponent = -(diff * diff) / (2.0 * variance);
    return qExp(exponent) / (qSqrt(2.0 * M_PI * variance));
}

/** @brief 计算对数似然 */
double GaussianMixture3::computeLogLikelihood(
    const QVector<double>& data) const
{
    double ll = 0.0;
    for (double x : data) {
        double density = 0.0;
        for (const auto& comp : m_currentResult.components) {
            density += comp.weight * gaussianPdf(x, comp.mean, comp.variance);
        }
        if (density > 1e-300) {
            ll += qLn(density);
        } else {
            ll += -30.0; /* 避免对数(-inf) */
        }
    }
    return ll;
}

/** @brief 初始化分量参数(K-means++思想) */
void GaussianMixture3::initializeComponents(const QVector<double>& data)
{
    int n = data.size();
    if (n == 0) return;

    /* 排序数据后均匀取初始均值 */
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    double dataMin = sorted.first();
    double dataMax = sorted.last();
    double range = dataMax - dataMin;
    if (range < 1e-10) range = 1.0;

    for (int k = 0; k < m_k; ++k) {
        m_currentResult.components.resize(m_k);
        double frac = static_cast<double>(k) / qMax(1, m_k - 1);
        if (m_k == 1) frac = 0.5;
        m_currentResult.components[k].mean = dataMin + frac * range;
        m_currentResult.components[k].variance = (range * range) / (4.0 * m_k);
        m_currentResult.components[k].weight = 1.0 / m_k;
    }
}

/** @brief 计算BIC */
double GaussianMixture3::computeBIC(double ll, int n, int k) const
{
    /* 每个分量3个参数(权重+均值+方差) */
    int numParams = k * 3 - 1;
    return -2.0 * ll + static_cast<double>(numParams) * qLn(n);
}

/** @brief 计算AIC */
double GaussianMixture3::computeAIC(double ll, int k) const
{
    int numParams = k * 3 - 1;
    return -2.0 * ll + 2.0 * static_cast<double>(numParams);
}
