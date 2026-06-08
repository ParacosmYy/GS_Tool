/**
 * @file GaussianMixture21.cpp
 * @brief GaussianMixture21 实现
 *
 * 实现高斯混合模型：蒙特卡洛EM算法与重要性采样E步近似。
 */

#include "utils/cluster226/GaussianMixture21.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussianMixture21::GaussianMixture21(QObject *parent) : QObject(parent) {}
GaussianMixture21::~GaussianMixture21() = default;

/* ---- Configuration ---- */

void GaussianMixture21::setParameters(int numComponents, int mcSamples)
{
    m_numComponents = qMax(2, numComponents);
    m_mcSamples = qMax(50, mcSamples);
}

/* ---- LCG random generator ---- */

double GaussianMixture21::randUniform()
{
    m_seed = (m_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return static_cast<double>(m_seed) / 0x7FFFFFFF;
}

double GaussianMixture21::randNormal()
{
    // Box-Muller transform
    double u1 = qMax(1e-15, randUniform());
    double u2 = randUniform();
    return qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
}

/* ---- Gaussian PDF ---- */

double GaussianMixture21::gaussianPdf(double x, double mean, double var) const
{
    if (var <= 0) var = 1e-10;
    double diff = x - mean;
    return (1.0 / qSqrt(2.0 * M_PI * var)) * qExp(-0.5 * diff * diff / var);
}

/* ---- Monte Carlo E-step with importance sampling ---- */

void GaussianMixture21::monteCarloEStep(
    QVector<QVector<double>>& responsibilities)
{
    int n = m_data.size();
    int k = m_components.size();

    for (int i = 0; i < n; ++i) {
        double x = m_data[i];

        // Standard E-step: compute weighted likelihoods
        QVector<double> weighted(k);
        double total = 0.0;
        for (int c = 0; c < k; ++c) {
            weighted[c] = m_components[c].weight *
                gaussianPdf(x, m_components[c].mean[0], m_components[c].variance);
            total += weighted[c];
        }

        if (total > 1e-300) {
            for (int c = 0; c < k; ++c)
                responsibilities[i][c] = weighted[c] / total;
        } else {
            // Fallback: importance sampling approximation
            double qTotal = 0.0;
            QVector<double> isWeights(k, 0.0);
            for (int s = 0; s < m_mcSamples; ++s) {
                int comp = static_cast<int>(randUniform() * k) % k;
                double sample = m_components[comp].mean[0] +
                    qSqrt(m_components[comp].variance) * randNormal();
                double pSample = gaussianPdf(sample,
                    m_components[comp].mean[0], m_components[comp].variance);
                double qSample = gaussianPdf(x, sample,
                    m_components[comp].variance);
                double ratio = (qSample > 0 && pSample > 0)
                    ? gaussianPdf(x, m_components[comp].mean[0],
                                  m_components[comp].variance) / pSample
                    : 1.0 / k;
                isWeights[comp] += ratio * m_components[comp].weight;
                qTotal += ratio * m_components[comp].weight;
            }
            for (int c = 0; c < k; ++c)
                responsibilities[i][c] = (qTotal > 0)
                    ? isWeights[c] / qTotal : 1.0 / k;
        }
    }
}

/* ---- M-step ---- */

void GaussianMixture21::mStep(
    const QVector<QVector<double>>& responsibilities)
{
    int n = m_data.size();
    int k = m_components.size();

    for (int c = 0; c < k; ++c) {
        double nk = 0.0;
        double meanSum = 0.0;
        double varSum = 0.0;

        for (int i = 0; i < n; ++i) {
            double r = responsibilities[i][c];
            nk += r;
            meanSum += r * m_data[i];
        }

        if (nk > 1e-10) {
            m_components[c].mean[0] = meanSum / nk;
            for (int i = 0; i < n; ++i) {
                double diff = m_data[i] - m_components[c].mean[0];
                varSum += responsibilities[i][c] * diff * diff;
            }
            m_components[c].variance = qMax(1e-10, varSum / nk);
            m_components[c].weight = nk / n;
        }
    }
}

/* ---- Log-likelihood ---- */

double GaussianMixture21::computeLogLikelihood() const
{
    double ll = 0.0;
    for (int i = 0; i < m_data.size(); ++i) {
        double p = 0.0;
        for (const auto& comp : m_components)
            p += comp.weight * gaussianPdf(m_data[i], comp.mean[0], comp.variance);
        ll += qLn(qMax(1e-300, p));
    }
    return ll;
}

/* ---- Fit ---- */

bool GaussianMixture21::fit(const QVector<double>& data,
                              int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < m_numComponents * 2) return false;

    m_data = data;
    int k = m_numComponents;

    // Initialize components via uniform spacing
    double dMin = *std::min_element(data.begin(), data.end());
    double dMax = *std::max_element(data.begin(), data.end());
    double range = qMax(1e-10, dMax - dMin);

    m_components.resize(k);
    for (int c = 0; c < k; ++c) {
        m_components[c].weight = 1.0 / k;
        m_components[c].mean = QVector<double>(1, dMin + range * (c + 0.5) / k);
        m_components[c].variance = range * range / (k * k);
    }

    // EM iterations
    double prevLL = -std::numeric_limits<double>::max();
    QVector<QVector<double>> resp(n, QVector<double>(k, 0.0));

    int iter = 0;
    for (iter = 0; iter < maxIter; ++iter) {
        monteCarloEStep(resp);
        mStep(resp);

        double ll = computeLogLikelihood();
        emit iterationProgress(iter, ll);

        if (qAbs(ll - prevLL) < tol) break;
        prevLL = ll;
    }

    m_stats.numComponents = k;
    m_stats.numSamples = n;
    m_stats.emIterations = iter;
    m_stats.logLikelihood = computeLogLikelihood();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit fitCompleted(iter, m_stats.logLikelihood, timer.elapsed());
    return true;
}

/* ---- Predict posterior ---- */

QVector<double> GaussianMixture21::predict(double x) const
{
    int k = m_components.size();
    QVector<double> posterior(k);
    double total = 0.0;
    for (int c = 0; c < k; ++c) {
        posterior[c] = m_components[c].weight *
            gaussianPdf(x, m_components[c].mean[0], m_components[c].variance);
        total += posterior[c];
    }
    if (total > 0)
        for (int c = 0; c < k; ++c)
            posterior[c] /= total;
    return posterior;
}

/* ---- Sample from mixture ---- */

QVector<double> GaussianMixture21::sample(int count) const
{
    QVector<double> samples;
    for (int i = 0; i < count; ++i) {
        double r = randUniform();
        double cumSum = 0.0;
        int chosen = 0;
        for (int c = 0; c < m_components.size(); ++c) {
            cumSum += m_components[c].weight;
            if (r <= cumSum) { chosen = c; break; }
        }
        double val = m_components[chosen].mean[0] +
            qSqrt(m_components[chosen].variance) * randNormal();
        samples.append(val);
    }
    return samples;
}

/* ---- Get components ---- */

QVector<GaussianMixture21::Component> GaussianMixture21::components() const
{
    return m_components;
}

/* ---- Reset ---- */

void GaussianMixture21::resetStatistics()
{
    m_components.clear();
    m_data.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
