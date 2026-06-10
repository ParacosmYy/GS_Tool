/**
 * @file GaussianMixture29.cpp
 * @brief GaussianMixture29 实现
 *
 * 实现高斯混合模型：序贯蒙特卡洛EM与重要性加权充分统计量在线流式。
 */

#include "utils/cluster263/GaussianMixture29.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture29::GaussianMixture29(QObject *parent)
    : QObject(parent) {}

GaussianMixture29::~GaussianMixture29() = default;

/* ---- Configuration ---- */

void GaussianMixture29::setParameters(int numComponents, double learningRate,
                                       int maxIterations)
{
    m_K = qMax(2, numComponents);
    m_lr = qBound(0.001, learningRate, 1.0);
    m_maxIter = qMax(1, maxIterations);
}

/* ---- Gaussian PDF ---- */

double GaussianMixture29::gaussianPdf(const QVector<double>& x, int k) const
{
    const auto& comp = m_components[k];
    int d = x.size();
    double var = qMax(comp.variance, 1e-10);
    double logProb = -0.5 * d * qLn(2.0 * M_PI * var);
    for (int i = 0; i < d; ++i) {
        double diff = x[i] - comp.mean[i];
        logProb -= 0.5 * diff * diff / var;
    }
    return qExp(logProb);
}

/* ---- Initialize sufficient statistics ---- */

void GaussianMixture29::initSufficientStats()
{
    m_suffStatN.resize(m_K);
    m_suffStatMean.resize(m_K);
    m_suffStatVar.resize(m_K);
    for (int k = 0; k < m_K; ++k) {
        m_suffStatN[k] = 0.0;
        m_suffStatMean[k].resize(m_dim, 0.0);
        m_suffStatVar[k] = 0.0;
    }
}

/* ---- E-step ---- */

QVector<QVector<double>> GaussianMixture29::eStep(
    const QVector<QVector<double>>& data) const
{
    int n = data.size();
    QVector<QVector<double>> resp(n);
    for (int i = 0; i < n; ++i) {
        resp[i].resize(m_K, 0.0);
        double sum = 0.0;
        for (int k = 0; k < m_K; ++k) {
            resp[i][k] = m_components[k].weight * gaussianPdf(data[i], k);
            sum += resp[i][k];
        }
        if (sum > 1e-300) {
            for (int k = 0; k < m_K; ++k)
                resp[i][k] /= sum;
        }
    }
    return resp;
}

/* ---- M-step ---- */

void GaussianMixture29::mStep(const QVector<QVector<double>>& data,
                                const QVector<QVector<double>>& resp)
{
    int n = data.size();
    for (int k = 0; k < m_K; ++k) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += resp[i][k];

        if (nk < 1e-10) continue;

        m_components[k].weight = nk / n;
        for (int d = 0; d < m_dim; ++d) {
            double meanSum = 0.0;
            for (int i = 0; i < n; ++i)
                meanSum += resp[i][k] * data[i][d];
            m_components[k].mean[d] = meanSum / nk;
        }

        double varSum = 0.0;
        for (int i = 0; i < n; ++i)
            for (int d = 0; d < m_dim; ++d) {
                double diff = data[i][d] - m_components[k].mean[d];
                varSum += resp[i][k] * diff * diff;
            }
        m_components[k].variance = qMax(1e-6, varSum / (nk * m_dim));
    }
}

/* ---- Initialize with k-means seeding ---- */

void GaussianMixture29::initialize(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return;
    m_n = data.size();
    m_dim = data[0].size();
    m_components.resize(m_K);

    // Pick K random samples as initial centroids
    for (int k = 0; k < m_K; ++k) {
        int idx = (k * m_n) / m_K;
        m_components[k].weight = 1.0 / m_K;
        m_components[k].mean = data[idx];
        m_components[k].variance = 1.0;
    }
    initSufficientStats();
}

/* ---- Batch fit ---- */

bool GaussianMixture29::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();
    if (data.size() < m_K) return false;

    initialize(data);

    double prevLL = -std::numeric_limits<double>::max();
    for (int iter = 0; iter < m_maxIter; ++iter) {
        auto resp = eStep(data);
        mStep(data, resp);
        double ll = logLikelihood(data);
        if (qAbs(ll - prevLL) < 1e-6) break;
        prevLL = ll;
    }

    double elapsed = timer.elapsed();
    m_stats.numComponents = m_K;
    m_stats.dimension = m_dim;
    m_stats.numSamples = m_n;
    m_stats.logLikelihood = prevLL;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit modelUpdated(m_K, prevLL, elapsed);
    return true;
}

/* ---- Online update with SMC ---- */

void GaussianMixture29::perturbParticles(const QVector<double>& sample,
                                           double weight)
{
    // Importance-weighted sufficient statistics update
    for (int k = 0; k < m_K; ++k) {
        double resp = m_components[k].weight * gaussianPdf(sample, k);
        if (resp < 1e-300) continue;

        m_suffStatN[k] += weight * resp;
        for (int d = 0; d < m_dim; ++d)
            m_suffStatMean[k][d] += weight * resp * sample[d];

        double diffSq = 0.0;
        for (int d = 0; d < m_dim; ++d) {
            double diff = sample[d] - m_components[k].mean[d];
            diffSq += diff * diff;
        }
        m_suffStatVar[k] += weight * resp * diffSq;
    }
}

void GaussianMixture29::updateOnline(const QVector<double>& sample)
{
    QElapsedTimer timer;
    timer.start();
    if (m_components.isEmpty()) {
        m_dim = sample.size();
        m_components.resize(m_K);
        for (int k = 0; k < m_K; ++k) {
            m_components[k].weight = 1.0 / m_K;
            m_components[k].mean = sample;
            m_components[k].variance = 1.0;
        }
        initSufficientStats();
    }

    // Compute responsibilities for the new sample
    double total = 0.0;
    QVector<double> resp(m_K);
    for (int k = 0; k < m_K; ++k) {
        resp[k] = m_components[k].weight * gaussianPdf(sample, k);
        total += resp[k];
    }
    if (total > 1e-300)
        for (int k = 0; k < m_K; ++k) resp[k] /= total;

    // Update sufficient statistics with importance weight
    perturbParticles(sample, m_lr);

    // Update parameters from sufficient statistics
    double totalN = 0.0;
    for (int k = 0; k < m_K; ++k) totalN += m_suffStatN[k];

    if (totalN > 1e-10) {
        for (int k = 0; k < m_K; ++k) {
            double nk = m_suffStatN[k];
            m_components[k].weight = nk / totalN;
            for (int d = 0; d < m_dim; ++d)
                m_components[k].mean[d] = m_suffStatMean[k][d] / nk;
            m_components[k].variance = qMax(1e-6, m_suffStatVar[k] / (nk * m_dim));
        }
    }

    m_n++;
    double elapsed = timer.elapsed();
    m_stats.numSamples = m_n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit modelUpdated(m_K, 0.0, elapsed);
}

/* ---- Predict ---- */

QVector<int> GaussianMixture29::predict(
    const QVector<QVector<double>>& data) const
{
    auto resp = eStep(data);
    QVector<int> labels(data.size());
    for (int i = 0; i < data.size(); ++i) {
        int best = 0;
        double bestVal = resp[i][0];
        for (int k = 1; k < m_K; ++k) {
            if (resp[i][k] > bestVal) { bestVal = resp[i][k]; best = k; }
        }
        labels[i] = best;
    }
    return labels;
}

/* ---- Posterior probabilities ---- */

QVector<QVector<double>> GaussianMixture29::posteriorProbabilities(
    const QVector<QVector<double>>& data) const
{
    return eStep(data);
}

/* ---- Log-likelihood ---- */

double GaussianMixture29::logLikelihood(
    const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double sum = 0.0;
        for (int k = 0; k < m_K; ++k)
            sum += m_components[k].weight * gaussianPdf(data[i], k);
        if (sum > 1e-300) ll += qLn(sum);
    }
    return ll;
}

/* ---- Accessors ---- */

QVector<GaussianMixture29::Component> GaussianMixture29::components() const
{
    return m_components;
}

/* ---- Reset ---- */

void GaussianMixture29::resetStatistics()
{
    m_components.clear();
    m_suffStatN.clear();
    m_suffStatMean.clear();
    m_suffStatVar.clear();
    m_n = 0;
    m_dim = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
