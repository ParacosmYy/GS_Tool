/**
 * @file Resampler6.cpp
 * @brief Resampler6 实现
 *
 * 实现重采样器：有理P/Q多相FIR、CIC预滤波抗混叠。
 */

#include "utils/signal226/Resampler6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler6::Resampler6(QObject *parent) : QObject(parent) {}
Resampler6::~Resampler6() = default;

/* ---- GCD ---- */

int Resampler6::gcd(int a, int b)
{
    a = qAbs(a); b = qAbs(b);
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a;
}

/* ---- Configure ---- */

bool Resampler6::configure(int inputRate, int outputRate, int numTaps, int cicOrder)
{
    if (inputRate <= 0 || outputRate <= 0) return false;

    m_inputRate = inputRate;
    m_outputRate = outputRate;
    m_numTaps = qMax(8, numTaps);
    m_cicOrder = qMax(1, cicOrder);

    int g = gcd(outputRate, inputRate);
    m_up = outputRate / g;
    m_down = inputRate / g;

    m_stats.inputRate = inputRate;
    m_stats.outputRate = outputRate;
    m_stats.upFactor = m_up;
    m_stats.downFactor = m_down;

    // Design filters
    designPolyphaseFilter();
    designCICFilter();

    // Allocate delay line
    int tapsPerPhase = m_polyFilter.isEmpty() ? m_numTaps : m_polyFilter[0].size();
    m_delayLine.resize(tapsPerPhase, 0.0);
    m_delayPos = 0;
    m_fracPhase = 0.0;

    m_stats.filterTaps = m_numTaps;
    m_stats.cicOrder = m_cicOrder;
    return true;
}

/* ---- Design polyphase FIR ---- */

void Resampler6::designPolyphaseFilter()
{
    // Filter length = numTaps * up
    int totalTaps = m_numTaps * m_up;
    double cutoff = qMin(1.0 / (double)m_up, 1.0 / (double)m_down);

    // Sinc-based lowpass filter with Hamming window
    QVector<double> protoFilter(totalTaps);
    double sum = 0.0;
    for (int n = 0; n < totalTaps; ++n) {
        double t = n - (totalTaps - 1) / 2.0;
        double sinc = (qFuzzyCompare(t, 0.0)) ? 1.0 : qSin(M_PI * cutoff * t) / (M_PI * t);
        // Hamming window
        double window = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (totalTaps - 1));
        protoFilter[n] = sinc * window * m_up; // Gain compensation
        sum += protoFilter[n];
    }

    // Normalize
    for (int n = 0; n < totalTaps; ++n)
        protoFilter[n] /= sum;

    // Decompose into polyphase branches
    int tapsPerPhase = (totalTaps + m_up - 1) / m_up;
    m_polyFilter.resize(m_up);
    for (int p = 0; p < m_up; ++p) {
        m_polyFilter[p].resize(tapsPerPhase, 0.0);
        for (int k = 0; k < tapsPerPhase; ++k) {
            int idx = p + k * m_up;
            if (idx < totalTaps)
                m_polyFilter[p][k] = protoFilter[idx];
        }
    }
}

/* ---- Design CIC pre-filter ---- */

void Resampler6::designCICFilter()
{
    m_integratorState.resize(m_cicOrder, 0.0);
    m_combState = 0.0;
    m_cicPhase = 0;
}

/* ---- Apply CIC ---- */

double Resampler6::applyCIC(double sample)
{
    // Integrator stages (running sum)
    double val = sample;
    for (int i = 0; i < m_cicOrder; ++i) {
        m_integratorState[i] += val;
        val = m_integratorState[i];
    }

    // Comb stage (difference)
    double output = val - m_combState;
    m_combState = val;

    // CIC gain compensation
    double gain = qPow(m_down, m_cicOrder);
    return output / gain;
}

/* ---- Apply polyphase ---- */

double Resampler6::applyPolyphase(int phase, double newSample)
{
    // Push into delay line
    m_delayLine[m_delayPos] = newSample;
    m_delayPos = (m_delayPos + 1) % m_delayLine.size();

    // Convolve with polyphase branch
    const QVector<double>& coeffs = m_polyFilter[phase % m_up];
    double output = 0.0;
    int delayLen = m_delayLine.size();
    for (int k = 0; k < coeffs.size() && k < delayLen; ++k) {
        int idx = (m_delayPos - 1 - k + delayLen) % delayLen;
        output += coeffs[k] * m_delayLine[idx];
    }
    return output;
}

/* ---- Process block ---- */

QVector<double> Resampler6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;

    for (int i = 0; i < input.size(); ++i) {
        // Step 1: CIC pre-filter for anti-aliasing (before upsampling)
        double cicOut = applyCIC(input[i]);

        // Step 2: Upsample by P and apply polyphase FIR
        for (int p = 0; p < m_up; ++p) {
            double filtered = applyPolyphase(p, (p == 0) ? cicOut : 0.0);

            // Step 3: Accumulate fractional phase and decimate by Q
            m_fracPhase += 1.0;
            if (m_fracPhase >= m_down) {
                m_fracPhase -= m_down;
                output.append(filtered);
            }
        }
    }

    m_stats.inputSamples += input.size();
    m_stats.outputSamples += output.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit resamplingCompleted(input.size(), output.size(), timer.elapsed());
    return output;
}

/* ---- Flush ---- */

QVector<double> Resampler6::flush()
{
    // Process tail zeros to drain delay line
    int drainLength = m_delayLine.size();
    QVector<double> zeros(drainLength, 0.0);
    return process(zeros);
}

/* ---- Reset ---- */

void Resampler6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_polyFilter.clear();
    m_delayLine.clear();
    m_integratorState.clear();
    m_delayPos = 0;
    m_fracPhase = 0.0;
    m_combState = 0.0;
}
