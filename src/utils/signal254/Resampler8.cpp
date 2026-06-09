/**
 * @file Resampler8.cpp
 * @brief Resampler8 实现
 *
 * 实现重采样器：窗函数sinc插值与多相滤波器组。
 */

#include "utils/signal254/Resampler8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler8::Resampler8(QObject *parent) : QObject(parent) {}
Resampler8::~Resampler8() = default;

/* ---- Configuration ---- */

void Resampler8::setWindow(Window win) { m_window = win; }

/* ---- Design windowed sinc lowpass filter ---- */

QVector<double> Resampler8::designSincFilter() const
{
    // Cutoff frequency: min(1/inputRate, 1/outputRate) * 0.5
    double cutoff = qMin(1.0, 1.0 / m_ratio) * 0.5;
    int N = m_filterTaps;
    int halfN = N / 2;

    QVector<double> filter(N, 0.0);
    for (int n = 0; n < N; ++n) {
        double t = static_cast<double>(n) - halfN;
        double sinc = (qAbs(t) < 1e-10) ? 1.0 : qSin(2.0 * M_PI * cutoff * t) / (M_PI * t);
        filter[n] = sinc * applyWindow(n, N);
    }

    // Normalize to unit energy
    double energy = 0.0;
    for (double v : filter) energy += v * v;
    energy = qSqrt(energy);
    if (energy > 1e-15)
        for (auto& v : filter) v /= energy;

    return filter;
}

/* ---- Apply window function ---- */

double Resampler8::applyWindow(int n, int N) const
{
    double w = 1.0;
    double x = static_cast<double>(n) / (N - 1);

    switch (m_window) {
    case Blackman:
        w = 0.42 - 0.5 * qCos(2.0 * M_PI * x) + 0.08 * qCos(4.0 * M_PI * x);
        break;
    case Hann:
        w = 0.5 * (1.0 - qCos(2.0 * M_PI * x));
        break;
    case Lanczos:
        w = (n == N / 2) ? 1.0 : qSin(M_PI * (2.0 * x - 1.0)) / (M_PI * (2.0 * x - 1.0));
        break;
    case Kaiser:
        // Simplified Kaiser with beta=8
        w = 1.0 / (1.0 + 64.0 * (x - 0.5) * (x - 0.5));
        break;
    }
    return w;
}

/* ---- Build polyphase filter bank ---- */

void Resampler8::buildPolyPhase(const QVector<double>& filter)
{
    int N = filter.size();
    m_polyPhase.resize(m_numPhases);

    for (int p = 0; p < m_numPhases; ++p) {
        m_polyPhase[p].resize(N, 0.0);
        double offset = static_cast<double>(p) / m_numPhases;
        for (int n = 0; n < N; ++n) {
            // Interpolate filter coefficient at fractional position
            double idx = n + offset;
            int idx0 = qFloor(idx);
            double frac = idx - idx0;
            double v0 = (idx0 >= 0 && idx0 < N) ? filter[idx0] : 0.0;
            double v1 = (idx0 + 1 >= 0 && idx0 + 1 < N) ? filter[idx0 + 1] : 0.0;
            m_polyPhase[p][n] = v0 * (1.0 - frac) + v1 * frac;
        }
    }
}

/* ---- Interpolate one output sample ---- */

double Resampler8::interpolate(int phaseIndex) const
{
    int N = m_filterTaps;
    double sum = 0.0;
    for (int n = 0; n < N; ++n) {
        int dlIdx = (m_delayPos - n + m_delayLine.size()) % m_delayLine.size();
        sum += m_polyPhase[phaseIndex][n] * m_delayLine[dlIdx];
    }
    return sum;
}

/* ---- Prepare resampler ---- */

bool Resampler8::prepare(int inputRate, int outputRate, int filterTaps)
{
    if (inputRate <= 0 || outputRate <= 0) return false;

    m_inputRate = inputRate;
    m_outputRate = outputRate;
    m_filterTaps = qMax(8, filterTaps);
    m_ratio = static_cast<double>(outputRate) / inputRate;

    if (m_ratio < 0.01 || m_ratio > 100.0) return false;

    m_phaseStep = 1.0 / m_ratio;
    m_phase = 0.0;

    // Determine number of phases based on ratio
    m_numPhases = qBound(16, qRound(m_ratio * 32), 256);

    // Design filter and build polyphase bank
    QVector<double> filter = designSincFilter();
    buildPolyPhase(filter);

    // Initialize delay line
    m_delayLine.resize(m_filterTaps, 0.0);
    m_delayPos = 0;

    m_stats.inputRate = inputRate;
    m_stats.outputRate = outputRate;
    m_stats.filterLength = m_filterTaps;
    m_stats.numPhases = m_numPhases;
    return true;
}

/* ---- Process input samples ---- */

QVector<double> Resampler8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int inN = input.size();
    if (inN == 0 || m_polyPhase.isEmpty()) return {};

    // Estimate output size
    int estOut = qCeil(inN * m_ratio) + m_filterTaps;
    QVector<double> output;
    output.reserve(estOut);

    for (int i = 0; i < inN; ++i) {
        // Push input sample into delay line
        m_delayLine[m_delayPos] = input[i];
        m_delayPos = (m_delayPos + 1) % m_delayLine.size();

        // Generate output samples while phase allows
        while (m_phase < 1.0) {
            int phaseIdx = qRound(m_phase * m_numPhases) % m_numPhases;
            output.append(interpolate(phaseIdx));
            m_phase += m_phaseStep;
        }
        m_phase -= 1.0;
    }

    m_stats.inputSamples += inN;
    m_stats.outputSamples += output.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processCompleted(inN, output.size(), timer.elapsed());
    return output;
}

/* ---- Flush remaining samples ---- */

QVector<double> Resampler8::flush()
{
    QVector<double> output;
    int remaining = m_filterTaps;
    QVector<double> zeros(remaining, 0.0);
    output = process(zeros);

    // Reset phase for next stream
    m_phase = 0.0;
    m_delayLine.fill(0.0);
    m_delayPos = 0;

    return output;
}

/* ---- Get current phase ---- */

double Resampler8::phase() const { return m_phase; }

/* ---- Reset ---- */

void Resampler8::resetStatistics()
{
    m_polyPhase.clear();
    m_delayLine.clear();
    m_phase = 0.0;
    m_delayPos = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
