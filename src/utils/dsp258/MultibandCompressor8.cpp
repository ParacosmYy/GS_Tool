/**
 * @file MultibandCompressor8.cpp
 * @brief MultibandCompressor8 实现
 *
 * 实现多段压缩器：线性相位交叉与逐带动态范围控制。
 */

#include "utils/dsp258/MultibandCompressor8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MultibandCompressor8::MultibandCompressor8(QObject *parent)
    : QObject(parent)
{
    m_numBands = 4;
    m_bandParams.resize(m_numBands);
    m_envelope.resize(m_numBands, 0.0);
    m_gainState.resize(m_numBands, 1.0);
    m_bandLevels.resize(m_numBands, -120.0);
    m_gainReduction.resize(m_numBands, 0.0);
}

MultibandCompressor8::~MultibandCompressor8() = default;

/* ---- Configuration ---- */

void MultibandCompressor8::setNumBands(int bands)
{
    m_numBands = qBound(2, bands, 8);
    m_bandParams.resize(m_numBands);
    m_envelope.resize(m_numBands, 0.0);
    m_gainState.resize(m_numBands, 1.0);
    m_bandLevels.resize(m_numBands, -120.0);
    m_gainReduction.resize(m_numBands, 0.0);
    designCrossoverFilters();
}

void MultibandCompressor8::setCrossoverFreqs(const QVector<double>& freqs)
{
    m_crossoverFreqs = freqs;
    designCrossoverFilters();
}

void MultibandCompressor8::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_numBands)
        m_bandParams[band] = params;
}

void MultibandCompressor8::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    designCrossoverFilters();
}

/* ---- Design linear-phase crossover FIR filters ---- */

void MultibandCompressor8::designCrossoverFilters()
{
    int numCrossovers = m_numBands - 1;
    if (numCrossovers <= 0 || m_crossoverFreqs.size() < numCrossovers) return;

    // Windowed-sinc FIR for each crossover (linear phase)
    int filterLen = 65; // Must be odd for linear phase
    int halfLen = filterLen / 2;

    m_lowpassCoeffs.resize(numCrossovers);
    m_highpassCoeffs.resize(numCrossovers);

    for (int x = 0; x < numCrossovers; ++x) {
        double fc = m_crossoverFreqs[x] / m_sampleRate;
        m_lowpassCoeffs[x].resize(filterLen);
        m_highpassCoeffs[x].resize(filterLen);

        double sumLP = 0.0;
        for (int n = 0; n < filterLen; ++n) {
            int center = n - halfLen;
            double sinc;
            if (center == 0)
                sinc = 2.0 * fc;
            else
                sinc = qSin(2.0 * M_PI * fc * center) / (M_PI * center);

            // Blackman window
            double win = 0.42 - 0.5 * qCos(2.0 * M_PI * n / (filterLen - 1))
                         + 0.08 * qCos(4.0 * M_PI * n / (filterLen - 1));

            m_lowpassCoeffs[x][n] = sinc * win;
            sumLP += m_lowpassCoeffs[x][n];
        }

        // Normalize lowpass
        for (int n = 0; n < filterLen; ++n)
            m_lowpassCoeffs[x][n] /= sumLP;

        // Highpass = complement
        for (int n = 0; n < filterLen; ++n)
            m_highpassCoeffs[x][n] = (n == halfLen) ? 1.0 - m_lowpassCoeffs[x][n]
                                                     : -m_lowpassCoeffs[x][n];
    }

    // Initialize delay lines
    m_delayLines.resize(m_numBands);
    m_delayIdx.resize(m_numBands, 0);
    for (int b = 0; b < m_numBands; ++b) {
        m_delayLines[b].resize(filterLen, 0.0);
    }
}

/* ---- Apply FIR filter ---- */

QVector<double> MultibandCompressor8::applyFIR(const QVector<double>& input,
                                                int bandIdx, bool lowpass)
{
    if (m_crossoverFreqs.isEmpty()) return input;

    int xoverIdx = qMin(bandIdx, m_lowpassCoeffs.size() - 1);
    if (xoverIdx < 0) return input;

    const QVector<double>& coeffs = lowpass ? m_lowpassCoeffs[xoverIdx]
                                             : m_highpassCoeffs[xoverIdx];
    int filterLen = coeffs.size();
    if (filterLen == 0) return input;

    QVector<double> output(input.size(), 0.0);
    for (int i = 0; i < input.size(); ++i) {
        // Shift into delay line
        m_delayLines[bandIdx][m_delayIdx[bandIdx]] = input[i];
        double sum = 0.0;
        int idx = m_delayIdx[bandIdx];
        for (int j = 0; j < filterLen; ++j) {
            sum += m_delayLines[bandIdx][idx] * coeffs[j];
            idx = (idx > 0) ? idx - 1 : filterLen - 1;
        }
        output[i] = sum;
        m_delayIdx[bandIdx] = (m_delayIdx[bandIdx] + 1) % filterLen;
    }
    return output;
}

/* ---- Compute gain for compression ---- */

double MultibandCompressor8::computeGain(int band, double inputLevel) const
{
    if (band < 0 || band >= m_numBands) return 1.0;

    const BandParams& p = m_bandParams[band];
    double threshLin = qPow(10.0, p.threshold / 20.0);
    double halfKnee = p.kneeWidth / 2.0;

    double outputLevel = inputLevel;
    if (inputLevel > 0.0) {
        double inputDb = 20.0 * qLog10(inputLevel);
        if (inputDb < (p.threshold - halfKnee)) {
            outputLevel = inputLevel;
        } else if (inputDb > (p.threshold + halfKnee)) {
            double overDb = inputDb - p.threshold;
            outputLevel = threshLin * qPow(10.0, overDb / p.ratio / 20.0);
        } else {
            // Soft knee interpolation
            double x = inputDb - p.threshold + halfKnee;
            double kneeFactor = (x * x) / (2.0 * p.kneeWidth);
            double gainDb = kneeFactor * (1.0 / p.ratio - 1.0);
            outputLevel = inputLevel * qPow(10.0, gainDb / 20.0);
        }
    }
    return (inputLevel > 1e-10) ? outputLevel / inputLevel : 1.0;
}

/* ---- Auto makeup gain ---- */

double MultibandCompressor8::autoMakeupGain(int band) const
{
    if (band < 0 || band >= m_numBands) return 0.0;
    const BandParams& p = m_bandParams[band];
    if (p.ratio <= 1.0) return 0.0;
    // Estimated gain reduction at threshold
    double reductionDb = (p.threshold - p.threshold / p.ratio);
    return -reductionDb * 0.5; // Half of lost gain
}

/* ---- Process block ---- */

QVector<double> MultibandCompressor8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return input;
    int n = input.size();
    m_blockSize = n;

    // Split into bands via crossover
    QVector<QVector<double>> bands(m_numBands);
    bands[0] = input;

    // Cascaded crossover: repeatedly split highest band
    for (int b = 0; b < m_numBands - 1 && b < m_lowpassCoeffs.size(); ++b) {
        bands[b] = applyFIR(bands[b], b, true);
        if (b + 1 < m_numBands)
            bands[b + 1] = applyFIR(input, b + 1, false);
    }

    // Compress each band
    QVector<double> output(n, 0.0);
    for (int b = 0; b < m_numBands; ++b) {
        const BandParams& p = m_bandParams[b];
        double attackCoeff = qExp(-1.0 / (m_sampleRate * p.attack / 1000.0));
        double releaseCoeff = qExp(-1.0 / (m_sampleRate * p.release / 1000.0));

        double rmsSum = 0.0;
        double totalGainReduction = 0.0;

        for (int i = 0; i < n; ++i) {
            double absVal = qFabs(bands[b][i]);
            // Envelope follower
            if (absVal > m_envelope[b])
                m_envelope[b] = attackCoeff * m_envelope[b] + (1.0 - attackCoeff) * absVal;
            else
                m_envelope[b] = releaseCoeff * m_envelope[b] + (1.0 - releaseCoeff) * absVal;

            // Compute gain
            double gain = computeGain(b, m_envelope[b]);
            double makeup = p.makeupGain > -120.0
                                ? p.makeupGain
                                : autoMakeupGain(b);
            double makeupLin = qPow(10.0, makeup / 20.0);

            output[i] += bands[b][i] * gain * makeupLin;
            rmsSum += bands[b][i] * bands[b][i];
            totalGainReduction += 20.0 * qLog10(qMax(gain, 1e-10));
        }

        m_bandLevels[b] = (rmsSum > 0.0) ? 10.0 * qLog10(rmsSum / n) : -120.0;
        m_gainReduction[b] = totalGainReduction / n;
    }

    double elapsed = timer.elapsed();
    m_stats.framesProcessed += n;
    m_stats.numBands = m_numBands;
    m_stats.blockSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, elapsed);
    return output;
}

/* ---- Accessors ---- */

QVector<double> MultibandCompressor8::bandLevels() const { return m_bandLevels; }
QVector<double> MultibandCompressor8::gainReduction() const { return m_gainReduction; }

/* ---- Reset ---- */

void MultibandCompressor8::resetStatistics()
{
    m_envelope.fill(0.0);
    m_gainState.fill(1.0);
    m_bandLevels.fill(-120.0);
    m_gainReduction.fill(0.0);
    for (auto& dl : m_delayLines)
        std::fill(dl.begin(), dl.end(), 0.0);
    m_delayIdx.fill(0);
    m_stats = Stats{};
    m_timeSum = 0.0;
}
