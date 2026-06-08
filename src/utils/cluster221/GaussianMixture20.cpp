/**
 * @file GaussianMixture20.cpp
 * @brief GaussianMixture20 实现
 *
 * 实现高斯混合模型：折叠Gibbs采样、Dirichlet过程非参数分量推断。
 */

#include "utils/cluster221/GaussianMixture20.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

GaussianMixture20::GaussianMixture20(QObject *parent) : QObject(parent) {}
GaussianMixture20::~GaussianMixture20() = default;

/* ---- Configuration ---- */

void GaussianMixture20::setParameters(int maxComponents, double alpha,
                                        int burnIn, int numSamples)
{
    m_maxComponents = qMax(2, maxComponents);
    m_alpha = qMax(0.01, alpha);
    m_burnIn = qMax(10, burnIn);
    m_numSamples = qMax(10, numSamples);
}

/* ---- Log multivariate gamma ---- */

double GaussianMixture20::logMVGamma(int d, double x)
{
    double result = d * (d - 1) / 4.0 * M_LN2;
    for (int i = 0; i < d; ++i)
        result += std::lgamma(x + (d - 1 - i) / 2.0);
    return result;
}

/* ---- Squared distance to cluster mean (diagonal covariance) ---- */

double GaussianMixture20::clusterDistance(const QVector<double>& point, int comp) const
{
    if (comp < 0 || comp >= m_means.size()) return std::numeric_limits<double>::max();
    double d = 0.0;
    for (int i = 0; i < qMin(point.size(), m_means[comp].size()); ++i) {
        double diff = point[i] - m_means[comp][i];
        d += diff * diff;
    }
    return d;
}

/* ---- Collapsed Gibbs predictive likelihood ---- */

double GaussianMixture20::collapsedLikelihood(const QVector<double>& point,
                                                 int comp, int compCount) const
{
    if (compCount == 0) return marginalLikelihood(point);

    double kappa0 = 0.01;
    double nu0 = (double)m_dim + 2.0;
    double kappaN = kappa0 + compCount;
    double nuN = nu0 + compCount;

    // Simplified Student-t predictive density
    double logProb = 0.0;
    for (int i = 0; i < qMin(point.size(), m_means[comp].size()); ++i) {
        double diff = point[i] - m_means[comp][i];
        double sigma2 = (1.0 + kappaN) / (kappaN * qMax(nuN - m_dim, 1.0));
        logProb += -0.5 * qLn(2.0 * M_PI * sigma2) - 0.5 * diff * diff / sigma2;
    }
    return logProb;
}

/* ---- Marginal likelihood for new component ---- */

double GaussianMixture20::marginalLikelihood(const QVector<double>& point) const
{
    double sigma2 = 100.0;
    double logProb = 0.0;
    for (int i = 0; i < point.size(); ++i)
        logProb += -0.5 * qLn(2.0 * M_PI * sigma2) - point[i] * point[i] / (2.0 * sigma2);
    return logProb;
}

/* ---- Sample assignment for one point ---- */

int GaussianMixture20::sampleAssignment(int pointIdx,
                                          const QVector<QVector<double>>& data)
{
    int maxK = qMin(m_maxComponents, m_n);
    QVector<double> logProbs(maxK + 1);
    QVector<int> activeCounts(maxK + 1, 0);

    // Count active assignments excluding current point
    for (int i = 0; i < m_n; ++i) {
        if (i != pointIdx && m_assignments[i] >= 0 && m_assignments[i] < maxK)
            activeCounts[m_assignments[i]]++;
    }

    // Compute unnormalized log probabilities
    double maxLogProb = -std::numeric_limits<double>::max();
    for (int k = 0; k <= maxK; ++k) {
        if (k < maxK) {
            double prior = qLn((activeCounts[k] + m_alpha / maxK) /
                               (m_n - 1 + m_alpha));
            logProbs[k] = prior + collapsedLikelihood(data[pointIdx], k, activeCounts[k]);
        } else {
            // New component probability
            double prior = qLn(m_alpha / maxK / (m_n - 1 + m_alpha));
            logProbs[k] = prior + marginalLikelihood(data[pointIdx]);
        }
        maxLogProb = qMax(maxLogProb, logProbs[k]);
    }

    // Normalize via log-sum-exp
    double sum = 0.0;
    for (int k = 0; k <= maxK; ++k) {
        logProbs[k] = qExp(logProbs[k] - maxLogProb);
        sum += logProbs[k];
    }

    // Sample from categorical distribution
    static thread_local std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, sum);
    double u = dist(rng);
    double cumSum = 0.0;
    for (int k = 0; k <= maxK; ++k) {
        cumSum += logProbs[k];
        if (u <= cumSum) return qMin(k, maxK - 1);
    }
    return maxK - 1;
}

/* ---- Update sufficient statistics ---- */

void GaussianMixture20::updateStatistics(const QVector<QVector<double>>& data)
{
    int maxK = qMin(m_maxComponents, m_n);
    m_counts.resize(maxK, 0);
    m_means.resize(maxK);
    for (auto& m : m_means) m.fill(0.0, m_dim);

    m_counts.fill(0);
    for (int i = 0; i < m_n; ++i) {
        int k = qBound(0, m_assignments[i], maxK - 1);
        m_counts[k]++;
        for (int d = 0; d < m_dim; ++d)
            m_means[k][d] += data[i][d];
    }

    for (int k = 0; k < maxK; ++k) {
        if (m_counts[k] > 0) {
            for (int d = 0; d < m_dim; ++d)
                m_means[k][d] /= m_counts[k];
        }
    }

    // Compute weights from DP posterior
    m_weights.resize(maxK);
    double totalAlpha = m_alpha;
    for (int k = 0; k < maxK; ++k)
        m_weights[k] = (m_counts[k] + m_alpha / maxK) / (m_n + totalAlpha);
}

/* ---- Log-likelihood ---- */

double GaussianMixture20::computeLogLikelihood(
    const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    double sigma2 = 1.0;
    for (int i = 0; i < m_n; ++i) {
        int k = qBound(0, m_assignments[i], m_means.size() - 1);
        double logP = qLn(qMax(m_weights[k], 1e-300));
        for (int d = 0; d < m_dim; ++d) {
            double diff = data[i][d] - m_means[k][d];
            logP += -0.5 * qLn(2.0 * M_PI * sigma2) - diff * diff / (2.0 * sigma2);
        }
        ll += logP;
    }
    return ll;
}

/* ---- Fit ---- */

GaussianMixture20::GMMResult GaussianMixture20::fit(
    const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return GMMResult();
    m_n = data.size();
    m_dim = data[0].size();
    m_stats.numPoints = m_n;
    m_stats.dim = m_dim;
    m_stats.maxComponents = m_maxComponents;

    // Initialize: assign each point to a random component
    m_assignments.resize(m_n, 0);
    int initK = qMin(m_maxComponents, qMax(2, m_n / 10));
    for (int i = 0; i < m_n; ++i)
        m_assignments[i] = i % initK;

    updateStatistics(data);

    int totalIter = m_burnIn + m_numSamples;
    GMMResult bestResult;
    double bestLL = -std::numeric_limits<double>::max();

    // Gibbs sampling sweeps
    for (int iter = 0; iter < totalIter; ++iter) {
        for (int i = 0; i < m_n; ++i)
            m_assignments[i] = sampleAssignment(i, data);

        updateStatistics(data);

        if (iter >= m_burnIn) {
            double ll = computeLogLikelihood(data);
            if (ll > bestLL) {
                bestLL = ll;
                bestResult.assignments = m_assignments;
                bestResult.means = m_means;
                bestResult.weights = m_weights;
                bestResult.logLikelihood = ll;
            }
        }
    }

    // Count active components
    int active = 0;
    for (int k = 0; k < m_counts.size(); ++k)
        if (m_counts[k] > 0) active++;

    bestResult.numComponents = active;
    bestResult.iterations = totalIter;
    m_stats.activeComponents = active;
    m_stats.totalIterations += totalIter;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit samplingCompleted(active, totalIter, bestLL, timer.elapsed());
    return bestResult;
}

/* ---- Predict ---- */

QVector<int> GaussianMixture20::predict(const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size(), 0);
    for (int i = 0; i < data.size(); ++i) {
        double bestD = std::numeric_limits<double>::max();
        for (int k = 0; k < m_means.size(); ++k) {
            double d = clusterDistance(data[i], k);
            if (d < bestD) { bestD = d; labels[i] = k; }
        }
    }
    return labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> GaussianMixture20::means() const { return m_means; }
QVector<double> GaussianMixture20::weights() const { return m_weights; }

/* ---- Reset ---- */

void GaussianMixture20::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_assignments.clear();
    m_means.clear();
    m_weights.clear();
    m_counts.clear();
    m_dim = 0;
    m_n = 0;
}
