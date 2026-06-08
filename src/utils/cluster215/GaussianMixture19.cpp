/**
 * @file GaussianMixture19.cpp
 * @brief GaussianMixture19 实现
 *
 * 实现高斯混合模型：确定性退火EM、温度调度、BIC模型选择。
 */

#include "utils/cluster215/GaussianMixture19.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture19::GaussianMixture19(QObject *parent) : QObject(parent) {}
GaussianMixture19::~GaussianMixture19() = default;

/* ---- Configuration ---- */

void GaussianMixture19::setParameters(int components, int maxIter,
                                        double initTemp, double coolingRate)
{
    m_components = qMax(2, components);
    m_maxIter = qMax(10, maxIter);
    m_initTemp = qMax(1.0, initTemp);
    m_coolingRate = qBound(0.5, coolingRate, 0.999);
}

/* ---- Initialize via k-means++ ---- */

void GaussianMixture19::initialize(const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_dim = data[0].size();
    m_means.resize(m_components);
    m_covariances.resize(m_components);
    m_weights.resize(m_components, 1.0 / m_components);

    // First center: random
    int first = 0;
    m_means[0] = data[first];

    // k-means++ seeding for remaining centers
    QVector<double> minDist(n, std::numeric_limits<double>::max());
    for (int k = 1; k < m_components; ++k) {
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int j = 0; j < m_dim; ++j)
                d += qPow(data[i][j] - m_means[k - 1][j], 2);
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }
        // Weighted random selection
        double threshold = totalDist * 0.5;
        double cumSum = 0.0;
        int selected = k;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= threshold) { selected = i; break; }
        }
        m_means[k] = data[selected];
    }

    // Initialize covariances to identity scaled by data variance
    for (int k = 0; k < m_components; ++k) {
        m_covariances[k].resize(m_dim);
        for (int d = 0; d < m_dim; ++d) {
            m_covariances[k][d].resize(m_dim, 0.0);
            m_covariances[k][d][d] = 1.0;
        }
    }
}

/* ---- Gaussian PDF ---- */

double GaussianMixture19::gaussianPdf(const QVector<double>& x, int comp) const
{
    const auto& mu = m_means[comp];
    const auto& cov = m_covariances[comp];

    // Compute det and inverse of covariance
    double det = 1.0;
    for (int d = 0; d < m_dim; ++d)
        det *= qMax(cov[d][d], 1e-12);

    double mahal = 0.0;
    for (int d = 0; d < m_dim; ++d) {
        double diff = x[d] - mu[d];
        mahal += diff * diff / qMax(cov[d][d], 1e-12);
    }

    double norm = 1.0 / qSqrt(qPow(2.0 * M_PI, m_dim) * qMax(det, 1e-30));
    return norm * qExp(-0.5 * mahal);
}

/* ---- E-step with temperature ---- */

void GaussianMixture19::eStep(const QVector<QVector<double>>& data,
                                double temperature)
{
    int n = data.size();
    m_resp.resize(n);

    for (int i = 0; i < n; ++i) {
        m_resp[i].resize(m_components);
        double sum = 0.0;
        for (int k = 0; k < m_components; ++k) {
            double p = m_weights[k] * gaussianPdf(data[i], k);
            m_resp[i][k] = qPow(qMax(p, 1e-300), 1.0 / temperature);
            sum += m_resp[i][k];
        }
        if (sum > 0.0) {
            for (int k = 0; k < m_components; ++k)
                m_resp[i][k] /= sum;
        }
    }
}

/* ---- M-step ---- */

void GaussianMixture19::mStep(const QVector<QVector<double>>& data)
{
    int n = data.size();

    for (int k = 0; k < m_components; ++k) {
        double nk = 0.0;
        for (int i = 0; i < n; ++i) nk += m_resp[i][k];
        nk = qMax(nk, 1e-10);

        // Update weight
        m_weights[k] = nk / n;

        // Update mean
        for (int d = 0; d < m_dim; ++d) {
            double sum = 0.0;
            for (int i = 0; i < n; ++i)
                sum += m_resp[i][k] * data[i][d];
            m_means[k][d] = sum / nk;
        }

        // Update covariance (diagonal approximation)
        for (int d = 0; d < m_dim; ++d) {
            double var = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = data[i][d] - m_means[k][d];
                var += m_resp[i][k] * diff * diff;
            }
            for (int d2 = 0; d2 < m_dim; ++d2)
                m_covariances[k][d][d2] = 0.0;
            m_covariances[k][d][d] = qMax(var / nk, 1e-6);
        }
    }
}

/* ---- Compute BIC ---- */

double GaussianMixture19::computeBIC(const QVector<QVector<double>>& data) const
{
    int n = data.size();
    double ll = logLikelihood(data);
    // params = K-1 weights + K*D means + K*D variances
    int params = (m_components - 1) + m_components * m_dim * 2;
    return -2.0 * ll + params * qLn(n);
}

/* ---- Log-likelihood ---- */

double GaussianMixture19::logLikelihood(
    const QVector<QVector<double>>& data) const
{
    double ll = 0.0;
    for (int i = 0; i < data.size(); ++i) {
        double p = 0.0;
        for (int k = 0; k < m_components; ++k)
            p += m_weights[k] * gaussianPdf(data[i], k);
        ll += qLn(qMax(p, 1e-300));
    }
    return ll;
}

/* ---- Fit with deterministic annealing ---- */

void GaussianMixture19::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < m_components) return;
    initialize(data);

    double temperature = m_initTemp;
    int iterations = 0;

    // Annealing phase: gradually cool temperature to 1.0
    while (temperature > 1.0 + 1e-6 && iterations < m_maxIter) {
        eStep(data, temperature);
        mStep(data);
        temperature *= m_coolingRate;
        iterations++;
    }

    // Final EM at temperature = 1.0
    for (int it = 0; it < m_maxIter - iterations; ++it) {
        eStep(data, 1.0);
        mStep(data);
    }
    iterations += m_maxIter - iterations;

    m_stats.numSamples = data.size();
    m_stats.numComponents = m_components;
    m_stats.dimensions = m_dim;
    m_stats.logLikelihood = logLikelihood(data);
    m_stats.bic = computeBIC(data);
    m_stats.finalTemperature = temperature;
    m_stats.emIterations = iterations;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fittingCompleted(m_components, m_stats.logLikelihood, timer.elapsed());
}

/* ---- Predict ---- */

QVector<int> GaussianMixture19::predict(
    const QVector<QVector<double>>& data) const
{
    QVector<int> labels(data.size());
    for (int i = 0; i < data.size(); ++i) {
        int best = 0;
        double bestP = -1.0;
        for (int k = 0; k < m_components; ++k) {
            double p = m_weights[k] * gaussianPdf(data[i], k);
            if (p > bestP) { bestP = p; best = k; }
        }
        labels[i] = best;
    }
    return labels;
}

/* ---- Accessors ---- */

QVector<QVector<double>> GaussianMixture19::responsibilities() const
{
    return m_resp;
}

QVector<QVector<double>> GaussianMixture19::means() const { return m_means; }

/* ---- Reset ---- */

void GaussianMixture19::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_means.clear();
    m_covariances.clear();
    m_weights.clear();
    m_resp.clear();
    m_dim = 0;
}
