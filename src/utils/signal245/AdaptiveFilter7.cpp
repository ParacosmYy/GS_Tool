/**
 * @file AdaptiveFilter7.cpp
 * @brief AdaptiveFilter7 实现
 *
 * 实现自适应滤波器：归一化LMS与梯度估计变步长收敛跟踪。
 */

#include "utils/signal245/AdaptiveFilter7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AdaptiveFilter7::AdaptiveFilter7(QObject *parent) : QObject(parent) {}
AdaptiveFilter7::~AdaptiveFilter7() = default;

/* ---- Configuration ---- */

void AdaptiveFilter7::setFilterOrder(int order)
{
    m_order = qMax(1, order);
    m_weights.resize(m_order, 0.0);
    m_delayLine.resize(m_order, 0.0);
    m_delayPos = 0;
}

void AdaptiveFilter7::setInitialMu(double mu) { m_mu = qBound(m_muMin, mu, m_muMax); }
void AdaptiveFilter7::setMuMin(double mu) { m_muMin = qMax(1e-6, mu); }
void AdaptiveFilter7::setMuMax(double mu) { m_muMax = qMin(2.0, mu); }
void AdaptiveFilter7::setMuAdaptRate(double rate) { m_muAdaptRate = qBound(1e-6, rate, 1.0); }

/* ---- Push sample into circular delay line ---- */

void AdaptiveFilter7::pushSample(double sample)
{
    m_delayLine[m_delayPos] = sample;
    m_delayPos = (m_delayPos + 1) % m_order;
}

/* ---- Dot product of delay line with weights ---- */

double AdaptiveFilter7::dotProduct() const
{
    double sum = 0.0;
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_delayPos - 1 - i + m_order) % m_order;
        sum += m_weights[i] * m_delayLine[idx];
    }
    return sum;
}

/* ---- Update step-size using gradient estimation ---- */

void AdaptiveFilter7::updateStepSize(double error)
{
    // Variable step-size via gradient estimation:
    // Use the correlation of consecutive errors to adjust mu
    // If errors have same sign -> not converging -> increase mu
    // If errors alternate sign -> oscillating -> decrease mu
    double grad = error * m_prevError;
    m_gradEstimate = 0.9 * m_gradEstimate + 0.1 * grad;

    // Adjust mu based on gradient estimate
    if (m_gradEstimate > 0) {
        // Errors correlated: slow convergence, increase mu
        m_mu += m_muAdaptRate * m_mu;
    } else {
        // Errors anti-correlated: oscillation, decrease mu
        m_mu -= m_muAdaptRate * m_mu;
    }
    m_mu = qBound(m_muMin, m_mu, m_muMax);
    m_prevError = error;
}

/* ---- NLMS weight update ---- */

void AdaptiveFilter7::updateWeights(double error)
{
    // Normalized LMS: w += mu * error * x / (x'x + eps)
    double inputPower = 0.0;
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_delayPos - 1 - i + m_order) % m_order;
        inputPower += m_delayLine[idx] * m_delayLine[idx];
    }

    double norm = inputPower + 1e-10;  // Regularization
    double factor = m_mu * error / norm;

    for (int i = 0; i < m_order; ++i) {
        int idx = (m_delayPos - 1 - i + m_order) % m_order;
        m_weights[i] += factor * m_delayLine[idx];
    }
}

/* ---- Process ---- */

QVector<double> AdaptiveFilter7::process(const QVector<double>& input,
                                          const QVector<double>& desired)
{
    QElapsedTimer timer;
    timer.start();

    // Ensure initialized
    if (m_weights.size() != m_order) {
        m_weights.resize(m_order, 0.0);
        m_delayLine.resize(m_order, 0.0);
    }

    int n = qMin(input.size(), desired.size());
    QVector<double> output(n);
    m_errorHistory.resize(n);

    double errorPowerSum = 0.0;

    for (int i = 0; i < n; ++i) {
        // Push input sample
        pushSample(input[i]);

        // Compute filter output
        output[i] = dotProduct();

        // Compute error
        double err = desired[i] - output[i];
        m_errorHistory[i] = err;
        errorPowerSum += err * err;

        // Update step-size via gradient estimation
        updateStepSize(err);

        // Update weights via NLMS
        updateWeights(err);
    }

    m_stats.filterOrder = m_order;
    m_stats.numSamples = n;
    m_stats.finalMu = m_mu;
    m_stats.finalErrorPower = (n > 0) ? errorPowerSum / n : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return output;
}

/* ---- Accessors ---- */

QVector<double> AdaptiveFilter7::coefficients() const { return m_weights; }
QVector<double> AdaptiveFilter7::errorHistory() const { return m_errorHistory; }

/* ---- Reset ---- */

void AdaptiveFilter7::resetStatistics()
{
    m_weights.fill(0.0);
    m_delayLine.fill(0.0);
    m_delayPos = 0;
    m_errorHistory.clear();
    m_prevError = 0.0;
    m_gradEstimate = 0.0;
    m_mu = qBound(m_muMin, 0.1, m_muMax);
    m_stats = Stats{}; m_timeSum = 0.0;
}
