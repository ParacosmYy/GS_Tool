/**
 * @file GaussianMixture.cpp
 * @brief 高斯混合模型(GMM)实现 — EM算法聚类与概率密度估计
 */

#include "utils/fit2/GaussianMixture.h"

#include <QtMath>
#include <QRandomGenerator>
#include <algorithm>

/** @brief 构造函数 @param numComponents 分量数 @param maxIterations 最大迭代 @param parent 父对象 */
GaussianMixture::GaussianMixture(int numComponents, int maxIterations,
                                 QObject* parent)
    : QObject(parent)
    , m_numComponents(qMax(1, numComponents))
    , m_maxIterations(qMax(1, maxIterations))
    , m_dimensions(0)
    , m_tolerance(1e-6)
    , m_fitted(false)
    , m_timeSum(0.0)
{
}

/** @brief 拟合模型 @param data 样本数据 @param dimensions 维度 @return 是否成功 */
bool GaussianMixture::fit(const QVector<QVector<double>>& data,
                           int dimensions)
{
    if (data.size() < m_numComponents * 2) return false;

    m_timer.start();
    m_dimensions = qMax(1, dimensions);
    int n = data.size();

    /* 验证数据维度一致性 */
    for (int i = 0; i < n; ++i) {
        if (data[i].size() != m_dimensions) return false;
    }

    /* 初始化参数 */
    initializeParameters(data);

    double prevLL = -1e30;
    QVector<QVector<double>> responsibilities(n,
        QVector<double>(m_numComponents, 0.0));

    /* EM迭代 */
    int iter = 0;
    for (iter = 0; iter < m_maxIterations; ++iter) {
        expectationStep(data, responsibilities);
        maximizationStep(data, responsibilities);

        double ll = computeLogLikelihood(data);
        if (qAbs(ll - prevLL) < m_tolerance * qAbs(prevLL)) {
            break;
        }
        prevLL = ll;
    }

    double finalLL = computeLogLikelihood(data);
    m_fitted = true;

    /* 统计更新 */
    ++m_stats.totalFits;
    double elapsed = m_timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    emit fitCompleted(iter, finalLL);
    return true;
}

/** @brief 预测聚类标签 @param sample 输入样本 @return 聚类标签 */
int GaussianMixture::predict(const QVector<double>& sample) const
{
    if (!m_fitted) return -1;

    QVector<double> probs = predictProbability(sample);
    int bestIdx = 0;
    double bestProb = probs[0];
    for (int k = 1; k < probs.size(); ++k) {
        if (probs[k] > bestProb) {
            bestProb = probs[k];
            bestIdx = k;
        }
    }
    return bestIdx;
}

/** @brief 预测各分量后验概率 @param sample 输入样本 @return 后验概率向量 */
QVector<double> GaussianMixture::predictProbability(
    const QVector<double>& sample) const
{
    QVector<double> probs(m_numComponents, 0.0);
    if (!m_fitted) return probs;

    double sum = 0.0;
    for (int k = 0; k < m_numComponents; ++k) {
        probs[k] = m_weights[k] * gaussianPdf(sample, k);
        sum += probs[k];
    }

    if (sum > 1e-300) {
        for (int k = 0; k < m_numComponents; ++k) {
            probs[k] /= sum;
        }
    }
    return probs;
}

/** @brief 获取各分量均值 @return 均值向量列表 */
QVector<QVector<double>> GaussianMixture::means() const
{
    return m_means;
}

/** @brief 获取各分量协方差矩阵 @return 协方差矩阵列表 */
QVector<QVector<QVector<double>>> GaussianMixture::covariances() const
{
    return m_covs;
}

/** @brief 获取混合权重 @return 权重向量 */
QVector<double> GaussianMixture::weights() const
{
    return m_weights;
}

/** @brief 重置统计 */
void GaussianMixture::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief K-Means++初始化参数 @param data 样本数据 */
void GaussianMixture::initializeParameters(
    const QVector<QVector<double>>& data)
{
    int n = data.size();
    int d = m_dimensions;

    /* K-Means++选取初始均值 */
    QVector<QVector<double>> centers;
    centers.append(data[QRandomGenerator::global()->bounded(n)]);

    for (int k = 1; k < m_numComponents; ++k) {
        QVector<double> dists(n, 0.0);
        double distSum = 0.0;
        for (int i = 0; i < n; ++i) {
            double minDist = 1e30;
            for (const auto& c : centers) {
                double dist = 0.0;
                for (int j = 0; j < d; ++j) {
                    double diff = data[i][j] - c[j];
                    dist += diff * diff;
                }
                minDist = qMin(minDist, dist);
            }
            dists[i] = minDist;
            distSum += minDist;
        }

        double threshold = QRandomGenerator::global()->generateDouble() * distSum;
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= threshold) {
                centers.append(data[i]);
                break;
            }
        }
    }

    /* 确保分量数正确 */
    while (centers.size() < m_numComponents) {
        centers.append(data[QRandomGenerator::global()->bounded(n)]);
    }

    m_means = centers;

    /* 初始化协方差为单位矩阵 */
    m_covs.clear();
    m_covL.clear();
    for (int k = 0; k < m_numComponents; ++k) {
        QVector<QVector<double>> cov(d, QVector<double>(d, 0.0));
        QVector<QVector<double>> covL(d, QVector<double>(d, 0.0));
        for (int j = 0; j < d; ++j) {
            cov[j][j] = 1.0;
            covL[j][j] = 1.0;
        }
        m_covs.append(cov);
        m_covL.append(covL);
    }

    /* 初始化权重为均匀分布 */
    m_weights.resize(m_numComponents);
    double w = 1.0 / m_numComponents;
    m_weights.fill(w);

    /* 初始化对数行列式(单位矩阵det=1, log=0) */
    m_logDet.resize(m_numComponents);
    m_logDet.fill(0.0);
}

/** @brief E步 — 计算后验概率 @param data 样本数据 @param responsibilities 后验概率矩阵 */
void GaussianMixture::expectationStep(
    const QVector<QVector<double>>& data,
    QVector<QVector<double>>& responsibilities)
{
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < m_numComponents; ++k) {
            responsibilities[i][k] = m_weights[k]
                * gaussianPdf(data[i], k);
            sum += responsibilities[i][k];
        }
        if (sum > 1e-300) {
            for (int k = 0; k < m_numComponents; ++k) {
                responsibilities[i][k] /= sum;
            }
        }
    }
}

/** @brief M步 — 更新参数 @param data 样本数据 @param responsibilities 后验概率 */
void GaussianMixture::maximizationStep(
    const QVector<QVector<double>>& data,
    const QVector<QVector<double>>& responsibilities)
{
    int n = data.size();
    int d = m_dimensions;

    for (int k = 0; k < m_numComponents; ++k) {
        /* 有效样本数 */
        double nk = 0.0;
        for (int i = 0; i < n; ++i) {
            nk += responsibilities[i][k];
        }
        if (nk < 1e-10) continue;

        /* 更新权重 */
        m_weights[k] = nk / static_cast<double>(n);

        /* 更新均值 */
        QVector<double> newMean(d, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < d; ++j) {
                newMean[j] += responsibilities[i][k] * data[i][j];
            }
        }
        for (int j = 0; j < d; ++j) {
            newMean[j] /= nk;
        }
        m_means[k] = newMean;

        /* 更新协方差矩阵 */
        QVector<QVector<double>> newCov(d, QVector<double>(d, 0.0));
        for (int i = 0; i < n; ++i) {
            QVector<double> diff(d);
            for (int j = 0; j < d; ++j) {
                diff[j] = data[i][j] - newMean[j];
            }
            for (int a = 0; a < d; ++a) {
                for (int b = a; b < d; ++b) {
                    newCov[a][b] += responsibilities[i][k] * diff[a] * diff[b];
                }
            }
        }
        double regFactor = 1e-6;
        for (int a = 0; a < d; ++a) {
            for (int b = a; b < d; ++b) {
                newCov[a][b] /= nk;
                newCov[b][a] = newCov[a][b];
            }
            newCov[a][a] += regFactor;
        }
        m_covs[k] = newCov;

        /* Cholesky分解 */
        QVector<QVector<double>> L(d, QVector<double>(d, 0.0));
        double logDet = 0.0;
        for (int a = 0; a < d; ++a) {
            for (int b = 0; b <= a; ++b) {
                double sum = newCov[a][b];
                for (int c = 0; c < b; ++c) {
                    sum -= L[a][c] * L[b][c];
                }
                if (a == b) {
                    L[a][b] = qSqrt(qMax(sum, 1e-10));
                    logDet += qLn(L[a][b]);
                } else {
                    L[a][b] = sum / (L[b][b] > 1e-10 ? L[b][b] : 1e-10);
                }
            }
        }
        m_covL[k] = L;
        m_logDet[k] = 2.0 * logDet;
    }
}

/** @brief 计算对数似然 @param data 样本数据 @return 对数似然值 */
double GaussianMixture::computeLogLikelihood(
    const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        double sampleLL = 0.0;
        for (int k = 0; k < m_numComponents; ++k) {
            sampleLL += m_weights[k] * gaussianPdf(data[i], k);
        }
        if (sampleLL > 0) {
            ll += qLn(sampleLL);
        } else {
            ll += -1e10;
        }
    }
    return ll;
}

/** @brief 多元高斯概率密度 @param x 输入向量 @param component 分量索引 @return 概率密度 */
double GaussianMixture::gaussianPdf(const QVector<double>& x,
                                     int component) const
{
    int d = m_dimensions;
    const auto& mu = m_means[component];
    const auto& L = m_covL[component];

    /* 计算差值并解 L * y = (x - mu) */
    QVector<double> diff(d);
    for (int i = 0; i < d; ++i) {
        diff[i] = x[i] - mu[i];
    }
    QVector<double> y = solveTriangular(L, diff);

    /* Mahalanobis距离平方 */
    double maha = dotProduct(y, y);

    /* 对数概率密度 */
    double logPdf = -0.5 * (static_cast<double>(d) * qLn(2.0 * M_PI)
                     + m_logDet[component] + maha);

    return qExp(qBound(-700.0, logPdf, 700.0));
}

/** @brief 向量点积 @param a 向量a @param b 向量b @return 点积 */
double GaussianMixture::dotProduct(const QVector<double>& a,
                                    const QVector<double>& b)
{
    double result = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

/** @brief 解下三角方程组 @param L 下三角矩阵 @param b 右端向量 @return 解向量 */
QVector<double> GaussianMixture::solveTriangular(
    const QVector<QVector<double>>& L, const QVector<double>& b)
{
    int n = b.size();
    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j) {
            sum -= L[i][j] * x[j];
        }
        x[i] = (qAbs(L[i][i]) > 1e-15) ? sum / L[i][i] : 0.0;
    }
    return x;
}
