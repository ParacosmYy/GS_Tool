/**
 * @file GaussianMixture4.cpp
 * @brief 高斯混合模型实现 — EM/Dirichlet先验/BIC/AIC/Gibbs采样
 */

#include "utils/cluster16/GaussianMixture4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>
#include <numeric>

/** @brief 构造函数 @param parent 父对象 */
GaussianMixture4::GaussianMixture4(QObject* parent)
    : QObject(parent)
    , m_k(2)
    , m_alpha(1.0)
    , m_maxIter(200)
    , m_tol(1e-6)
    , m_timeSum(0.0)
{
}

/** @brief 设置分量数 @param k 分量数(≥1) */
void GaussianMixture4::setK(int k)
{
    m_k = qMax(1, k);
}

/** @brief 设置Dirichlet先验浓度参数 @param alpha 浓度参数(>0) */
void GaussianMixture4::setDirichletAlpha(double alpha)
{
    m_alpha = qMax(0.01, alpha);
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void GaussianMixture4::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(10, maxIter);
}

/** @brief 设置收敛阈值 @param tol 对数似然变化阈值 */
void GaussianMixture4::setTolerance(double tol)
{
    m_tol = qMax(1e-10, tol);
}

/** @brief EM拟合 @param data 样本数据 @return 最终对数似然 */
double GaussianMixture4::fit(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_k) return 0.0;

    /* 初始化分量参数 */
    initializeComponents(data);

    double prevLL = -1e30;
    int iter = 0;
    bool converged = false;

    for (iter = 0; iter < m_maxIter; ++iter) {
        /* E步: 计算后验责任 */
        double ll = eStep(data);

        /* M步: 更新参数(带Dirichlet先验) */
        mStep(data);

        /* 检查收敛 */
        if (qAbs(ll - prevLL) < m_tol) {
            converged = true;
            break;
        }
        prevLL = ll;
    }

    double finalLL = computeLogLikelihood(data);

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalFits;
    m_stats.totalSamplesProcessed += static_cast<quint64>(data.size());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    emit fitCompleted(finalLL, iter + 1);
    return finalLL;
}

/** @brief 预测样本属于哪个分量 @param x 样本值 @return 分量索引 */
int GaussianMixture4::predict(double x) const
{
    if (m_components.isEmpty()) return -1;

    int bestIdx = 0;
    double bestResp = -1.0;
    for (int k = 0; k < m_components.size(); ++k) {
        double r = m_components[k].weight * gaussianPdf(x,
            m_components[k].mean, m_components[k].variance);
        if (r > bestResp) {
            bestResp = r;
            bestIdx = k;
        }
    }
    return bestIdx;
}

/** @brief 预测每个分量的后验概率 @param x 样本值 @return 责任向量 */
QVector<double> GaussianMixture4::predictProba(double x) const
{
    QVector<double> proba(m_components.size());
    double total = 0.0;
    for (int k = 0; k < m_components.size(); ++k) {
        proba[k] = m_components[k].weight * gaussianPdf(x,
            m_components[k].mean, m_components[k].variance);
        total += proba[k];
    }
    if (total > 0) {
        for (auto& p : proba) p /= total;
    }
    return proba;
}

/** @brief BIC模型选择 @param data 样本 @param kMin 最小k @param kMax 最大k @return 最优k */
int GaussianMixture4::selectModelBIC(const QVector<double>& data,
                                      int kMin, int kMax)
{
    QElapsedTimer timer;
    timer.start();

    double bestScore = 1e30;
    int bestK = kMin;

    for (int k = kMin; k <= kMax; ++k) {
        setK(k);
        double ll = fit(data);
        double bic = computeBIC(data, k, ll);
        if (bic < bestScore) {
            bestScore = bic;
            bestK = k;
        }
    }

    m_stats.bestBic = bestScore;
    m_stats.bestK = bestK;

    emit modelSelected(bestK, bestScore);
    return bestK;
}

/** @brief AIC模型选择 @param data 样本 @param kMin 最小k @param kMax 最大k @return 最优k */
int GaussianMixture4::selectModelAIC(const QVector<double>& data,
                                      int kMin, int kMax)
{
    double bestScore = 1e30;
    int bestK = kMin;

    for (int k = kMin; k <= kMax; ++k) {
        setK(k);
        double ll = fit(data);
        double aic = computeAIC(data, k, ll);
        if (aic < bestScore) {
            bestScore = aic;
            bestK = k;
        }
    }

    emit modelSelected(bestK, bestScore);
    return bestK;
}

/** @brief Collapsed Gibbs采样 @param data 样本 @param burnIn 燃烧期 @param samples 采样数 */
void GaussianMixture4::gibbsSample(const QVector<double>& data,
                                     int burnIn, int samples)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < 2) return;

    int n = data.size();
    initializeComponents(data);

    /* 分配向量: z[i] = 分量索引 */
    QVector<int> z(n, 0);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uniK(0, m_k - 1);
    for (int i = 0; i < n; ++i) z[i] = uniK(rng);

    /* 计数向量 */
    QVector<int> counts(m_k, 0);
    for (int i = 0; i < n; ++i) counts[z[i]]++;

    /* 超参数(正态-逆Gamma共轭先验) */
    double mu0 = 0.0, kappa0 = 0.01, alphaPrior = 1.0, beta0 = 1.0;

    int totalIters = burnIn + samples;
    QVector<QVector<double>> sampledMeans(samples);
    QVector<QVector<double>> sampledVars(samples);

    for (int t = 0; t < totalIters; ++t) {
        for (int i = 0; i < n; ++i) {
            /* 移除当前样本 */
            counts[z[i]]--;

            /* 采样新的分量分配 */
            QVector<double> logProbs(m_k);
            for (int k = 0; k < m_k; ++k) {
                int nk = counts[k];
                double kappa_n = kappa0 + nk;
                double alpha_n = alphaPrior + nk / 2.0;
                double sumX = 0.0;
                for (int j = 0; j < n; ++j) {
                    if (j != i && z[j] == k) sumX += data[j];
                }
                double mu_n = (kappa0 * mu0 + sumX) / kappa_n;
                double beta_n = beta0 + 0.5 * nk; // 简化
                (void)beta_n;
                logProbs[k] = qLn(nk + m_alpha / m_k)
                    + gaussianPdf(data[i], mu_n, 1.0 / kappa_n);
            }

            /* 归一化并采样 */
            double maxLog = *std::max_element(logProbs.begin(), logProbs.end());
            double totalP = 0.0;
            for (auto& lp : logProbs) {
                lp = qExp(lp - maxLog);
                totalP += lp;
            }
            std::uniform_real_distribution<double> uniF(0.0, totalP);
            double r = uniF(rng);
            double cumSum = 0.0;
            int newK = m_k - 1;
            for (int k = 0; k < m_k; ++k) {
                cumSum += logProbs[k];
                if (r <= cumSum) { newK = k; break; }
            }
            z[i] = newK;
            counts[newK]++;
        }

        /* 燃烧期后记录样本 */
        if (t >= burnIn) {
            int idx = t - burnIn;
            sampledMeans[idx].resize(m_k);
            sampledVars[idx].resize(m_k);
            for (int k = 0; k < m_k; ++k) {
                double sumX = 0.0, sumX2 = 0.0;
                int nk = 0;
                for (int i = 0; i < n; ++i) {
                    if (z[i] == k) { sumX += data[i]; sumX2 += data[i]*data[i]; nk++; }
                }
                if (nk > 0) {
                    sampledMeans[idx][k] = sumX / nk;
                    sampledVars[idx][k] = (sumX2 / nk) - (sumX / nk) * (sumX / nk);
                }
            }
        }
    }

    /* 取后验均值作为最终参数 */
    for (int k = 0; k < m_k; ++k) {
        double meanSum = 0.0, varSum = 0.0;
        for (int s = 0; s < samples; ++s) {
            if (k < sampledMeans[s].size()) {
                meanSum += sampledMeans[s][k];
                varSum += sampledVars[s][k];
            }
        }
        m_components[k].mean = meanSum / samples;
        m_components[k].variance = qMax(1e-10, varSum / samples);
        m_components[k].weight = static_cast<double>(counts[k]) / n;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalFits;
    m_stats.totalSamplesProcessed += static_cast<quint64>(n);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    emit fitCompleted(computeLogLikelihood(data), totalIters);
}

/** @brief 计算概率密度 @param x 样本值 @return 混合密度p(x) */
double GaussianMixture4::density(double x) const
{
    double p = 0.0;
    for (const auto& c : m_components) {
        p += c.weight * gaussianPdf(x, c.mean, c.variance);
    }
    return p;
}

/** @brief 重置统计 */
void GaussianMixture4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 高斯概率密度 @param x 样本 @param mean 均值 @param variance 方差
 * @return p(x|N(mean,variance))
 */
double GaussianMixture4::gaussianPdf(double x, double mean, double variance) const
{
    if (variance <= 0) variance = 1e-10;
    double diff = x - mean;
    return qExp(-0.5 * diff * diff / variance)
        / (qSqrt(2.0 * M_PI * variance));
}

/** @brief 计算对数似然 @param data 样本 @return log-likelihood */
double GaussianMixture4::computeLogLikelihood(const QVector<double>& data) const
{
    double ll = 0.0;
    for (double x : data) {
        double p = density(x);
        if (p > 0) ll += qLn(p);
        else ll += -30.0;
    }
    return ll;
}

/** @brief 计算BIC @param data 样本 @param k 分量数 @param ll 对数似然 @return BIC值 */
double GaussianMixture4::computeBIC(const QVector<double>& data,
                                     int k, double ll) const
{
    int p = 3 * k - 1; /* 每个分量: weight, mean, variance 减去约束 */
    return -2.0 * ll + static_cast<double>(p) * qLn(data.size());
}

/** @brief 计算AIC @param data 样本 @param k 分量数 @param ll 对数似然 @return AIC值 */
double GaussianMixture4::computeAIC(const QVector<double>& data,
                                     int k, double ll) const
{
    (void)data;
    int p = 3 * k - 1;
    return -2.0 * ll + 2.0 * static_cast<double>(p);
}

/** @brief 初始化分量参数(K-means++风格) @param data 样本 */
void GaussianMixture4::initializeComponents(const QVector<double>& data)
{
    m_components.resize(m_k);
    if (data.isEmpty()) return;

    /* 按数据范围均匀初始化均值 */
    double minVal = *std::min_element(data.begin(), data.end());
    double maxVal = *std::max_element(data.begin(), data.end());
    double range = maxVal - minVal;
    if (range < 1e-10) range = 1.0;

    /* K-means++种子选择 */
    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    m_components[0].mean = minVal + range * uni(rng);
    for (int k = 1; k < m_k; ++k) {
        QVector<double> dists(data.size());
        double totalDist = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            double minDist = 1e30;
            for (int j = 0; j < k; ++j) {
                double d = qAbs(data[i] - m_components[j].mean);
                if (d < minDist) minDist = d;
            }
            dists[i] = minDist * minDist;
            totalDist += dists[i];
        }
        double r = uni(rng) * totalDist;
        double cumSum = 0.0;
        for (int i = 0; i < data.size(); ++i) {
            cumSum += dists[i];
            if (cumSum >= r) {
                m_components[k].mean = data[i];
                break;
            }
        }
    }

    /* 初始化方差和权重 */
    for (int k = 0; k < m_k; ++k) {
        m_components[k].variance = range * range / (4.0 * m_k);
        m_components[k].weight = 1.0 / m_k;
        m_components[k].responsibilities.resize(data.size());
    }
}

/** @brief E步: 计算后验责任 @param data 样本 @return 对数似然 */
double GaussianMixture4::eStep(const QVector<double>& data)
{
    for (auto& c : m_components) {
        c.responsibilities.resize(data.size());
    }

    double ll = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double totalR = 0.0;
        for (int k = 0; k < m_k; ++k) {
            m_components[k].responsibilities[i] = m_components[k].weight
                * gaussianPdf(data[i], m_components[k].mean,
                              m_components[k].variance);
            totalR += m_components[k].responsibilities[i];
        }
        if (totalR > 0) {
            for (int k = 0; k < m_k; ++k) {
                m_components[k].responsibilities[i] /= totalR;
            }
            ll += qLn(totalR);
        }
    }
    return ll;
}

/** @brief M步: 更新参数(带Dirichlet先验) @param data 样本 */
void GaussianMixture4::mStep(const QVector<double>& data)
{
    int n = data.size();
    for (int k = 0; k < m_k; ++k) {
        double nk = 0.0;
        double sumR = 0.0, sumRX = 0.0, sumRX2 = 0.0;
        for (int i = 0; i < n; ++i) {
            double r = m_components[k].responsibilities[i];
            nk += r;
            sumR += r;
            sumRX += r * data[i];
            sumRX2 += r * data[i] * data[i];
        }

        /* 带Dirichlet先验的权重更新 */
        m_components[k].weight = (nk + m_alpha - 1.0)
            / (n + m_alpha * m_k - m_k);

        /* 均值更新 */
        if (nk > 1e-10) {
            m_components[k].mean = sumRX / nk;
            double var = (sumRX2 / nk) - m_components[k].mean * m_components[k].mean;
            m_components[k].variance = qMax(1e-10, var);
        }
    }
}
