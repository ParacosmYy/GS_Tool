/**
 * @file AdaptiveFilter4.cpp
 * @brief AdaptiveFilter4 实现
 *
 * 实现仿射投影自适应滤波器：投影阶数控制、正则化、收敛优化。
 */

#include "utils/signal199/AdaptiveFilter4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AdaptiveFilter4::AdaptiveFilter4(QObject *parent) : QObject(parent)
{
    resetFilter();
}
AdaptiveFilter4::~AdaptiveFilter4() = default;

/* ---- Configuration ---- */

void AdaptiveFilter4::setFilterLength(int len)
{
    m_filterLen = qMax(1, len);
    resetFilter();
}

void AdaptiveFilter4::setStepSize(double mu) { m_stepSize = qBound(0.001, mu, 2.0); }
void AdaptiveFilter4::setProjectionOrder(int p) { m_projOrder = qMax(1, p); }
void AdaptiveFilter4::setRegularization(double d) { m_regularization = qBound(1e-10, d, 1.0); }
void AdaptiveFilter4::setAdaptiveOrder(bool e) { m_adaptiveOrder = e; }

/* ---- Reset ---- */

void AdaptiveFilter4::resetFilter()
{
    m_weights.resize(m_filterLen);
    m_weights.fill(0.0);

    m_inputBuf.resize(m_filterLen);
    m_inputBuf.fill(0.0);
    m_bufIdx = 0;

    m_inputMatrix.resize(m_projOrder);
    for (auto& row : m_inputMatrix) {
        row.resize(m_filterLen);
        row.fill(0.0);
    }
    m_errorVec.resize(m_projOrder);
    m_errorVec.fill(0.0);
    m_matIdx = 0;
    m_sampleCount = 0;
}

void AdaptiveFilter4::resetStatistics()
{
    m_stats = Stats{};
    m_stats.filterLength = m_filterLen;
    m_stats.projectionOrder = m_projOrder;
    m_timeSum = 0.0;
}

/* ---- Auto projection order ---- */

int AdaptiveFilter4::autoProjectionOrder(double errorPower)
{
    // Increase order if error is large, decrease if small
    if (errorPower > 0.1)
        return qMin(m_projOrder * 2, m_filterLen);
    if (errorPower < 0.001)
        return qMax(1, m_projOrder / 2);
    return m_projOrder;
}

/* ---- Solve regularized normal equations ---- */

QVector<double> AdaptiveFilter4::solveRegularized(
    const QVector<QVector<double>>& U, const QVector<double>& e) const
{
    int P = U.size();
    if (P == 0) return {};

    // Form Gram matrix G = U*U^T + delta*I
    QVector<QVector<double>> G(P, QVector<double>(P, 0.0));
    for (int i = 0; i < P; ++i) {
        for (int j = 0; j < P; ++j) {
            for (int k = 0; k < m_filterLen; ++k)
                G[i][j] += U[i][k] * U[j][k];
        }
        G[i][i] += m_regularization;
    }

    // Solve G * a = e via Gaussian elimination
    QVector<double> rhs = e;
    // Forward elimination
    for (int i = 0; i < P; ++i) {
        double pivot = G[i][i];
        if (qAbs(pivot) < 1e-15) continue;
        for (int j = i + 1; j < P; ++j) {
            double factor = G[j][i] / pivot;
            for (int k = i; k < P; ++k)
                G[j][k] -= factor * G[i][k];
            rhs[j] -= factor * rhs[i];
        }
    }
    // Back substitution
    QVector<double> a(P, 0.0);
    for (int i = P - 1; i >= 0; --i) {
        if (qAbs(G[i][i]) < 1e-15) continue;
        a[i] = rhs[i];
        for (int j = i + 1; j < P; ++j)
            a[i] -= G[i][j] * a[j];
        a[i] /= G[i][i];
    }
    return a;
}

/* ---- Filter output ---- */

double AdaptiveFilter4::filterOutput(double input) const
{
    double y = 0.0;
    int idx = m_bufIdx;
    for (int i = 0; i < m_filterLen; ++i) {
        idx = (idx - 1 + m_filterLen) % m_filterLen;
        y += m_weights[i] * m_inputBuf[idx];
    }
    return y;
}

/* ---- Process one sample ---- */

double AdaptiveFilter4::process(double desired, double input)
{
    QElapsedTimer timer;
    timer.start();

    // Update input buffer (circular)
    m_inputBuf[m_bufIdx] = input;
    m_bufIdx = (m_bufIdx + 1) % m_filterLen;

    // Compute filter output
    double y = filterOutput(input);
    double error = desired - y;

    // Update input matrix (shift old rows down, add new row)
    int P = m_projOrder;
    m_matIdx = (m_matIdx + 1) % P;

    // Fill current row with latest input vector
    for (int i = 0; i < m_filterLen; ++i) {
        int srcIdx = (m_bufIdx - 1 - i + m_filterLen * 2) % m_filterLen;
        m_inputMatrix[m_matIdx][i] = m_inputBuf[srcIdx];
    }
    m_errorVec[m_matIdx] = error;

    // Auto-tune projection order
    if (m_adaptiveOrder) {
        double errPower = error * error;
        int newP = autoProjectionOrder(errPower);
        if (newP != P) {
            P = newP;
            m_projOrder = P;
        }
    }

    // APA update: solve regularized normal equations
    if (m_sampleCount >= static_cast<double>(P)) {
        QVector<QVector<double>> U(P);
        QVector<double> e(P);
        for (int k = 0; k < P; ++k) {
            int rowIdx = (m_matIdx - k + P) % P;
            U[k] = m_inputMatrix[rowIdx];
            e[k] = m_errorVec[rowIdx];
        }

        auto alpha = solveRegularized(U, e);

        // Update weights: w += mu * U^T * alpha
        for (int i = 0; i < m_filterLen; ++i) {
            double delta = 0.0;
            for (int k = 0; k < P; ++k)
                delta += U[k][i] * alpha[k];
            m_weights[i] += m_stepSize * delta;
        }
    }

    m_sampleCount++;

    // Compute misalignment
    double wNorm = 0.0;
    for (double w : m_weights) wNorm += w * w;
    m_stats.misalignment = qSqrt(wNorm);

    m_stats.totalSamples++;
    m_stats.filterLength = m_filterLen;
    m_stats.projectionOrder = m_projOrder;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamples;

    emit filterUpdated(static_cast<int>(m_sampleCount), error, m_stats.misalignment);
    return y;
}

/* ---- Process batch ---- */

QVector<double> AdaptiveFilter4::processBatch(const QVector<double>& desired,
                                                const QVector<double>& input)
{
    int n = qMin(desired.size(), input.size());
    QVector<double> output(n);
    for (int i = 0; i < n; ++i)
        output[i] = process(desired[i], input[i]);
    return output;
}

/* ---- Accessors ---- */

QVector<double> AdaptiveFilter4::coefficients() const { return m_weights; }
int AdaptiveFilter4::currentProjectionOrder() const { return m_projOrder; }
