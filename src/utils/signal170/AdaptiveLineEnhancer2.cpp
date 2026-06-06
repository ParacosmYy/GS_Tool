/**
 * @file AdaptiveLineEnhancer2.cpp
 * @brief AdaptiveLineEnhancer2 实现
 *
 * 实现自适应线增强器：LMS/NLMS/RLS自适应滤波、延迟线解相关、干扰消除。
 */

#include "utils/signal170/AdaptiveLineEnhancer2.h"

#include <QElapsedTimer>
#include <QtMath>

AdaptiveLineEnhancer2::AdaptiveLineEnhancer2(QObject *parent)
    : QObject(parent)
{
    reset();
}

AdaptiveLineEnhancer2::~AdaptiveLineEnhancer2() = default;

void AdaptiveLineEnhancer2::setAlgorithm(Algorithm algo) { m_algo = algo; }
void AdaptiveLineEnhancer2::setFilterOrder(int order) { m_order = qMax(1, order); }
void AdaptiveLineEnhancer2::setStepSize(double mu) { m_mu = qBound(1e-6, mu, 1.0); }
void AdaptiveLineEnhancer2::setDelayLine(int delay) { m_delay = qMax(1, delay); }
void AdaptiveLineEnhancer2::setForgettingFactor(double lambda) { m_lambda = qBound(0.9, lambda, 1.0); }

void AdaptiveLineEnhancer2::reset()
{
    m_weights.assign(m_order, 0.0);
    m_delayBuffer.assign(m_delay + m_order, 0.0);
    m_delayIdx = 0;
    m_errorHistory.clear();

    /* Initialize RLS inverse correlation matrix */
    double delta = 100.0;
    m_P.assign(m_order, QVector<double>(m_order, 0.0));
    for (int i = 0; i < m_order; ++i)
        m_P[i][i] = delta;
}

double AdaptiveLineEnhancer2::lmsStep(double desired, double reference)
{
    /* Compute filter output */
    double output = 0.0;
    for (int i = 0; i < m_order; ++i)
        output += m_weights[i] * m_delayBuffer[m_delayIdx + i];

    /* Error = desired - output */
    double error = desired - output;

    /* Update weights: w += mu * error * x */
    for (int i = 0; i < m_order; ++i)
        m_weights[i] += m_mu * error * m_delayBuffer[m_delayIdx + i];

    return error;
}

double AdaptiveLineEnhancer2::nlmsStep(double desired, double reference)
{
    /* Compute filter output */
    double output = 0.0;
    double xPower = 0.0;
    for (int i = 0; i < m_order; ++i) {
        output += m_weights[i] * m_delayBuffer[m_delayIdx + i];
        xPower += m_delayBuffer[m_delayIdx + i] * m_delayBuffer[m_delayIdx + i];
    }

    double error = desired - output;

    /* Normalized step: w += (mu / (eps + ||x||^2)) * error * x */
    double normStep = m_mu / (1e-10 + xPower);
    for (int i = 0; i < m_order; ++i)
        m_weights[i] += normStep * error * m_delayBuffer[m_delayIdx + i];

    return error;
}

double AdaptiveLineEnhancer2::rlsStep(double desired, double reference)
{
    int N = m_order;

    /* Gain vector: k = P * x / (lambda + x^T * P * x) */
    QVector<double> Px(N, 0.0);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            Px[i] += m_P[i][j] * m_delayBuffer[m_delayIdx + j];

    double xPx = 0.0;
    for (int i = 0; i < N; ++i)
        xPx += m_delayBuffer[m_delayIdx + i] * Px[i];

    double denom = m_lambda + xPx;
    if (qAbs(denom) < 1e-30) denom = 1e-30;

    QVector<double> k(N);
    for (int i = 0; i < N; ++i)
        k[i] = Px[i] / denom;

    /* Filter output */
    double output = 0.0;
    for (int i = 0; i < N; ++i)
        output += m_weights[i] * m_delayBuffer[m_delayIdx + i];

    double error = desired - output;

    /* Update weights: w += k * error */
    for (int i = 0; i < N; ++i)
        m_weights[i] += k[i] * error;

    /* Update P: P = (1/lambda) * (P - k * x^T * P) */
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            m_P[i][j] = (m_P[i][j] - k[i] * Px[j]) / m_lambda;

    return error;
}

QVector<double> AdaptiveLineEnhancer2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    /* Re-initialize if order changed */
    if (m_weights.size() != m_order) reset();

    QVector<double> enhanced(n, 0.0);
    m_errorHistory.resize(n);

    for (int t = 0; t < n; ++t) {
        /* Push input into delay buffer */
        m_delayBuffer[m_delayIdx] = input[t];
        m_delayBuffer[m_delayIdx + m_delay] = input[t];

        /* Also shift the reference buffer */
        for (int i = m_delay + m_order - 1; i > m_delay; --i)
            m_delayBuffer[m_delayIdx + i] = m_delayBuffer[m_delayIdx + i - 1];
        m_delayBuffer[m_delayIdx + m_delay] = input[t];

        double error = 0.0;
        switch (m_algo) {
        case LMS:
            error = lmsStep(input[t], input[t]);
            break;
        case NormalizedLMS:
            error = nlmsStep(input[t], input[t]);
            break;
        case RLS:
            error = rlsStep(input[t], input[t]);
            break;
        }

        /* Enhanced signal = input - error (error is noise estimate) */
        enhanced[t] = input[t] - error;
        m_errorHistory[t] = error;
    }

    /* Compute MSE */
    double mse = 0.0;
    for (int i = 0; i < n; ++i)
        mse += m_errorHistory[i] * m_errorHistory[i];
    mse /= qMax(1, n);

    m_stats.totalSamples += n;
    m_stats.lastMse = mse;
    m_stats.filterOrder = m_order;
    m_timeSum += timer.elapsed() * 1000.0; /* convert to μs */
    m_stats.avgProcessingTimeUs = (m_stats.totalSamples > 0)
        ? m_timeSum / m_stats.totalSamples : 0.0;

    emit processingCompleted(n, mse);
    return enhanced;
}

QVector<double> AdaptiveLineEnhancer2::filterCoefficients() const { return m_weights; }
QVector<double> AdaptiveLineEnhancer2::errorSignal() const { return m_errorHistory; }

void AdaptiveLineEnhancer2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
