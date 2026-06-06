/**
 * @file AdaptiveFilter3.cpp
 * @brief AdaptiveFilter3 实现
 *
 * 实现自适应滤波器：LMS/NLMS/RLS算法、可变步长、收敛监控。
 */

#include "utils/signal183/AdaptiveFilter3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AdaptiveFilter3::AdaptiveFilter3(QObject *parent) : QObject(parent)
{
    resetFilter();
}
AdaptiveFilter3::~AdaptiveFilter3() = default;

/* ---- Configuration ---- */

void AdaptiveFilter3::setFilterOrder(int order)
{
    m_order = qMax(1, order);
    resetFilter();
}
void AdaptiveFilter3::setStepSize(double mu) { m_mu = qBound(1e-8, mu, 1.0); }
void AdaptiveFilter3::setAlgorithm(Algorithm algo) { m_algo = algo; }
void AdaptiveFilter3::setRLSLambda(double lambda) { m_lambda = qBound(0.9, lambda, 1.0); }
void AdaptiveFilter3::setRLSDelta(double delta) { m_delta = qMax(1e-8, delta); }
void AdaptiveFilter3::setVariableStepSize(bool enabled) { m_variableStep = enabled; }
void AdaptiveFilter3::setStepSizeRange(double muMin, double muMax)
{
    m_muMin = qMax(1e-10, muMin);
    m_muMax = qMax(m_muMin, muMax);
}

/* ---- Reset ---- */

void AdaptiveFilter3::resetFilter()
{
    m_weights.resize(m_order, 0.0);
    m_delayLine.resize(m_order, 0.0);
    m_errorHistory.clear();
    m_prevError = 0.0;

    // Initialize RLS inverse correlation matrix
    m_P.resize(m_order);
    for (int i = 0; i < m_order; ++i) {
        m_P[i].resize(m_order, 0.0);
        m_P[i][i] = 1.0 / m_delta;
    }
}

/* ---- Delay line ---- */

void AdaptiveFilter3::pushSample(double sample)
{
    // Shift right and insert at front
    for (int i = m_order - 1; i > 0; --i)
        m_delayLine[i] = m_delayLine[i - 1];
    m_delayLine[0] = sample;
}

/* ---- Compute output ---- */

double AdaptiveFilter3::computeOutput() const
{
    double y = 0.0;
    for (int i = 0; i < m_order; ++i)
        y += m_weights[i] * m_delayLine[i];
    return y;
}

/* ---- LMS ---- */

double AdaptiveFilter3::updateLMS(double input, double desired)
{
    pushSample(input);
    double output = computeOutput();
    double error = desired - output;

    // Weight update: w += mu * error * x
    for (int i = 0; i < m_order; ++i)
        m_weights[i] += m_mu * error * m_delayLine[i];

    return error;
}

/* ---- NLMS ---- */

double AdaptiveFilter3::updateNLMS(double input, double desired)
{
    pushSample(input);
    double output = computeOutput();
    double error = desired - output;

    // Normalized power of input
    double power = 0.0;
    for (int i = 0; i < m_order; ++i)
        power += m_delayLine[i] * m_delayLine[i];
    power += 1e-10; // Regularization

    double muEff = m_mu / power;
    for (int i = 0; i < m_order; ++i)
        m_weights[i] += muEff * error * m_delayLine[i];

    return error;
}

/* ---- RLS ---- */

double AdaptiveFilter3::updateRLS(double input, double desired)
{
    pushSample(input);
    double output = computeOutput();
    double error = desired - output;

    // Gain vector: k = P * x / (lambda + x^T * P * x)
    QVector<double> Px(m_order, 0.0);
    for (int i = 0; i < m_order; ++i)
        for (int j = 0; j < m_order; ++j)
            Px[i] += m_P[i][j] * m_delayLine[j];

    double xPx = 0.0;
    for (int i = 0; i < m_order; ++i)
        xPx += m_delayLine[i] * Px[i];

    double denom = m_lambda + xPx;
    if (qFuzzyIsNull(denom)) denom = 1e-30;

    QVector<double> k(m_order);
    for (int i = 0; i < m_order; ++i)
        k[i] = Px[i] / denom;

    // Weight update: w += k * error
    for (int i = 0; i < m_order; ++i)
        m_weights[i] += k[i] * error;

    // P update: P = (P - k * x^T * P) / lambda
    for (int i = 0; i < m_order; ++i) {
        for (int j = 0; j < m_order; ++j) {
            m_P[i][j] = (m_P[i][j] - k[i] * Px[j]) / m_lambda;
        }
    }

    return error;
}

/* ---- Variable step size ---- */

void AdaptiveFilter3::adaptStepSize(double error)
{
    // Gradient-based step size adaptation
    double gradient = error * m_prevError;
    m_prevError = error;

    if (gradient > 0.0)
        m_mu = qMin(m_mu * 1.01, m_muMax);
    else
        m_mu = qMax(m_mu * 0.99, m_muMin);
}

/* ---- Single sample ---- */

double AdaptiveFilter3::processSample(double input, double desired)
{
    double error = 0.0;
    switch (m_algo) {
    case LMS:  error = updateLMS(input, desired); break;
    case NLMS: error = updateNLMS(input, desired); break;
    case RLS:  error = updateRLS(input, desired); break;
    }

    if (m_variableStep && m_algo != RLS)
        adaptStepSize(error);

    m_errorHistory.append(error);
    m_stats.totalSamples++;

    return computeOutput();
}

/* ---- Batch ---- */

QVector<double> AdaptiveFilter3::processBatch(const QVector<double>& input,
                                                const QVector<double>& desired)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), desired.size());
    QVector<double> output(n);

    for (int i = 0; i < n; ++i)
        output[i] = processSample(input[i], desired[i]);

    double finalErr = m_errorHistory.isEmpty() ? 0.0 :
                          qAbs(m_errorHistory.last());

    m_stats.filterOrder = m_order;
    m_stats.numIterations = n;
    m_stats.finalError = finalErr;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples);

    emit processingCompleted(n, finalErr);
    return output;
}

/* ---- Accessors ---- */

QVector<double> AdaptiveFilter3::coefficients() const { return m_weights; }
QVector<double> AdaptiveFilter3::errorHistory() const { return m_errorHistory; }

/* ---- Reset statistics ---- */

void AdaptiveFilter3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_errorHistory.clear();
}
