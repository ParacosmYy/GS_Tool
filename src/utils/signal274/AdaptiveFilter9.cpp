/**
 * @file AdaptiveFilter9.cpp
 * @brief AdaptiveFilter9 实现
 *
 * 实现自适应滤波器：归一化LMS与梯度估计变步长时变系统辨识。
 */

#include "utils/signal274/AdaptiveFilter9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AdaptiveFilter9::AdaptiveFilter9(QObject *parent)
    : QObject(parent)
{
    m_weights.resize(m_order, 0.0);
    m_inputBuf.resize(m_order, 0.0);
    m_currentMu = m_mu;
}

AdaptiveFilter9::~AdaptiveFilter9() = default;

/* ---- Configuration ---- */

void AdaptiveFilter9::setFilterOrder(int order)
{
    m_order = qBound(1, order, 4096);
    m_weights.resize(m_order, 0.0);
    m_inputBuf.resize(m_order, 0.0);
    m_bufPos = 0;
}

void AdaptiveFilter9::setStepSize(double mu)
{
    m_mu = qBound(1e-8, mu, 2.0);
    m_currentMu = m_mu;
}

void AdaptiveFilter9::setRegularization(double eps)
{
    m_eps = qBound(1e-16, eps, 1.0);
}

void AdaptiveFilter9::setVariableStepSize(bool enabled, double muMin, double muMax, double nu)
{
    m_variableStep = enabled;
    m_muMin = qBound(1e-10, muMin, 1.0);
    m_muMax = qBound(m_muMin, muMax, 2.0);
    m_nu = qBound(1e-6, nu, 1.0);
}

/* ---- Delayed sample from circular buffer ---- */

double AdaptiveFilter9::getDelayedSample(int delay) const
{
    int idx = (m_bufPos - delay + m_order) % m_order;
    return m_inputBuf[idx];
}

/* ---- Compute filter output: y = w^T * x ---- */

double AdaptiveFilter9::computeOutput() const
{
    double y = 0.0;
    for (int i = 0; i < m_order; ++i)
        y += m_weights[i] * getDelayedSample(i);
    return y;
}

/* ---- NLMS weight update: w += mu * e * x / (||x||^2 + eps) ---- */

void AdaptiveFilter9::updateWeights(double error, double power)
{
    double normFactor = power + m_eps;
    double step = m_currentMu / normFactor;

    for (int i = 0; i < m_order; ++i)
        m_weights[i] += step * error * getDelayedSample(i);
}

/* ---- Variable step-size update via gradient estimation ---- */

void AdaptiveFilter9::updateStepSize(double error)
{
    // Gradient estimation: dJ/dmu ≈ e(n) * e(n-1)
    m_gradEst = m_nu * error * m_prevError + (1.0 - m_nu) * m_prevGrad;

    // Update step size
    m_currentMu += 0.01 * m_gradEst;
    m_currentMu = qBound(m_muMin, m_currentMu, m_muMax);

    m_prevError = error;
    m_prevGrad = m_gradEst;
}

/* ---- Main process ---- */

QVector<double> AdaptiveFilter9::process(const QVector<double>& input,
                                           const QVector<double>& desired)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), desired.size());
    if (n == 0) return {};

    QVector<double> output(n, 0.0);
    m_errors.resize(n);
    m_learningCurve.resize(n);

    double errorPowerSum = 0.0;

    for (int i = 0; i < n; ++i) {
        // Push new sample into circular buffer
        m_inputBuf[m_bufPos] = input[i];
        m_bufPos = (m_bufPos + 1) % m_order;

        // Compute filter output
        double y = computeOutput();
        output[i] = y;

        // Compute error
        double error = desired[i] - y;
        m_errors[i] = error;

        // Compute input power ||x||^2
        double power = 0.0;
        for (int j = 0; j < m_order; ++j) {
            double xj = getDelayedSample(j);
            power += xj * xj;
        }

        // Update weights via NLMS
        updateWeights(error, power);

        // Variable step-size adaptation
        if (m_variableStep)
            updateStepSize(error);

        // Track learning curve (instantaneous squared error)
        double se = error * error;
        errorPowerSum += se;
        m_learningCurve[i] = se;
    }

    double elapsed = timer.elapsed();
    m_stats.filterOrder = m_order;
    m_stats.blockSize = n;
    m_stats.finalErrorPower = (n > 0) ? errorPowerSum / n : 0.0;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit filteringDone(m_order, n, m_stats.finalErrorPower, elapsed);

    return output;
}

/* ---- Accessors ---- */

QVector<double> AdaptiveFilter9::coefficients() const { return m_weights; }
QVector<double> AdaptiveFilter9::errors() const { return m_errors; }
QVector<double> AdaptiveFilter9::learningCurve() const { return m_learningCurve; }

/* ---- Reset ---- */

void AdaptiveFilter9::resetStatistics()
{
    m_weights.fill(0.0);
    m_inputBuf.fill(0.0);
    m_bufPos = 0;
    m_errors.clear();
    m_learningCurve.clear();
    m_currentMu = m_mu;
    m_gradEst = 0.0;
    m_prevError = 0.0;
    m_prevGrad = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
