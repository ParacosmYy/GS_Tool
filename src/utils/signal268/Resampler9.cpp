/**
 * @file Resampler9.cpp
 * @brief Resampler9 实现
 *
 * 实现重采样器：多相分解与Kaiser窗sinc插值任意比率采样率转换。
 */

#include "utils/signal268/Resampler9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler9::Resampler9(QObject *parent)
    : QObject(parent)
{
    setParameters(44100, 48000);
}

Resampler9::~Resampler9() = default;

/* ---- GCD ---- */

int Resampler9::gcd(int a, int b)
{
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a;
}

/* ---- Kaiser window ---- */

double Resampler9::kaiserWindow(int n, int N, double beta) const
{
    if (N <= 1) return 1.0;

    // Modified Bessel function I0(x) approximation
    auto besselI0 = [](double x) -> double {
        double sum = 1.0;
        double term = 1.0;
        for (int k = 1; k <= 25; ++k) {
            term *= (x / (2.0 * k)) * (x / (2.0 * k));
            sum += term;
            if (term < 1e-12) break;
        }
        return sum;
    };

    double alpha = (N - 1) / 2.0;
    double arg = beta * qSqrt(1.0 - qPow((n - alpha) / alpha, 2));
    return besselI0(arg) / besselI0(beta);
}

/* ---- Design prototype filter ---- */

void Resampler9::designFilter()
{
    int N = m_numTaps * m_L;  // Total filter length
    m_filter.resize(N);

    // Cutoff: min(1/L, 1/M) * pi, normalized to 0.5
    double cutoff = qMin(1.0 / m_L, 1.0 / m_M);

    for (int n = 0; n < N; ++n) {
        double t = n - (N - 1) / 2.0;
        double sinc;
        if (qAbs(t) < 1e-10)
            sinc = 2.0 * cutoff;
        else
            sinc = qSin(2.0 * M_PI * cutoff * t) / (M_PI * t);

        m_filter[n] = sinc * kaiserWindow(n, N, m_kaiserBeta) * m_L;
    }

    // Normalize filter gain
    double sum = 0.0;
    for (double v : m_filter) sum += v;
    if (sum > 0.0) {
        double norm = m_L / (sum / N);
        for (double& v : m_filter) v *= norm;
    }
}

/* ---- Polyphase decomposition ---- */

void Resampler9::decomposePolyphase()
{
    // Split prototype filter into L sub-filters
    m_polyphase.resize(m_L);
    int subLen = m_filter.size() / m_L;

    for (int k = 0; k < m_L; ++k) {
        m_polyphase[k].resize(subLen);
        for (int n = 0; n < subLen; ++n) {
            int idx = k + n * m_L;
            if (idx < m_filter.size())
                m_polyphase[k][n] = m_filter[idx];
            else
                m_polyphase[k][n] = 0.0;
        }
    }
}

/* ---- Configuration ---- */

void Resampler9::setParameters(int inputRate, int outputRate, int numTaps,
                                 double kaiserBeta)
{
    m_inputRate = qMax(1, inputRate);
    m_outputRate = qMax(1, outputRate);
    m_numTaps = qMax(4, numTaps);
    m_kaiserBeta = qBound(0.0, kaiserBeta, 20.0);

    m_gcd = gcd(m_inputRate, m_outputRate);
    m_L = m_outputRate / m_gcd;   // Interpolation
    m_M = m_inputRate / m_gcd;    // Decimation

    designFilter();
    decomposePolyphase();

    // Initialize delay line
    int subLen = m_filter.size() / m_L;
    m_delayLine.resize(subLen, 0.0);
    m_delayPos = 0;
    m_phase = 0.0;
}

/* ---- Process buffer ---- */

QVector<double> Resampler9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int subLen = m_delayLine.size();
    QVector<double> output;

    for (int i = 0; i < input.size(); ++i) {
        // Insert sample into delay line
        m_delayLine[m_delayPos % subLen] = input[i];
        m_delayPos++;

        // Generate output samples at interpolation phases
        while (m_phase < m_L) {
            int polyIdx = static_cast<int>(m_phase);
            if (polyIdx >= m_L) break;

            const auto& subFilter = m_polyphase[polyIdx];
            double y = 0.0;
            for (int k = 0; k < subLen; ++k) {
                int srcIdx = (m_delayPos - 1 - k + subLen * 2) % subLen;
                y += m_delayLine[srcIdx] * subFilter[k];
            }
            output.append(y);
            m_phase += m_M;
        }
        m_phase -= m_L;
    }

    double elapsed = timer.elapsed();
    m_stats.inputSamples += input.size();
    m_stats.outputSamples += output.size();
    m_stats.inputRate = m_inputRate;
    m_stats.outputRate = m_outputRate;
    m_stats.filterTaps = m_numTaps;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit resamplingDone(input.size(), output.size(), elapsed);
    return output;
}

/* ---- Process single sample (streaming) ---- */

void Resampler9::processSample(double input, QVector<double>& outputSamples)
{
    int subLen = m_delayLine.size();
    m_delayLine[m_delayPos % subLen] = input;
    m_delayPos++;

    while (m_phase < m_L) {
        int polyIdx = static_cast<int>(m_phase);
        if (polyIdx >= m_L) break;

        const auto& subFilter = m_polyphase[polyIdx];
        double y = 0.0;
        for (int k = 0; k < subLen; ++k) {
            int srcIdx = (m_delayPos - 1 - k + subLen * 2) % subLen;
            y += m_delayLine[srcIdx] * subFilter[k];
        }
        outputSamples.append(y);
        m_phase += m_M;
    }
    m_phase -= m_L;
}

/* ---- Accessors ---- */

QVector<QVector<double>> Resampler9::filterBank() const { return m_polyphase; }

/* ---- Reset ---- */

void Resampler9::reset()
{
    m_delayLine.fill(0.0);
    m_delayPos = 0;
    m_phase = 0.0;
}

void Resampler9::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
