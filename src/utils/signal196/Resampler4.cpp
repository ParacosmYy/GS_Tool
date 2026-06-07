/**
 * @file Resampler4.cpp
 * @brief Resampler4 实现
 *
 * 实现多相位重采样器：CIC抗混叠滤波、有理数速率转换、多相位分支处理。
 */

#include "utils/signal196/Resampler4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler4::Resampler4(QObject *parent) : QObject(parent)
{
    m_intState.resize(m_cicOrder, 0.0);
    m_combState.resize(m_cicOrder, 0.0);
    designFilters();
}

Resampler4::~Resampler4() = default;

/* ---- GCD ---- */

int Resampler4::gcd(int a, int b)
{
    while (b) { int t = b; b = a % b; a = t; }
    return a;
}

/* ---- Configuration ---- */

void Resampler4::setRatio(int P, int Q)
{
    m_P = qMax(1, P);
    m_Q = qMax(1, Q);
    int g = gcd(m_P, m_Q);
    m_P /= g;
    m_Q /= g;
    m_phase = 0.0;
    designFilters();
}

void Resampler4::setCicOrder(int order)
{
    m_cicOrder = qMax(1, order);
    m_intState.resize(m_cicOrder, 0.0);
    m_combState.resize(m_cicOrder, 0.0);
    designFilters();
}

void Resampler4::setTapsPerBranch(int taps)
{
    m_tapsPerBranch = qMax(2, taps);
    designFilters();
}

/* ---- Design polyphase filter bank ---- */

void Resampler4::designFilters()
{
    // Design prototype lowpass filter (sinc * CIC compensation window)
    int totalTaps = m_P * m_tapsPerBranch;
    double fc = 1.0 / qMax(m_P, m_Q);  // cutoff frequency

    QVector<double> proto(totalTaps);
    int center = totalTaps / 2;

    for (int n = 0; n < totalTaps; ++n) {
        double x = n - center;
        // Sinc function
        double sinc = (qAbs(x) < 1e-10) ? 1.0 : qSin(M_PI * fc * x) / (M_PI * x);
        // CIC compensation: inverse sinc^N in passband
        double cicComp = 1.0;
        if (qAbs(x) > 1e-10) {
            double u = M_PI * x / totalTaps;
            double sincCic = qSin(u) / u;
            cicComp = qPow(qMax(sincCic, 0.01), m_cicOrder);
        }
        // Hamming window
        double win = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (totalTaps - 1));
        proto[n] = sinc * cicComp * win;
    }

    // Normalize
    double energy = 0.0;
    for (double v : proto) energy += v * v;
    energy = qSqrt(qMax(energy, 1e-15));
    for (double& v : proto) v /= energy;

    // Polyphase decomposition: branch b gets proto[b], proto[b+P], proto[b+2P], ...
    m_branches.resize(m_P);
    for (int b = 0; b < m_P; ++b) {
        m_branches[b].clear();
        for (int k = 0; b + k * m_P < totalTaps; ++k)
            m_branches[b].append(proto[b + k * m_P]);
    }
}

/* ---- CIC integrator ---- */

double Resampler4::cicIntegrate(double sample)
{
    double x = sample;
    for (int i = 0; i < m_cicOrder; ++i) {
        m_intState[i] += x;
        x = m_intState[i];
    }
    return x;
}

/* ---- CIC comb ---- */

double Resampler4::cicComb(double sample)
{
    double x = sample;
    for (int i = 0; i < m_cicOrder; ++i) {
        double prev = m_combState[i];
        m_combState[i] = x;
        x = x - prev;
    }
    return x;
}

/* ---- CIC response ---- */

QVector<double> Resampler4::cicResponse(int length) const
{
    QVector<double> h(length, 0.0);
    // CIC impulse response: N-stage moving average
    int R = m_P;
    for (int n = 0; n < length; ++n) {
        double val = 1.0;
        for (int s = 0; s < m_cicOrder; ++s) {
            // Triangular convolution approximation
            double t = static_cast<double>(n) / length;
            val *= (t < 0.5) ? (1.0 - 2.0 * qAbs(t - 0.25)) : 0.0;
        }
        h[n] = val;
    }
    // Normalize
    double mx = 0.0;
    for (double v : h) mx = qMax(mx, qAbs(v));
    if (mx > 1e-15)
        for (double& v : h) v /= mx;
    return h;
}

/* ---- Polyphase branch ---- */

QVector<double> Resampler4::polyphaseBranch(int b) const
{
    if (b < 0 || b >= m_branches.size()) return {};
    return m_branches[b];
}

/* ---- Ratio ---- */

QPair<int, int> Resampler4::ratio() const { return {m_P, m_Q}; }

/* ---- Process ---- */

QVector<double> Resampler4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || (m_P == 1 && m_Q == 1)) return input;

    // Append to internal buffer
    m_buffer.append(input);

    // Output length estimate
    int outLen = static_cast<int>(
        static_cast<double>(m_buffer.size()) * m_P / m_Q) + 2;
    QVector<double> output;
    output.reserve(outLen);

    int inLen = m_buffer.size();
    int tapLen = m_tapsPerBranch;
    int needHistory = tapLen;

    // Polyphase resampling with fractional phase
    while (true) {
        // Current input index (integer part)
        int inIdx = static_cast<int>(m_phase);
        int branch = static_cast<int>((m_phase - inIdx) * m_P);

        // Clamp branch
        branch = branch % m_P;

        // Need enough samples
        if (inIdx + tapLen > inLen) break;

        // Compute output sample from polyphase branch
        double y = 0.0;
        const QVector<double>& filt = m_branches[branch];
        for (int k = 0; k < filt.size() && inIdx + k < inLen; ++k)
            y += filt[k] * m_buffer[inIdx + k];

        // Apply CIC anti-alias filtering
        y = cicIntegrate(y);
        y = cicComb(y);

        output.append(y);

        // Advance phase
        m_phase += static_cast<double>(m_Q) / m_P;
    }

    // Remove consumed samples from buffer
    int consumed = static_cast<int>(m_phase);
    if (consumed > 0 && consumed <= m_buffer.size()) {
        m_buffer = m_buffer.mid(consumed);
        m_phase -= consumed;
    }

    m_stats.totalSamples += input.size();
    m_stats.inputRate = m_Q;
    m_stats.outputRate = m_P;
    m_stats.cicOrder = m_cicOrder;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamples > 0)
        ? m_timeSum / m_stats.totalSamples * 1000 : 0.0;

    emit processingCompleted(input.size(), output.size(), timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Resampler4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_phase = 0.0;
    m_buffer.clear();
    m_intState.fill(0.0);
    m_combState.fill(0.0);
}
