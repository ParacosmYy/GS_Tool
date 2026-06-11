/**
 * @file Resampler11.cpp
 * @brief Resampler11 实现
 *
 * 实现重采样器：多相抗混叠滤波器与sinc插值实现线性相位保持的任意比采样率转换。
 */

#include "utils/signal296/Resampler11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler11::Resampler11(QObject *parent)
    : QObject(parent)
{
    designFilter();
}

Resampler11::~Resampler11() = default;

/* ---- Configuration ---- */

void Resampler11::setRates(double inputRate, double outputRate)
{
    m_inputRate = qBound(8000.0, inputRate, 192000.0);
    m_outputRate = qBound(8000.0, outputRate, 192000.0);
    m_ratio = m_outputRate / m_inputRate;
    m_phaseAccum = 0.0;
    designFilter();
}

void Resampler11::setQuality(Quality quality)
{
    m_quality = quality;
    switch (quality) {
    case Fast:   m_filterLength = 32; m_numPhases = 16; break;
    case Medium: m_filterLength = 64; m_numPhases = 32; break;
    case High:   m_filterLength = 128; m_numPhases = 64; break;
    }
    designFilter();
}

/* ---- Sinc function ---- */

double Resampler11::sinc(double x) const
{
    if (qAbs(x) < 1e-10) return 1.0;
    double px = M_PI * x;
    return qSin(px) / px;
}

/* ---- Blackman window ---- */

double Resampler11::blackman(int n, int N) const
{
    double Nm1 = static_cast<double>(N - 1);
    double w = 0.42 - 0.5 * qCos(2.0 * M_PI * n / Nm1)
               + 0.08 * qCos(4.0 * M_PI * n / Nm1);
    return w;
}

/* ---- Design polyphase filter ---- */

void Resampler11::designFilter()
{
    // Cutoff frequency for anti-aliasing
    // When downsampling: cutoff at outputRate/2
    // When upsampling: cutoff at inputRate/2
    double cutoff;
    if (m_ratio > 1.0)
        cutoff = 1.0 / m_ratio; // Upsampling: cutoff relative to input
    else
        cutoff = m_ratio;       // Downsampling: cutoff relative to input

    cutoff = qBound(0.01, cutoff, 1.0);

    // Design a prototype lowpass filter
    int protoLen = m_filterLength * m_numPhases;
    QVector<double> protoFilter(protoLen, 0.0);

    double sumSq = 0.0;
    for (int i = 0; i < protoLen; ++i) {
        double n = i - (protoLen - 1) / 2.0;
        double h = sinc(n * cutoff) * cutoff * blackman(i, protoLen);
        protoFilter[i] = h;
        sumSq += h * h;
    }

    // Normalize
    double norm = 1.0 / qSqrt(sumSq / protoLen);
    for (int i = 0; i < protoLen; ++i)
        protoFilter[i] *= norm * m_numPhases;

    // Decompose into polyphase components
    m_polyphaseCoeffs.resize(m_numPhases);
    for (int p = 0; p < m_numPhases; ++p) {
        m_polyphaseCoeffs[p].resize(m_filterLength);
        for (int k = 0; k < m_filterLength; ++k) {
            int idx = p + k * m_numPhases;
            if (idx < protoLen)
                m_polyphaseCoeffs[p][k] = protoFilter[idx];
            else
                m_polyphaseCoeffs[p][k] = 0.0;
        }
    }

    // Allocate filter state
    m_filterState.resize(m_filterLength * 2, 0.0);
    m_statePos = 0;
    m_phaseAccum = 0.0;
}

/* ---- Apply polyphase filter ---- */

double Resampler11::applyPolyphase(const QVector<double>& history, int phase) const
{
    const auto& coeffs = m_polyphaseCoeffs[phase % m_numPhases];
    double sum = 0.0;
    int len = qMin(coeffs.size(), history.size());
    for (int i = 0; i < len; ++i)
        sum += coeffs[i] * history[i];
    return sum;
}

/* ---- Process ---- */

Resampler11::ResampleResult Resampler11::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ResampleResult result;
    int n = input.size();
    result.inputSamples = n;

    if (n == 0 || m_ratio <= 0.0) {
        result.actualRatio = m_ratio;
        double elapsed = timer.elapsed();
        emit processDone(0, 0, elapsed);
        return result;
    }

    // Estimate output size
    int outSize = static_cast<int>(qCeil(n * m_ratio)) + m_filterLength;
    result.output.reserve(outSize);

    // Feed input samples through filter state and generate output
    int inputPos = 0;
    while (inputPos < n) {
        // Push input sample into filter state (circular buffer)
        m_filterState[m_statePos % m_filterState.size()] = input[inputPos];
        m_statePos++;

        // Generate output samples at fractional positions
        while (m_phaseAccum < 1.0 && (inputPos < n)) {
            int phase = static_cast<int>(m_phaseAccum * m_numPhases);
            phase = qBound(0, phase, m_numPhases - 1);

            // Gather history for polyphase filter
            QVector<double> history(m_filterLength, 0.0);
            for (int k = 0; k < m_filterLength; ++k) {
                int idx = (m_statePos - 1 - k + m_filterState.size()) % m_filterState.size();
                if (idx >= 0 && idx < m_filterState.size())
                    history[k] = m_filterState[idx];
            }

            double outSample = applyPolyphase(history, phase);
            result.output.append(outSample);

            m_phaseAccum += m_ratio;
        }
        m_phaseAccum -= 1.0;
        inputPos++;
    }

    result.outputSamples = result.output.size();
    result.actualRatio = m_ratio;

    double elapsed = timer.elapsed();
    m_stats.inputRate = m_inputRate;
    m_stats.outputRate = m_outputRate;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit processDone(n, result.outputSamples, elapsed);
    return result;
}

/* ---- Reset state ---- */

void Resampler11::resetState()
{
    m_filterState.fill(0.0);
    m_statePos = 0;
    m_phaseAccum = 0.0;
}

/* ---- Reset statistics ---- */

void Resampler11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
