/**
 * @file AdaptiveFilter11.cpp
 * @brief AdaptiveFilter11 实现
 *
 * 实现自适应滤波器：仿射投影算法与正则化矩阵求逆实现快速跟踪多径信道均衡。
 */

#include "utils/signal302/AdaptiveFilter11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AdaptiveFilter11::AdaptiveFilter11(QObject *parent)
    : QObject(parent)
{
    m_weights.resize(m_filterOrder, 0.0);
    m_inputMatrix.resize(m_projectionOrder, QVector<double>(m_filterOrder, 0.0));
}

AdaptiveFilter11::~AdaptiveFilter11() = default;

/* ---- Configuration ---- */

void AdaptiveFilter11::setFilterOrder(int order)
{
    m_filterOrder = qBound(4, order, 1024);
    m_weights.resize(m_filterOrder, 0.0);
    m_inputMatrix.resize(m_projectionOrder, QVector<double>(m_filterOrder, 0.0));
}

void AdaptiveFilter11::setProjectionOrder(int order)
{
    m_projectionOrder = qBound(1, order, 64);
    m_inputMatrix.resize(m_projectionOrder, QVector<double>(m_filterOrder, 0.0));
}

void AdaptiveFilter11::setStepSize(double mu) { m_stepSize = qBound(1e-6, mu, 2.0); }
void AdaptiveFilter11::setRegularization(double lambda) { m_regularization = qBound(1e-12, lambda, 1.0); }

/* ---- Solve regularized system via Cholesky decomposition ---- */

QVector<double> AdaptiveFilter11::solveRegularized(const QVector<QVector<double>>& X,
                                                     const QVector<double>& d) const
{
    int P = m_projectionOrder;

    // Form A = X * X^T + lambda * I  (P x P)
    QVector<QVector<double>> A(P, QVector<double>(P, 0.0));
    for (int i = 0; i < P; ++i)
        for (int j = i; j < P; ++j) {
            double dot = 0.0;
            int M = m_filterOrder;
            for (int k = 0; k < M; ++k)
                dot += X[i][k] * X[j][k];
            A[i][j] = dot;
            if (i == j) A[i][j] += m_regularization;
            A[j][i] = A[i][j];
        }

    // Form b = X * d (P x 1)
    QVector<double> b(P, 0.0);
    for (int i = 0; i < P; ++i) {
        double dot = 0.0;
        int dLen = qMin(d.size(), m_filterOrder);
        for (int k = 0; k < dLen; ++k)
            dot += X[i][k] * d[k];
        b[i] = dot;
    }

    // Cholesky decomposition: A = L * L^T
    QVector<QVector<double>> L(P, QVector<double>(P, 0.0));
    for (int i = 0; i < P; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = A[i][j];
            for (int k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];
            if (i == j) {
                L[i][j] = qSqrt(qMax(1e-300, sum));
            } else {
                L[i][j] = (L[j][j] > 1e-300) ? sum / L[j][j] : 0.0;
            }
        }
    }

    // Forward substitution: L * y = b
    QVector<double> y(P, 0.0);
    for (int i = 0; i < P; ++i) {
        double sum = b[i];
        for (int k = 0; k < i; ++k) sum -= L[i][k] * y[k];
        y[i] = (L[i][i] > 1e-300) ? sum / L[i][i] : 0.0;
    }

    // Back substitution: L^T * x = y
    QVector<double> x(P, 0.0);
    for (int i = P - 1; i >= 0; --i) {
        double sum = y[i];
        for (int k = i + 1; k < P; ++k) sum -= L[k][i] * x[k];
        x[i] = (L[i][i] > 1e-300) ? sum / L[i][i] : 0.0;
    }

    return x;
}

/* ---- Shift input matrix: push new sample ---- */

void AdaptiveFilter11::shiftInputMatrix(double newSample)
{
    int P = m_projectionOrder;
    int M = m_filterOrder;

    // Shift rows down (oldest row dropped)
    for (int p = P - 1; p > 0; --p)
        m_inputMatrix[p] = m_inputMatrix[p - 1];

    // New row: shift in new sample
    for (int k = M - 1; k > 0; --k)
        m_inputMatrix[0][k] = m_inputMatrix[0][k - 1];
    m_inputMatrix[0][0] = newSample;
}

/* ---- Compute filter output ---- */

double AdaptiveFilter11::computeOutput(const QVector<double>& inputVec) const
{
    double y = 0.0;
    int M = qMin(m_filterOrder, inputVec.size());
    for (int k = 0; k < M; ++k)
        y += m_weights[k] * inputVec[k];
    return y;
}

/* ---- Main APA process ---- */

AdaptiveFilter11::FilterResult AdaptiveFilter11::process(const QVector<double>& input,
                                                            const QVector<double>& desired)
{
    QElapsedTimer timer;
    timer.start();

    FilterResult result;
    int n = qMin(input.size(), desired.size());
    if (n == 0) return result;

    result.output.resize(n, 0.0);
    result.error.resize(n, 0.0);
    result.weightTrace.resize(n, 0.0);

    double errorPower = 0.0;
    int convergedIter = 0;
    double prevErrorPower = 1e300;

    for (int i = 0; i < n; ++i) {
        // Update input matrix with new sample
        shiftInputMatrix(input[i]);

        // Compute filter output
        double y = computeOutput(m_inputMatrix[0]);
        result.output[i] = y;

        // Compute error
        double e = desired[i] - y;
        result.error[i] = e;

        // APA weight update: w += mu * X^T * (X*X^T + lambda*I)^{-1} * e_vec
        // Build error vector for projection order
        QVector<double> eVec(m_projectionOrder, 0.0);
        eVec[0] = e;

        // Solve regularized system for step direction
        QVector<double> stepDir = solveRegularized(m_inputMatrix, eVec);

        // Update weights
        for (int p = 0; p < m_projectionOrder; ++p) {
            for (int k = 0; k < m_filterOrder; ++k)
                m_weights[k] += m_stepSize * stepDir[p] * m_inputMatrix[p][k];
        }

        // Track weight norm
        double wNorm = 0.0;
        for (double w : m_weights) wNorm += w * w;
        result.weightTrace[i] = qSqrt(wNorm);

        // Error power (exponential moving average)
        errorPower = 0.99 * errorPower + 0.01 * e * e;
        result.errorPower = errorPower;

        // Convergence check
        if (i > m_filterOrder * 2 && qAbs(prevErrorPower - errorPower) < 1e-10) {
            if (convergedIter == 0) convergedIter = i;
        }
        prevErrorPower = errorPower;
    }

    result.finalErrorPower = errorPower;
    result.iterations = n;
    result.converged = (convergedIter > 0);

    m_stats.totalBlocks++;
    m_stats.filterOrder = m_filterOrder;
    m_stats.projectionOrder = m_projectionOrder;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;

    emit filterDone(n, errorPower, elapsed);
    return result;
}

/* ---- Reset ---- */

void AdaptiveFilter11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_weights.fill(0.0);
    for (auto& row : m_inputMatrix)
        row.fill(0.0);
}
