/**
 * @file Resampler7.cpp
 * @brief Resampler7 实现
 *
 * 实现重采样器：多相分解与Farrow结构任意有理采样率转换。
 */

#include "utils/signal240/Resampler7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler7::Resampler7(QObject *parent) : QObject(parent) {}
Resampler7::~Resampler7() = default;

/* ---- GCD ---- */

int Resampler7::gcd(int a, int b)
{
    while (b) { int t = b; b = a % b; a = t; }
    return a;
}

/* ---- Design prototype lowpass filter ---- */

QVector<double> Resampler7::designPrototype(int len, double cutoff) const
{
    QVector<double> h(len, 0.0);
    int mid = len / 2;
    double sumSq = 0.0;
    for (int i = 0; i < len; ++i) {
        int n = i - mid;
        // Sinc function
        double sinc = (n == 0) ? 1.0 : qSin(M_PI * cutoff * n) / (M_PI * n);
        // Hamming window
        double win = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (len - 1));
        h[i] = sinc * win;
        sumSq += h[i] * h[i];
    }
    // Normalize
    double norm = qSqrt(sumSq);
    if (norm > 1e-15)
        for (int i = 0; i < len; ++i) h[i] /= norm;
    return h;
}

/* ---- Decompose into polyphase branches ---- */

void Resampler7::decomposePolyphase(const QVector<double>& proto, int branches)
{
    int tapLen = proto.size() / branches;
    m_polyphase.resize(branches);
    for (int b = 0; b < branches; ++b) {
        m_polyphase[b].resize(tapLen);
        for (int t = 0; t < tapLen; ++t) {
            int idx = b + t * branches;
            m_polyphase[b][t] = (idx < proto.size()) ? proto[idx] : 0.0;
        }
    }
}

/* ---- Compute Farrow polynomial coefficients ---- */

void Resampler7::computeFarrowCoeffs(int order, int tapLen)
{
    // Lagrange interpolation coefficients for Farrow structure
    m_farrowCoeff.resize(order + 1);
    for (int p = 0; p <= order; ++p) {
        m_farrowCoeff[p].resize(tapLen, 0.0);
        for (int t = 0; t < tapLen; ++t) {
            // Newton forward difference approximation
            if (p == 0)
                m_farrowCoeff[p][t] = (t < m_polyphase[0].size()) ? m_polyphase[0][t] : 0.0;
            else if (p == 1 && t + 1 < tapLen)
                m_farrowCoeff[p][t] = m_farrowCoeff[0][t + 1] - m_farrowCoeff[0][t];
            else
                m_farrowCoeff[p][t] = 0.0;
        }
    }
}

/* ---- Evaluate polyphase branch at fractional phase ---- */

double Resampler7::evaluateBranch(int branch, double frac) const
{
    if (branch < 0 || branch >= m_polyphase.size()) return 0.0;
    const auto& taps = m_polyphase[branch];
    double y = 0.0;
    for (int t = 0; t < taps.size(); ++t) {
        int idx = (m_delayPos - static_cast<int>(taps.size()) + t);
        while (idx < 0) idx += m_delayLine.size();
        idx %= m_delayLine.size();
        y += m_delayLine[idx] * taps[t];
    }
    return y;
}

/* ---- Configure ---- */

bool Resampler7::configure(const Config& config)
{
    m_config = config;
    return configureRatio(config.outputRate, config.inputRate, config.polyphaseBranches);
}

/* ---- Configure with ratio ---- */

bool Resampler7::configureRatio(int P, int Q, int branches)
{
    if (P <= 0 || Q <= 0 || branches <= 0) return false;
    int g = gcd(P, Q);
    m_P = P / g;
    m_Q = Q / g;
    m_config.polyphaseBranches = branches;

    // Design prototype lowpass at cutoff = 1/max(P,Q)
    double cutoff = 1.0 / qMax(m_P, m_Q);
    m_filterLen = branches * qMax(8, 2 * qMax(m_P, m_Q));
    QVector<double> proto = designPrototype(m_filterLen, cutoff);
    decomposePolyphase(proto, branches);
    computeFarrowCoeffs(m_config.farrowOrder, m_filterLen / branches);

    // Initialize delay line
    int tapLen = m_filterLen / branches;
    m_delayLine.resize(tapLen + 4, 0.0);
    m_delayPos = 0;
    m_phaseAccum = 0.0;

    m_stats.inputRate = m_config.inputRate;
    m_stats.outputRate = m_config.outputRate;
    return true;
}

/* ---- Process ---- */

QVector<double> Resampler7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;

    for (int i = 0; i < input.size(); ++i) {
        // Write to delay line
        m_delayLine[m_delayPos] = input[i];
        m_delayPos = (m_delayPos + 1) % m_delayLine.size();

        // Produce output samples at interpolated positions
        while (m_phaseAccum < 1.0) {
            double fracPhase = m_phaseAccum * m_P;
            int branch = static_cast<int>(fracPhase) % m_config.polyphaseBranches;
            double frac = fracPhase - qFloor(fracPhase);

            // Evaluate polyphase filter at fractional position
            double y = evaluateBranch(branch, frac);
            output.append(y);

            m_phaseAccum += static_cast<double>(m_Q) / m_P;
        }
        m_phaseAccum -= 1.0;
    }

    m_stats.inputSamples += input.size();
    m_stats.outputSamples += output.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit resamplingCompleted(input.size(), output.size(), timer.elapsed());
    return output;
}

/* ---- Process single sample ---- */

QVector<double> Resampler7::processOne(double sample)
{
    QVector<double> input = {sample};
    return process(input);
}

/* ---- Flush ---- */

QVector<double> Resampler7::flush()
{
    // Feed zeros through to drain delay line
    QVector<double> tail;
    int tapLen = m_filterLen / m_config.polyphaseBranches;
    for (int i = 0; i < tapLen + 2; ++i)
        tail.append(0.0);
    return process(tail);
}

/* ---- Ratio ---- */

double Resampler7::ratio() const
{
    return static_cast<double>(m_P) / m_Q;
}

/* ---- Reset ---- */

void Resampler7::reset()
{
    m_delayLine.fill(0.0);
    m_delayPos = 0;
    m_phaseAccum = 0.0;
}

/* ---- Reset statistics ---- */

void Resampler7::resetStatistics()
{
    m_polyphase.clear(); m_farrowCoeff.clear(); m_delayLine.clear();
    m_filterLen = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
