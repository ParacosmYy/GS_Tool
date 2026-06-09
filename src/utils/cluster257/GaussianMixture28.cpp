/**
 * @file GaussianMixture28.cpp
 * @brief GaussianMixture28 实现
 *
 * 实现高斯混合模型：在线序列EM与遗忘因子流式自适应。
 */

#include "utils/cluster257/GaussianMixture28.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

GaussianMixture28::GaussianMixture28(QObject *parent)
    : QObject(parent) {}
GaussianMixture28::~GaussianMixture28() = default;

/* ---- Configuration ---- */

void GaussianMixture28::setNumComponents(int k) { m_k = qMax(1, k); }
void GaussianMixture28::setForgettingFactor(double alpha) { m_alpha = qBound(0.01, alpha, 1.0); }

/* ---- Gaussian PDF (diagonal covariance) ---- */

double GaussianMixture28::gaussianPdf(const QVector<double>& x, const Component& c) const
{
    double logP = logGaussianPdf(x, c);
    return qExp(logP);
}

double GaussianMixture28::logGaussianPdf(const QVector<double>& x, const Component& c) const
{
    int d = x.size();
    double logDet = 0.0;
    double mahal = 0.0;
    for (int i = 0; i < d; ++i) {
        double var = qMax(c.variance[i], 1e-12);
        logDet += qLn(var);
        double diff = x[i] - c.mean[i];
        mahal += diff * diff / var;
    }
    return -0.5 * (d * qLn(2.0 * M_PI) + logDet + mahal);
}

/* ---- K-means++ seeding ---- */

void GaussianMixture28::kMeansPPSeeding(const QVector<QVector<double>>& data)
{
    int n = data.size();
    if (n == 0) return;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, n - 1);

    m_components.resize(m_k);
    int firstIdx = uni(rng);
    for (int d = 0; d < m_dims; ++d)
        m_components[0].mean.append(data[firstIdx][d]);
    m_components[0].variance.resize(m_dims, 1.0);
    m_components[0].weight = 1.0 / m_k;

    QVector<double> minDist(n, std::numeric_limits<double>::max());
    for (int k = 1; k < m_k; ++k) {
        // Compute distances to nearest center
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int dim = 0; dim < m_dims; ++dim) {
                double diff = data[i][dim] - m_components[k - 1].mean[dim];
                d += diff * diff;
            }
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }
        // Weighted random selection
        std::uniform_real_distribution<double> dist(0.0, qMax(totalDist, 1e-12));
        double threshold = dist(rng);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        for (int d = 0; d < m_dims; ++d)
            m_components[k].mean.append(data[chosen][d]);
        m_components[k].variance.resize(m_dims, 1.0);
        m_components[k].weight = 1.0 / m_k;
    }
    // Initialize remaining fields
    for (int k = 0; k < m_k; ++k)
        m_components[k].logWeight = qLn(m_components[k].weight);
}

/* ---- Recompute parameters ---- */

void GaussianMixture28::recomputeParameters()
{
    for (int k = 0; k < m_k; ++k) {
        Component& c = m_components[k];
        c.logWeight = (c.weight > 0.0) ? qLn(c.weight) : -std::numeric_limits<double>::infinity();
        for (int d = 0; d < m_dims; ++d)
            c.variance[d] = qMax(c.variance[d], 1e-12);
    }
}

/* ---- Initialize from batch data ---- */

bool GaussianMixture28::initialize(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_k) return false;
    m_dims = data[0].size();
    m_sampleCount = data.size();

    kMeansPPSeeding(data);
    // Run a few EM iterations to refine
    batchEM(data, 10);

    double elapsed = timer.elapsed();
    m_stats.numComponents = m_k;
    m_stats.numSamples = m_sampleCount;
    m_stats.numDimensions = m_dims;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return true;
}

/* ---- Batch EM ---- */

void GaussianMixture28::batchEM(const QVector<QVector<double>>& data, int maxIter)
{
    int n = data.size();
    if (n == 0) return;

    for (int iter = 0; iter < maxIter; ++iter) {
        // E-step: compute responsibilities
        QVector<QVector<double>> resp(n, QVector<double>(m_k, 0.0));
        double totalLL = 0.0;
        for (int i = 0; i < n; ++i) {
            double sumExp = 0.0;
            double maxLog = -std::numeric_limits<double>::infinity();
            for (int k = 0; k < m_k; ++k) {
                resp[i][k] = m_components[k].logWeight + logGaussianPdf(data[i], m_components[k]);
                if (resp[i][k] > maxLog) maxLog = resp[i][k];
            }
            for (int k = 0; k < m_k; ++k)
                sumExp += qExp(resp[i][k] - maxLog);
            double logNorm = maxLog + qLn(sumExp);
            totalLL += logNorm;
            for (int k = 0; k < m_k; ++k)
                resp[i][k] = qExp(resp[i][k] - logNorm);
        }
        m_logLikelihood = totalLL;

        // M-step: update parameters
        for (int k = 0; k < m_k; ++k) {
            double nk = 0.0;
            for (int i = 0; i < n; ++i) nk += resp[i][k];
            if (nk < 1e-12) continue;

            m_components[k].weight = nk / n;
            for (int d = 0; d < m_dims; ++d) {
                double meanSum = 0.0;
                for (int i = 0; i < n; ++i) meanSum += resp[i][k] * data[i][d];
                double newMean = meanSum / nk;

                double varSum = 0.0;
                for (int i = 0; i < n; ++i) {
                    double diff = data[i][d] - newMean;
                    varSum += resp[i][k] * diff * diff;
                }
                m_components[k].mean[d] = newMean;
                m_components[k].variance[d] = varSum / nk;
            }
        }
        recomputeParameters();
    }
}

/* ---- Sufficient statistics update ---- */

void GaussianMixture28::updateSufficientStats(const QVector<double>& x, const QVector<double>& resp)
{
    double respSum = 0.0;
    for (int k = 0; k < m_k; ++k) respSum += resp[k];

    for (int k = 0; k < m_k; ++k) {
        double r = resp[k];
        double newWeight = m_alpha * m_components[k].weight + (1.0 - m_alpha) * (r / qMax(respSum, 1e-12));
        for (int d = 0; d < m_dims; ++d) {
            double oldMean = m_components[k].mean[d];
            double newMean = m_alpha * oldMean + (1.0 - m_alpha) * r * x[d] / qMax(newWeight, 1e-12);
            double diff = x[d] - newMean;
            double oldVar = m_components[k].variance[d];
            m_components[k].variance[d] = m_alpha * oldVar + (1.0 - m_alpha) * r * diff * diff / qMax(newWeight, 1e-12);
            m_components[k].mean[d] = newMean;
        }
        m_components[k].weight = newWeight;
    }
    recomputeParameters();
}

/* ---- Online update ---- */

void GaussianMixture28::update(const QVector<double>& sample)
{
    QElapsedTimer timer;
    timer.start();

    if (m_components.isEmpty() || sample.size() != m_dims) return;
    QVector<double> resp = responsibilities(sample);
    updateSufficientStats(sample, resp);
    m_sampleCount++;

    double elapsed = timer.elapsed();
    m_stats.numSamples = m_sampleCount;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit modelUpdated(m_k, m_sampleCount, m_logLikelihood, elapsed);
}

/* ---- Responsibilities ---- */

QVector<double> GaussianMixture28::responsibilities(const QVector<double>& sample) const
{
    QVector<double> resp(m_k, 0.0);
    double sumExp = 0.0;
    double maxLog = -std::numeric_limits<double>::infinity();
    for (int k = 0; k < m_k; ++k) {
        resp[k] = m_components[k].logWeight + logGaussianPdf(sample, m_components[k]);
        if (resp[k] > maxLog) maxLog = resp[k];
    }
    for (int k = 0; k < m_k; ++k)
        sumExp += qExp(resp[k] - maxLog);
    for (int k = 0; k < m_k; ++k)
        resp[k] = qExp(resp[k] - maxLog - qLn(qMax(sumExp, 1e-12)));
    return resp;
}

/* ---- Predict ---- */

int GaussianMixture28::predict(const QVector<double>& sample) const
{
    QVector<double> resp = responsibilities(sample);
    int best = 0;
    for (int k = 1; k < m_k; ++k)
        if (resp[k] > resp[best]) best = k;
    return best;
}

/* ---- Accessors ---- */

QVector<GaussianMixture28::Component> GaussianMixture28::components() const { return m_components; }

/* ---- Reset ---- */

void GaussianMixture28::resetStatistics()
{
    m_components.clear();
    m_sampleCount = 0;
    m_logLikelihood = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
