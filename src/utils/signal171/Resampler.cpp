/**
 * @file Resampler.cpp
 * @brief Resampler 实现
 *
 * 实现多相重采样器：FIR抗混叠滤波器设计、多相分解、任意比率重采样。
 */

#include "utils/signal171/Resampler.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

Resampler::Resampler(QObject *parent) : QObject(parent) {}
Resampler::~Resampler() = default;

double Resampler::sinc(double x)
{
    if (qAbs(x) < 1e-10) return 1.0;
    double px = M_PI * x;
    return qSin(px) / px;
}

double Resampler::windowFunc(int n, int N) const
{
    double w = 0.0;
    switch (m_window) {
    case Blackman:
        w = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (N - 1))
            + 0.08 * qCos(4.0 * M_PI * n / (N - 1));
        break;
    case Hamming:
        w = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1));
        break;
    case Hann:
        w = 0.5 * (1.0 - qCos(2.0 * M_PI * n / (N - 1)));
        break;
    case Kaiser: {
        /* Simplified Kaiser using Bessel approximation */
        double alpha = (N - 1) / 2.0;
        double x = (n - alpha) / alpha;
        double beta = m_kaiserBeta;
        /* I0 approximation */
        double sum = 1.0;
        double term = 1.0;
        for (int k = 1; k <= 20; ++k) {
            term *= (beta * qSqrt(1.0 - x * x) / (2.0 * k));
            term *= (beta * qSqrt(1.0 - x * x) / (2.0 * k));  // Squared for proper scaling
            sum += term;
        }
        /* Normalize by I0(beta) */
        double i0beta = 1.0;
        double t = 1.0;
        for (int k = 1; k <= 20; ++k) {
            t *= (beta / (2.0 * k));
            t *= (beta / (2.0 * k));
            i0beta += t;
        }
        w = sum / i0beta;
        break;
    }
    }
    return w;
}

void Resampler::setRatio(double inRate, double outRate)
{
    if (inRate <= 0 || outRate <= 0) return;

    /* Compute L/M from ratio */
    double ratio = outRate / inRate;
    /* Simple fraction approximation */
    int bestL = 1, bestM = 1;
    double bestErr = qAbs(ratio - 1.0);
    for (int l = 1; l <= 100; ++l) {
        int m = qMax(1, qRound(l / ratio));
        double err = qAbs(static_cast<double>(l) / m - ratio);
        if (err < bestErr) { bestErr = err; bestL = l; bestM = m; }
    }
    m_L = bestL;
    m_M = bestM;
    m_stats.ratio = static_cast<double>(m_L) / m_M;

    /* Default filter length: multiple of L */
    m_filterLen = qMax(m_L * 16, 64);
    designFilter();
    decomposePolyphase();

    /* Init delay line */
    int subLen = m_filterLen / m_L;
    m_delayLine.resize(subLen + 1, 0.0);
    m_delayIdx = 0;
    m_phase = 0;
}

void Resampler::setFilterLength(int len) { m_filterLen = qMax(1, len); }
void Resampler::setWindowType(WindowType type) { m_window = type; }
void Resampler::setKaiserBeta(double beta) { m_kaiserBeta = beta; }

void Resampler::designFilter()
{
    int N = m_filterLen;
    m_filter.resize(N);
    double fc = 1.0 / qMax(m_L, m_M);  /* Cutoff at min(in,out)/2 */
    double gain = static_cast<double>(m_L);

    for (int n = 0; n < N; ++n) {
        double t = n - (N - 1) / 2.0;
        m_filter[n] = gain * fc * sinc(t * fc) * windowFunc(n, N);
    }
}

void Resampler::decomposePolyphase()
{
    if (m_filter.isEmpty() || m_L <= 0) return;
    int subLen = (m_filter.size() + m_L - 1) / m_L;
    m_polyFilter.resize(m_L);

    for (int k = 0; k < m_L; ++k) {
        m_polyFilter[k].resize(subLen, 0.0);
        for (int i = 0; i < subLen; ++i) {
            int idx = k + i * m_L;
            if (idx < m_filter.size())
                m_polyFilter[k][i] = m_filter[idx];
        }
    }
}

QVector<double> Resampler::pushSample(double sample)
{
    /* Write to delay line */
    m_delayLine[m_delayIdx] = sample;
    m_delayIdx = (m_delayIdx + 1) % m_delayLine.size();

    QVector<double> outputs;

    /* Generate outputs for all phases of current input */
    while (m_phase < m_L) {
        /* Convolve with polyphase filter for current phase */
        int subLen = m_polyFilter[m_phase].size();
        double y = 0.0;
        for (int i = 0; i < subLen; ++i) {
            int bufPos = (m_delayIdx - 1 - i + m_delayLine.size()) % m_delayLine.size();
            y += m_polyFilter[m_phase][i] * m_delayLine[bufPos];
        }
        outputs.append(y);
        m_phase += m_M;
    }
    m_phase -= m_L;

    m_stats.totalSamplesIn++;
    m_stats.totalSamplesOut += outputs.size();
    return outputs;
}

QVector<double> Resampler::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(input.size() * m_L / qMax(1, m_M) + 16);

    for (double s : input) {
        QVector<double> chunk = pushSample(s);
        for (double v : chunk) output.append(v);
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamplesIn > 0)
        ? m_timeSum / m_stats.totalSamplesIn : 0.0;

    emit processingCompleted(input.size(), output.size());
    return output;
}

QVector<QVector<double>> Resampler::polyphaseFilters() const { return m_polyFilter; }
QVector<double> Resampler::filterCoeffs() const { return m_filter; }

void Resampler::reset()
{
    std::fill(m_delayLine.begin(), m_delayLine.end(), 0.0);
    m_delayIdx = 0;
    m_phase = 0;
}

void Resampler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
