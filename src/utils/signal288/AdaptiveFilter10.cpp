/**
 * @file AdaptiveFilter10.cpp
 * @brief AdaptiveFilter10 实现
 *
 * 实现自适应滤波器：递归最小二乘与指数遗忘因子的快速收敛系统辨识。
 */

#include "utils/signal288/AdaptiveFilter10.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

AdaptiveFilter10::AdaptiveFilter10(QObject *parent)
    : QObject(parent)
{
    setConfig(FilterConfig{});
}

AdaptiveFilter10::~AdaptiveFilter10() = default;

/* ---- Configuration ---- */

void AdaptiveFilter10::setConfig(const FilterConfig& cfg)
{
    m_config = cfg;
    m_config.filterOrder = qBound(1, cfg.filterOrder, 512);
    m_config.forgettingFactor = qBound(0.9, cfg.forgettingFactor, 1.0);
    m_config.regularization = qBound(1e-12, cfg.regularization, 1.0);
    initFilter();
}

/* ---- Initialize filter ---- */

void AdaptiveFilter10::initFilter()
{
    int N = m_config.filterOrder;
    m_w.fill(0.0, N);
    m_xBuf.fill(0.0, N);
    m_bufIdx = 0;

    // Initialize P = (1/delta) * I
    m_P.resize(N, QVector<double>(N, 0.0));
    for (int i = 0; i < N; ++i)
        m_P[i][i] = 1.0 / m_config.regularization;
}

/* ---- Get input vector from circular buffer ---- */

QVector<double> AdaptiveFilter10::getInputVector() const
{
    int N = m_config.filterOrder;
    QVector<double> x(N);
    for (int i = 0; i < N; ++i) {
        // Most recent sample first
        int idx = (m_bufIdx - 1 - i + N) % N;
        x[i] = m_xBuf[idx];
    }
    return x;
}

/* ---- RLS update step ---- */

void AdaptiveFilter10::rlsUpdate(const QVector<double>& x, double error)
{
    int N = m_config.filterOrder;
    double lambda = m_config.forgettingFactor;

    // Compute gain vector: k = P*x / (lambda + x'*P*x)
    QVector<double> Px(N, 0.0);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            Px[i] += m_P[i][j] * x[j];

    double xPx = 0.0;
    for (int i = 0; i < N; ++i)
        xPx += x[i] * Px[i];

    double denom = lambda + xPx;
    if (qAbs(denom) < 1e-30) return;

    QVector<double> k(N);
    for (int i = 0; i < N; ++i)
        k[i] = Px[i] / denom;

    // Update weights: w = w + k * error
    for (int i = 0; i < N; ++i)
        m_w[i] += k[i] * error;

    // Update P: P = (P - k*x'*P) / lambda
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double kxP = 0.0;
            for (int l = 0; l < N; ++l)
                kxP += k[i] * x[l] * m_P[l][j];
            m_P[i][j] = (m_P[i][j] - kxP) / lambda;
        }
    }
}

/* ---- Process single sample ---- */

AdaptiveFilter10::FilterOutput AdaptiveFilter10::processSample(double input, double desired)
{
    FilterOutput out;

    // Push input into circular buffer
    m_xBuf[m_bufIdx] = input;
    m_bufIdx = (m_bufIdx + 1) % m_config.filterOrder;

    // Get input vector
    QVector<double> x = getInputVector();

    // Compute filter output: y = w' * x
    double y = 0.0;
    for (int i = 0; i < m_config.filterOrder; ++i)
        y += m_w[i] * x[i];

    // Compute error
    double error = desired - y;

    // RLS update
    rlsUpdate(x, error);

    out.output = y;
    out.error = error;

    // Compute weights norm
    double wNorm = 0.0;
    for (int i = 0; i < m_config.filterOrder; ++i)
        wNorm += m_w[i] * m_w[i];
    out.weightsNorm = qSqrt(wNorm);

    // Update stats
    m_stats.totalSamples++;
    m_errorSum += error * error;
    m_stats.avgError = qSqrt(m_errorSum / m_stats.totalSamples);
    m_stats.filterOrder = m_config.filterOrder;

    emit sampleProcessed(error, out.weightsNorm);
    return out;
}

/* ---- Process block ---- */

QVector<AdaptiveFilter10::FilterOutput> AdaptiveFilter10::processBlock(
    const QVector<double>& input, const QVector<double>& desired)
{
    QElapsedTimer timer;
    timer.start();

    int len = qMin(input.size(), desired.size());
    QVector<FilterOutput> results;
    results.reserve(len);

    double blockErrSum = 0.0;
    for (int i = 0; i < len; ++i) {
        FilterOutput out = processSample(input[i], desired[i]);
        results.append(out);
        blockErrSum += out.error * out.error;
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples);
    emit blockDone(len, qSqrt(blockErrSum / qMax(1, len)), elapsed);

    return results;
}

/* ---- Filter only (no adaptation) ---- */

double AdaptiveFilter10::filterOnly(double input) const
{
    double y = 0.0;
    int N = m_config.filterOrder;
    for (int i = 0; i < N; ++i) {
        int idx = (m_bufIdx - 1 - i + N) % N;
        y += m_w[i] * m_xBuf[idx];
    }
    // Note: doesn't update buffer (const method), uses current state
    return y;
}

/* ---- System identification ---- */

QVector<AdaptiveFilter10::FilterOutput> AdaptiveFilter10::systemIdentify(
    const QVector<double>& input, const QVector<double>& systemOutput)
{
    resetStatistics();
    return processBlock(input, systemOutput);
}

/* ---- Reset ---- */

void AdaptiveFilter10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_errorSum = 0.0;
    initFilter();
}
