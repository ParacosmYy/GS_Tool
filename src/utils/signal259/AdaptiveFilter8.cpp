/**
 * @file AdaptiveFilter8.cpp
 * @brief AdaptiveFilter8 实现
 *
 * 实现自适应滤波器：RLS与QR分解数值稳定快速RLS。
 */

#include "utils/signal259/AdaptiveFilter8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

AdaptiveFilter8::AdaptiveFilter8(QObject *parent)
    : QObject(parent) { initFilter(); }
AdaptiveFilter8::~AdaptiveFilter8() = default;

/* ---- Configuration ---- */

void AdaptiveFilter8::configure(const Config& config) { m_config = config; initFilter(); }
void AdaptiveFilter8::setOrder(int order) { m_config.order = qMax(1, order); initFilter(); }
void AdaptiveFilter8::setForgettingFactor(double lambda) { m_config.forgettingFactor = qBound(0.9, lambda, 1.0); }
void AdaptiveFilter8::setRegularization(double delta) { m_config.regularization = qMax(1e-12, delta); }
void AdaptiveFilter8::setQRMode(bool enabled) { m_config.useQR = enabled; }

/* ---- Initialize filter state ---- */

void AdaptiveFilter8::initFilter()
{
    m_order = m_config.order;
    int n = m_order;

    // Initialize inverse correlation matrix P = delta^-1 * I
    m_P.resize(n);
    for (int i = 0; i < n; ++i) {
        m_P[i].resize(n, 0.0);
        m_P[i][i] = 1.0 / m_config.regularization;
    }

    m_w.resize(n, 0.0);
    m_xBuf.resize(n, 0.0);
    m_bufIdx = 0;

    // QR state: upper triangular R matrix
    m_R.resize(n);
    for (int i = 0; i < n; ++i) {
        m_R[i].resize(n, 0.0);
        m_R[i][i] = qSqrt(m_config.regularization);
    }
    m_errQR.resize(n, 0.0);

    m_errors.clear();
}

/* ---- Givens rotation ---- */

void AdaptiveFilter8::givensRotate(double a, double b, double& c, double& s) const
{
    if (qFabs(b) < 1e-300) {
        c = 1.0;
        s = 0.0;
    } else if (qFabs(b) > qFabs(a)) {
        double t = -a / b;
        s = 1.0 / qSqrt(1.0 + t * t);
        c = s * t;
    } else {
        double t = -b / a;
        c = 1.0 / qSqrt(1.0 + t * t);
        s = c * t;
    }
}

/* ---- Standard RLS update ---- */

double AdaptiveFilter8::rlsUpdate(double input, double desired)
{
    double lambda = m_config.forgettingFactor;
    int n = m_order;

    // Shift input buffer
    m_xBuf[m_bufIdx] = input;
    m_bufIdx = (m_bufIdx + 1) % n;

    // Form input vector (tapped delay line)
    QVector<double> x(n);
    for (int i = 0; i < n; ++i)
        x[i] = m_xBuf[(m_bufIdx - 1 - i + n) % n];

    // Compute gain vector: k = P*x / (lambda + x'*P*x)
    QVector<double> Px(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            Px[i] += m_P[i][j] * x[j];

    double xPx = 0.0;
    for (int i = 0; i < n; ++i)
        xPx += x[i] * Px[i];

    double denom = lambda + xPx;
    if (qFabs(denom) < 1e-300) denom = 1e-300;

    QVector<double> k(n);
    for (int i = 0; i < n; ++i)
        k[i] = Px[i] / denom;

    // Compute output and error
    double output = 0.0;
    for (int i = 0; i < n; ++i)
        output += m_w[i] * x[i];
    double error = desired - output;

    // Update weights
    for (int i = 0; i < n; ++i)
        m_w[i] += k[i] * error;

    // Update P: P = (P - k*x'*P) / lambda
    for (int i = 0; i < n; ++i) {
        double kxP = 0.0;
        for (int j = 0; j < n; ++j)
            kxP += k[i] * x[j] * m_P[j][i]; // Simplified
        for (int j = 0; j < n; ++j)
            m_P[i][j] = (m_P[i][j] - k[i] * Px[j]) / lambda;
    }

    return error;
}

/* ---- QR-based RLS update ---- */

double AdaptiveFilter8::qrRLSUpdate(double input, double desired)
{
    double lambda = m_config.forgettingFactor;
    int n = m_order;

    // Form input vector
    m_xBuf[m_bufIdx] = input;
    m_bufIdx = (m_bufIdx + 1) % n;

    QVector<double> x(n);
    for (int i = 0; i < n; ++i)
        x[i] = m_xBuf[(m_bufIdx - 1 - i + n) % n];

    // Scale R by forgetting factor
    double sqLambda = qSqrt(lambda);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            m_R[i][j] *= sqLambda;

    // Apply Givens rotations to eliminate new row
    for (int j = 0; j < n; ++j) {
        double c, s;
        givensRotate(m_R[j][j], x[j], c, s);

        // Apply rotation to R row j and x
        for (int k = j; k < n; ++k) {
            double r1 = m_R[j][k];
            double r2 = (k < x.size()) ? x[k] : 0.0;
            m_R[j][k] = c * r1 + s * r2;
            if (k < x.size()) x[k] = -s * r1 + c * r2;
        }
    }

    // Compute output
    double output = 0.0;
    for (int i = 0; i < n; ++i)
        output += m_w[i] * x[i];
    double error = desired - output;

    // Update weights via back-substitution on R
    QVector<double> rhs(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            rhs[i] += m_R[i][j] * m_w[j];
    rhs[0] += error;

    for (int i = n - 1; i >= 0; --i) {
        double sum = rhs[i];
        for (int j = i + 1; j < n; ++j)
            sum -= m_R[i][j] * m_w[j];
        if (qFabs(m_R[i][i]) > 1e-300)
            m_w[i] = sum / m_R[i][i];
    }

    return error;
}

/* ---- Process single sample ---- */

double AdaptiveFilter8::processSample(double input, double desired)
{
    QElapsedTimer timer;
    timer.start();

    double error;
    if (m_config.useQR)
        error = qrRLSUpdate(input, desired);
    else
        error = rlsUpdate(input, desired);

    m_errors.append(error * error);

    double elapsed = timer.elapsed();
    m_stats.filterOrder = m_order;
    m_stats.samplesProcessed++;
    m_stats.finalError = error;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit filterUpdated(m_order, error, elapsed);
    return error;
}

/* ---- Process block ---- */

QVector<double> AdaptiveFilter8::processBlock(const QVector<double>& input,
                                               const QVector<double>& desired)
{
    int n = qMin(input.size(), desired.size());
    QVector<double> errors(n);
    for (int i = 0; i < n; ++i)
        errors[i] = processSample(input[i], desired[i]);
    return errors;
}

/* ---- Get coefficients ---- */

QVector<double> AdaptiveFilter8::coefficients() const { return m_w; }

/* ---- Get learning curve ---- */

QVector<double> AdaptiveFilter8::learningCurve() const { return m_errors; }

/* ---- Reset ---- */

void AdaptiveFilter8::resetStatistics()
{
    initFilter();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
