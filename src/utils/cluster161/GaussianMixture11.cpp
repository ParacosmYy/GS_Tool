/**
 * @file GaussianMixture11.cpp
 * @brief GaussianMixture11 实现
 *
 * 实现高斯混合模型：KMeans++初始化、EM迭代(E步/M步)、
 * 对角协方差高斯PDF计算和BIC模型选择。
 */

#include "utils/cluster161/GaussianMixture11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
GaussianMixture11::GaussianMixture11(QObject* parent)
    : QObject(parent)
{
}

GaussianMixture11::~GaussianMixture11() = default;

void GaussianMixture11::setKRange(int minK, int maxK)
{
    m_minK = qMax(1, minK);
    m_maxK = qMax(m_minK, maxK);
}

void GaussianMixture11::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(10, maxIter);
}

void GaussianMixture11::setConvergenceThreshold(double threshold)
{
    m_convergenceThreshold = qMax(1e-12, threshold);
}

/**
 * @brief 计算对角高斯PDF的对数
 *
 * log N(x|mu,sigma^2) = -0.5 * [D*log(2pi) + sum(log(sigma_d^2)) + sum((x_d-mu_d)^2/sigma_d^2)]
 */
double GaussianMixture11::logGaussianPDF(const QVector<double>& x, int compIdx) const
{
    const auto& comp = m_components[compIdx];
    const int dim = qMin(x.size(), comp.mean.size());
    double logDet = 0.0;
    double mahal = 0.0;

    for (int d = 0; d < dim; ++d) {
        double var = qMax(comp.variance[d], 1e-10);
        logDet += qLn(var);
        double diff = x[d] - comp.mean[d];
        mahal += diff * diff / var;
    }

    return -0.5 * (dim * qLn(2.0 * M_PI) + logDet + mahal);
}

/**
 * @brief KMeans++初始化分量均值和方差
 */
void GaussianMixture11::initializeComponents(const QVector<QVector<double>>& data, int k)
{
    const int n = data.size();
    const int dim = data[0].size();
    std::mt19937 rng(42);

    /* Pick first center randomly */
    QVector<int> centers;
    centers.reserve(k);
    centers.append(rng() % n);

    QVector<double> minDist(n, std::numeric_limits<double>::max());

    for (int c = 1; c < k; ++c) {
        /* Update min distances to nearest center */
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int dd = 0; dd < dim; ++dd) {
                double diff = data[i][dd] - data[centers[c - 1]][dd];
                d += diff * diff;
            }
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }
        /* Weighted random selection */
        double r = std::uniform_real_distribution<double>(0, totalDist)(rng);
        double cumulative = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumulative += minDist[i];
            if (cumulative >= r) { chosen = i; break; }
        }
        centers.append(chosen);
    }

    /* Initialize components */
    m_components.resize(k);
    for (int c = 0; c < k; ++c) {
        m_components[c].mean = data[centers[c]];
        m_components[c].variance = QVector<double>(dim, 1.0);
        m_components[c].weight = 1.0 / k;
    }
}

/**
 * @brief E步：计算后验概率
 * @return 当前对数似然
 */
double GaussianMixture11::expectationStep(const QVector<QVector<double>>& data)
{
    const int n = data.size();
    const int k = m_components.size();
    m_resp.resize(n);
    for (auto& row : m_resp) row.resize(k);

    double totalLogLikelihood = 0.0;

    for (int i = 0; i < n; ++i) {
        QVector<double> logProbs(k);
        double maxLog = -std::numeric_limits<double>::max();

        for (int c = 0; c < k; ++c) {
            logProbs[c] = qLn(m_components[c].weight) + logGaussianPDF(data[i], c);
            maxLog = qMax(maxLog, logProbs[c]);
        }

        /* Log-sum-exp for numerical stability */
        double sumExp = 0.0;
        for (int c = 0; c < k; ++c) {
            sumExp += qExp(logProbs[c] - maxLog);
        }
        double logSum = maxLog + qLn(sumExp);
        totalLogLikelihood += logSum;

        for (int c = 0; c < k; ++c) {
            m_resp[i][c] = qExp(logProbs[c] - logSum);
        }
    }

    return totalLogLikelihood;
}

/**
 * @brief M步：更新均值、方差、权重
 */
void GaussianMixture11::maximizationStep(const QVector<QVector<double>>& data)
{
    const int n = data.size();
    const int k = m_components.size();
    const int dim = data[0].size();

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        QVector<double> newMean(dim, 0.0);
        QVector<double> newVar(dim, 0.0);

        for (int i = 0; i < n; ++i) {
            double r = m_resp[i][c];
            nk += r;
            for (int d = 0; d < dim; ++d) {
                newMean[d] += r * data[i][d];
            }
        }

        if (nk < 1e-10) continue;

        for (int d = 0; d < dim; ++d) {
            newMean[d] /= nk;
        }

        for (int i = 0; i < n; ++i) {
            double r = m_resp[i][c];
            for (int d = 0; d < dim; ++d) {
                double diff = data[i][d] - newMean[d];
                newVar[d] += r * diff * diff;
            }
        }

        for (int d = 0; d < dim; ++d) {
            newVar[d] = qMax(newVar[d] / nk, 1e-6);
        }

        m_components[c].mean = newMean;
        m_components[c].variance = newVar;
        m_components[c].weight = nk / n;
    }
}

/**
 * @brief 计算BIC
 */
double GaussianMixture11::computeBIC(const QVector<QVector<double>>& data,
                                      double logLikelihood, int k) const
{
    const int n = data.size();
    const int dim = data[0].size();
    /* Parameters: k-1 weights + k*dim means + k*dim variances */
    int numParams = (k - 1) + k * dim * 2;
    return -2.0 * logLikelihood + numParams * qLn(n);
}

/**
 * @brief 用指定K运行EM
 */
double GaussianMixture11::runEM(const QVector<QVector<double>>& data, int k)
{
    initializeComponents(data, k);

    double prevLL = -std::numeric_limits<double>::max();

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        double ll = expectationStep(data);
        maximizationStep(data);

        if (qAbs(ll - prevLL) < m_convergenceThreshold) break;
        prevLL = ll;
    }

    return expectationStep(data);
}

/**
 * @brief 拟合GMM并自动选择最优K
 *
 * 遍历[minK, maxK]，对每个K运行EM，选择BIC最小的K。
 */
QVector<int> GaussianMixture11::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return QVector<int>();

    int maxK = qMin(m_maxK, n);
    double bestBIC = std::numeric_limits<double>::max();
    int bestK = m_minK;
    QVector<Component> bestComponents;
    QVector<QVector<double>> bestResp;

    for (int k = m_minK; k <= maxK; ++k) {
        double ll = runEM(data, k);
        double bic = computeBIC(data, ll, k);

        if (bic < bestBIC) {
            bestBIC = bic;
            bestK = k;
            bestComponents = m_components;
            bestResp = m_resp;
        }
    }

    m_components = bestComponents;
    m_resp = bestResp;

    /* Assign hard labels */
    QVector<int> labels(n);
    for (int i = 0; i < n; ++i) {
        double maxResp = -1.0;
        int bestCluster = 0;
        for (int c = 0; c < bestK; ++c) {
            if (m_resp[i][c] > maxResp) {
                maxResp = m_resp[i][c];
                bestCluster = c;
            }
        }
        labels[i] = bestCluster;
    }

    m_stats.totalFits++;
    m_stats.bestK = bestK;
    m_stats.bestBIC = bestBIC;
    m_stats.finalLogLikelihood = expectationStep(data);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFits > 0)
        ? m_timeSum / m_stats.totalFits : 0.0;

    emit fitCompleted(bestK, bestBIC);
    return labels;
}

void GaussianMixture11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
