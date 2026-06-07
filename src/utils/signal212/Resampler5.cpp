/**
 * @file Resampler5.cpp
 * @brief Resampler5 实现
 *
 * 实现采样率转换器：多相FIR插值、级联积分梳状(CIC)抽取、任意比采样率转换。
 */

#include "utils/signal212/Resampler5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler5::Resampler5(QObject *parent) : QObject(parent) {}
Resampler5::~Resampler5() = default;

/* ---- Configuration ---- */

void Resampler5::setInputRate(int rate) { m_inputRate = qMax(1, rate); }
void Resampler5::setOutputRate(int rate) { m_outputRate = qMax(1, rate); }
void Resampler5::setFIRLength(int length) { m_firLength = qMax(4, length); }

/* ---- GCD ---- */

int Resampler5::gcd(int a, int b) { return (b == 0) ? a : gcd(b, a % b); }

/* ---- Sinc ---- */

double Resampler5::sinc(double x)
{
    if (qAbs(x) < 1e-10) return 1.0;
    return qSin(M_PI * x) / (M_PI * x);
}

/* ---- Blackman window ---- */

double Resampler5::blackman(int n, int N)
{
    return 0.42 - 0.5 * qCos(2.0 * M_PI * n / (N - 1))
                + 0.08 * qCos(4.0 * M_PI * n / (N - 1));
}

/* ---- Design lowpass FIR ---- */

QVector<double> Resampler5::designLowpassFIR(int length, double cutoff) const
{
    QVector<double> h(length, 0.0);
    int mid = length / 2;
    for (int n = 0; n < length; ++n) {
        double x = n - mid;
        h[n] = 2.0 * cutoff * sinc(2.0 * cutoff * x) * blackman(n, length);
    }
    // Normalize
    double sum = 0.0;
    for (double v : h) sum += v;
    if (sum > 0.0)
        for (double& v : h) v /= sum;
    return h;
}

/* ---- Polyphase decomposition ---- */

QVector<QVector<double>> Resampler5::polyphaseDecompose(
    const QVector<double>& firCoeffs, int L) const
{
    QVector<QVector<double>> filters(L);
    int subLen = (firCoeffs.size() + L - 1) / L;
    for (int k = 0; k < L; ++k) {
        filters[k].resize(subLen, 0.0);
        for (int i = 0; i < subLen; ++i) {
            int idx = k + i * L;
            if (idx < firCoeffs.size()) filters[k][i] = firCoeffs[idx];
        }
    }
    return filters;
}

/* ---- Compute ratio ---- */

void Resampler5::computeRatio(int& L, int& M) const
{
    int g = gcd(m_outputRate, m_inputRate);
    L = m_outputRate / g;
    M = m_inputRate / g;
}

/* ---- Initialize ---- */

void Resampler5::initialize()
{
    int L, M;
    computeRatio(L, M);

    // Design interpolation lowpass FIR
    double cutoff = 1.0 / (2.0 * qMax(L, M));
    auto fir = designLowpassFIR(m_firLength * L, cutoff);

    // Scale for interpolation gain
    for (double& v : fir) v *= L;

    m_polyFilters = polyphaseDecompose(fir, L);

    // Setup CIC decimator
    m_cicParams.decimationFactor = M;
    m_cicParams.order = 3;
    m_cicParams.differentialDelay = 1;
    resetCIC();

    m_initialized = true;
    m_stats.inputRate = m_inputRate;
    m_stats.outputRate = m_outputRate;
    m_stats.cicOrder = m_cicParams.order;
}

/* ---- Reset CIC ---- */

void Resampler5::resetCIC()
{
    int order = m_cicParams.order;
    m_integratorState.resize(order, 0.0);
    m_combState.resize(order, 0.0);
    m_cicPhase = 0;
}

/* ---- Polyphase FIR interpolation ---- */

QVector<double> Resampler5::interpolatePolyphase(
    const QVector<double>& input, int L) const
{
    if (m_polyFilters.isEmpty()) return input;
    int nSub = m_polyFilters[0].size();
    QVector<double> output;
    output.reserve(input.size() * L);

    for (int i = 0; i < input.size(); ++i) {
        for (int k = 0; k < L; ++k) {
            double sum = 0.0;
            for (int j = 0; j < nSub; ++j) {
                int idx = i - j;
                if (idx >= 0 && idx < input.size())
                    sum += m_polyFilters[k][j] * input[idx];
            }
            output.append(sum);
        }
    }
    return output;
}

/* ---- CIC decimation ---- */

QVector<double> Resampler5::decimateCIC(const QVector<double>& input, int M)
{
    int order = m_cicParams.order;
    QVector<double> output;
    output.reserve(input.size() / M + 1);

    for (int i = 0; i < input.size(); ++i) {
        // Integrator stages
        m_integratorState[0] += input[i];
        for (int s = 1; s < order; ++s)
            m_integratorState[s] += m_integratorState[s - 1];

        m_cicPhase++;
        if (m_cicPhase >= M) {
            m_cicPhase = 0;
            // Comb stages
            double val = m_integratorState[order - 1];
            for (int s = 0; s < order; ++s) {
                double prev = m_combState[s];
                m_combState[s] = val;
                val -= prev;
            }
            output.append(val);
        }
    }
    return output;
}

/* ---- Process (full pipeline) ---- */

QVector<double> Resampler5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) initialize();

    int L, M;
    computeRatio(L, M);

    // Step 1: Polyphase FIR interpolation by L
    QVector<double> interpolated = interpolatePolyphase(input, L);

    // Step 2: CIC decimation by M
    QVector<double> output = decimateCIC(interpolated, M);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit resamplingCompleted(input.size(), output.size(), timer.elapsed());

    return output;
}

/* ---- Reset ---- */

void Resampler5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_initialized = false;
    m_polyFilters.clear();
    resetCIC();
}
