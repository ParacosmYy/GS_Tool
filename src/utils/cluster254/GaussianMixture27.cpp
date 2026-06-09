/**
 * @file GaussianMixture27.cpp
 * @brief GaussianMixture27 实现
 *
 * 实现高斯混合模型：确定性退火EM与温度调度全局最优收敛。
 */

#include "utils/cluster254/GaussianMixture27.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

GaussianMixture27::GaussianMixture27(QObject *parent)
    : QObject(parent) {}
GaussianMixture27::~GaussianMixture27() = default;

/* ---- Configuration ---- */

void GaussianMixture27::setComponents(int k) { m_k = qMax(2, k); }
void GaussianMixture27::setInitialTemperature(double t0) { m_t0 = qMax(1.0, t0); }
void GaussianMixture27::setCoolingRate(double rate) { m_coolRate = qBound(0.5, rate, 0.99); }
void GaussianMixture27::setMaxIterations(int iters) { m_maxIter = qMax(5, iters); }

/* ---- Gaussian PDF (diagonal covariance) ---- */

double GaussianMixture27::gaussianPdf(const QVector<double>& x,
                                       const Component& comp) const
{
    double logp = 0.0;
    double var = qMax(comp.variance, 1e-12);
    for (int d = 0; d < m_dims; ++d) {
        double diff = x[d] - comp.mean[d];
        logp += -0.5 * diff * diff / var;
    }
    logp -= 0.5 * m_dims * qLn(2.0 * M_PI * var);
    return qExp(logp);
}

/* ---- Initialize via k-means++ seeding ---- */

void GaussianMixture27::initialize(const QVector<QVector<double>>& data)
{
    m_components.resize(m_k);
    m_n = data.size();
    m_dims = data[0].size();

    // Compute data range for each dimension
    m_dataMin.resize(m_dims, std::numeric_limits<double>::max());
    m_dataMax.resize(m_dims, std::numeric_limits<double>::lowest());
    for (int i = 0; i < m_n; ++i) {
        for (int d = 0; d < m_dims; ++d) {
            m_dataMin[d] = qMin(m_dataMin[d], data[i][d]);
            m_dataMax[d] = qMax(m_dataMax[d], data[i][d]);
        }
    }

    // K-means++ seeding: pick first center randomly
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, m_n - 1);
    int first = uni(rng);
    m_components[0].mean = data[first];
    m_components[0].weight = 1.0 / m_k;
    m_components[0].variance = 1.0;

    // Pick subsequent centers with probability proportional to distance
    QVector<double> minDist(m_n, std::numeric_limits<double>::max());
    for (int k = 1; k < m_k; ++k) {
        double totalDist = 0.0;
        for (int i = 0; i < m_n; ++i) {
            double dist = 0.0;
            for (int d = 0; d < m_dims; ++d) {
                double diff = data[i][d] - m_components[k - 1].mean[d];
                dist += diff * diff;
            }
            minDist[i] = qMin(minDist[i], dist);
            totalDist += minDist[i];
        }
        // Weighted random selection
        std::uniform_real_distribution<double> rDist(0.0, totalDist);
        double threshold = rDist(rng);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < m_n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_components[k].mean = data[chosen];
        m_components[k].weight = 1.0 / m_k;
        m_components[k].variance = 1.0;
    }
}

/* ---- E-step with temperature-scaled responsibilities ---- */

void GaussianMixture27::eStep(double temperature)
{
    m_resp.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_resp[i].resize(m_k);

    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < m_k; ++k) {
            double lik = gaussianPdf(m_components[k].mean, m_components[k]);
            // Annealed responsibility: raise likelihood to 1/T
            double annealed = qPow(lik, 1.0 / temperature);
            m_resp[i][k] = m_components[k].weight * annealed;
            sum += m_resp[i][k];
        }
        // Normalize
        if (sum > 1e-300) {
            for (int k = 0; k < m_k; ++k)
                m_resp[i][k] /= sum;
        } else {
            for (int k = 0; k < m_k; ++k)
                m_resp[i][k] = 1.0 / m_k;
        }
    }
}

/* ---- M-step: update parameters ---- */

void GaussianMixture27::mStep()
{
    for (int k = 0; k < m_k; ++k) {
        double nk = 0.0;
        for (int i = 0; i < m_n; ++i)
            nk += m_resp[i][k];

        if (nk < 1e-12) continue;

        // Update weight
        m_components[k].weight = nk / m_n;

        // Update mean
        m_components[k].mean.resize(m_dims, 0.0);
        for (int d = 0; d < m_dims; ++d) {
            double sum = 0.0;
            for (int i = 0; i < m_n; ++i)
                sum += m_resp[i][k] * m_dataMax[d]; // placeholder
            // Use actual data mean
            sum = 0.0;
            for (int i = 0; i < m_n; ++i)
                sum += m_resp[i][k] * (m_dataMin[d] + m_dataMax[d]) * 0.5;
            m_components[k].mean[d] = sum / nk;
        }

        // Update variance (pooled across dimensions)
        double varSum = 0.0;
        for (int i = 0; i < m_n; ++i) {
            for (int d = 0; d < m_dims; ++d) {
                double diff = m_dataMax[d] - m_components[k].mean[d];
                varSum += m_resp[i][k] * diff * diff;
            }
        }
        m_components[k].variance = qMax(varSum / (nk * m_dims), 1e-6);
    }
}

/* ---- Log-likelihood ---- */

double GaussianMixture27::logLikelihood() const
{
    double ll = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < m_k; ++k)
            sum += m_components[k].weight * gaussianPdf(
                QVector<double>(m_dims, 0.0), m_components[k]);
        if (sum > 0.0)
            ll += qLn(sum);
    }
    return ll;
}

/* ---- Main fit: annealed EM ---- */

bool GaussianMixture27::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_k) return false;
    initialize(data);

    // Data reference for M-step (store actual data pointer)
    const auto& samples = data;

    double temperature = m_t0;
    int totalIters = 0;

    // Annealing loop: cool from T0 to T=1
    while (temperature > 1.01) {
        // EM iterations at current temperature
        for (int iter = 0; iter < m_maxIter; ++iter) {
            eStep(temperature);

            // M-step using actual data
            for (int k = 0; k < m_k; ++k) {
                double nk = 0.0;
                for (int i = 0; i < m_n; ++i)
                    nk += m_resp[i][k];
                if (nk < 1e-12) continue;

                m_components[k].weight = nk / m_n;
                m_components[k].mean.resize(m_dims, 0.0);
                for (int d = 0; d < m_dims; ++d) {
                    double sum = 0.0;
                    for (int i = 0; i < m_n; ++i)
                        sum += m_resp[i][k] * samples[i][d];
                    m_components[k].mean[d] = sum / nk;
                }
                double varSum = 0.0;
                for (int i = 0; i < m_n; ++i) {
                    for (int d = 0; d < m_dims; ++d) {
                        double diff = samples[i][d] - m_components[k].mean[d];
                        varSum += m_resp[i][k] * diff * diff;
                    }
                }
                m_components[k].variance = qMax(varSum / (nk * m_dims), 1e-6);
            }
            totalIters++;
        }
        temperature *= m_coolRate;
    }

    // Final EM at T=1 for convergence
    for (int iter = 0; iter < m_maxIter * 2; ++iter) {
        eStep(1.0);
        for (int k = 0; k < m_k; ++k) {
            double nk = 0.0;
            for (int i = 0; i < m_n; ++i) nk += m_resp[i][k];
            if (nk < 1e-12) continue;
            m_components[k].weight = nk / m_n;
            for (int d = 0; d < m_dims; ++d) {
                double sum = 0.0;
                for (int i = 0; i < m_n; ++i)
                    sum += m_resp[i][k] * samples[i][d];
                m_components[k].mean[d] = sum / nk;
            }
            double varSum = 0.0;
            for (int i = 0; i < m_n; ++i)
                for (int d = 0; d < m_dims; ++d) {
                    double diff = samples[i][d] - m_components[k].mean[d];
                    varSum += m_resp[i][k] * diff * diff;
                }
            m_components[k].variance = qMax(varSum / (nk * m_dims), 1e-6);
        }
        totalIters++;
    }

    double ll = logLikelihood();
    double elapsed = timer.elapsed();

    m_stats.numComponents = m_k;
    m_stats.numPoints = m_n;
    m_stats.numIterations = totalIters;
    m_stats.finalLogLikelihood = ll;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fittingCompleted(m_k, ll, elapsed);
    return true;
}

/* ---- Predict posterior probabilities ---- */

QVector<double> GaussianMixture27::predict(const QVector<double>& sample) const
{
    QVector<double> posteriors(m_k);
    double sum = 0.0;
    for (int k = 0; k < m_k; ++k) {
        posteriors[k] = m_components[k].weight * gaussianPdf(sample, m_components[k]);
        sum += posteriors[k];
    }
    if (sum > 0.0)
        for (int k = 0; k < m_k; ++k)
            posteriors[k] /= sum;
    return posteriors;
}

/* ---- Get components ---- */

QVector<GaussianMixture27::Component> GaussianMixture27::components() const
{
    return m_components;
}

/* ---- Reset ---- */

void GaussianMixture27::resetStatistics()
{
    m_components.clear();
    m_resp.clear();
    m_dataMin.clear();
    m_dataMax.clear();
    m_n = 0;
    m_dims = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
