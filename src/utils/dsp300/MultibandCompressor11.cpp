/**
 * @file MultibandCompressor11.cpp
 * @brief MultibandCompressor11 实现
 *
 * 实现多频段压缩器：线性相位交叉与逐频段侧链滤波实现相位相干多频率动态处理。
 */

#include "utils/dsp300/MultibandCompressor11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandCompressor11::MultibandCompressor11(QObject *parent)
    : QObject(parent)
{
    // Default 4-band crossover: 120, 500, 4000 Hz
    m_crossoverFreqs = {120.0, 500.0, 4000.0};
    m_bandParams.resize(4);
    m_bandStates.resize(4);
    m_delayLines.resize(4);
    m_delayPos.resize(4, 0);
    for (int i = 0; i < 4; ++i) {
        m_delayLines[i].resize(m_filterOrder, 0.0);
        m_bandStates[i] = BandState{};
    }
}

MultibandCompressor11::~MultibandCompressor11() = default;

/* ---- Configuration ---- */

void MultibandCompressor11::setSampleRate(double rate)
{
    m_sampleRate = qBound(8000.0, rate, 192000.0);
}

void MultibandCompressor11::setNumBands(int bands)
{
    m_numBands = qBound(2, bands, 16);
    m_bandParams.resize(m_numBands);
    m_bandStates.resize(m_numBands);
    m_delayLines.resize(m_numBands);
    m_delayPos.resize(m_numBands, 0);
    for (int i = 0; i < m_numBands; ++i) {
        m_delayLines[i].resize(m_filterOrder, 0.0);
        m_bandStates[i] = BandState{};
    }
}

void MultibandCompressor11::setCrossoverFreqs(const QVector<double>& freqs)
{
    m_crossoverFreqs = freqs;
}

void MultibandCompressor11::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_numBands)
        m_bandParams[band] = params;
}

/* ---- Design linear-phase crossover FIR via windowed sinc ---- */

QVector<double> MultibandCompressor11::designLPFIR(double cutoffHz, int order) const
{
    int N = order + 1;
    QVector<double> coeffs(N);
    double fc = cutoffHz / m_sampleRate;

    for (int n = 0; n < N; ++n) {
        double shifted = n - order / 2.0;
        double sinc = (qFabs(shifted) < 1e-10) ? 1.0 : qSin(2.0 * M_PI * fc * shifted) / (M_PI * shifted);
        // Hann window for linear phase
        double window = 0.5 * (1.0 - qCos(2.0 * M_PI * n / order));
        coeffs[n] = 2.0 * fc * sinc * window;
    }
    return coeffs;
}

/* ---- Apply FIR with delay line ---- */

double MultibandCompressor11::applyFIR(double sample, int bandIdx, const QVector<double>& coeffs)
{
    auto& dl = m_delayLines[bandIdx];
    int pos = m_delayPos[bandIdx];
    dl[pos] = sample;

    double sum = 0.0;
    int N = coeffs.size();
    for (int i = 0; i < N; ++i) {
        int idx = (pos - i + dl.size()) % dl.size();
        sum += coeffs[i] * dl[idx];
    }

    m_delayPos[bandIdx] = (pos + 1) % dl.size();
    return sum;
}

/* ---- Compute gain reduction ---- */

double MultibandCompressor11::computeGainReduction(double inputDB, const BandParams& params) const
{
    if (inputDB < params.threshold - params.knee / 2.0) return 0.0;

    double slope = 1.0 - 1.0 / params.ratio;
    if (params.knee > 0.0 && inputDB < params.threshold + params.knee / 2.0) {
        // Soft knee interpolation
        double halfKnee = params.knee / 2.0;
        double x = inputDB - params.threshold + halfKnee;
        return slope * x * x / (2.0 * params.knee);
    }
    return slope * (inputDB - params.threshold);
}

/* ---- Sidechain filter emphasis ---- */

double MultibandCompressor11::sidechainFilter(double sample, int bandIdx)
{
    const auto& params = m_bandParams[bandIdx];
    if (params.sidechainFreq <= 0.0) return sample;

    // Simple first-order high-shelf emphasis for sidechain
    double fc = params.sidechainFreq / m_sampleRate;
    double alpha = (1.0 - qCos(2.0 * M_PI * fc)) / (1.0 + qCos(2.0 * M_PI * fc));
    static QVector<double> scPrev;
    if (scPrev.size() != m_numBands) scPrev.resize(m_numBands, 0.0);

    double filtered = alpha * sample + (1.0 - alpha) * scPrev[bandIdx];
    scPrev[bandIdx] = filtered;
    return filtered;
}

/* ---- Update envelope follower ---- */

double MultibandCompressor11::updateEnvelope(double input, double attackCoeff,
                                              double releaseCoeff, double prevEnvelope) const
{
    double absVal = qFabs(input);
    double coeff = (absVal > prevEnvelope) ? attackCoeff : releaseCoeff;
    return coeff * absVal + (1.0 - coeff) * prevEnvelope;
}

/* ---- Main process ---- */

MultibandCompressor11::ProcessResult MultibandCompressor11::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    int n = input.size();
    result.output.resize(n, 0.0);
    result.gainReduction.resize(m_numBands, 0.0);

    // Precompute FIR coefficients for each crossover
    QVector<QVector<double>> lpCoeffs;
    for (int i = 0; i < m_crossoverFreqs.size(); ++i)
        lpCoeffs.append(designLPFIR(m_crossoverFreqs[i], m_filterOrder));

    // Attack/release coefficients in per-sample domain
    QVector<double> attackCoeff(m_numBands);
    QVector<double> releaseCoeff(m_numBands);
    for (int b = 0; b < m_numBands; ++b) {
        double atkMs = qMax(0.01, m_bandParams[b].attack);
        double relMs = qMax(0.01, m_bandParams[b].release);
        attackCoeff[b] = 1.0 - qExp(-1.0 / (atkMs * 0.001 * m_sampleRate));
        releaseCoeff[b] = 1.0 - qExp(-1.0 / (relMs * 0.001 * m_sampleRate));
    }

    double peakOut = 0.0;

    for (int i = 0; i < n; ++i) {
        double sample = input[i];
        double reconstructed = 0.0;

        for (int b = 0; b < m_numBands; ++b) {
            // Crossover filtering
            double bandSample;
            if (b < lpCoeffs.size()) {
                // Low-pass for current band
                double lp = applyFIR(sample, b, lpCoeffs[b]);
                if (b == 0) {
                    bandSample = lp;
                } else {
                    // High-pass = original - low-pass (complementary for linear phase)
                    double prevLP = applyFIR(sample, b, lpCoeffs[b - 1]);
                    bandSample = prevLP - lp;
                }
            } else {
                // Highest band: high-pass from last crossover
                double lastLP = applyFIR(sample, b, lpCoeffs.last());
                bandSample = sample - lastLP;
            }

            // Apply sidechain emphasis
            double scSample = sidechainFilter(bandSample, b);

            // Update envelope
            double& env = m_bandStates[b].envelope;
            env = updateEnvelope(scSample, attackCoeff[b], releaseCoeff[b], env);

            // Convert to dB
            double envDB = (env > 1e-10) ? 20.0 * qLn(env) / M_LN2 * (1.0 / 6.0) : -120.0;
            // Correct: 20*log10(env) = 20*ln(env)/ln(10)
            envDB = (env > 1e-10) ? 20.0 * qLn(env) / qLn(10.0) : -120.0;

            m_bandStates[b].rmsLevel = envDB;

            // Compute gain reduction
            double gr = computeGainReduction(envDB, m_bandParams[b]);
            m_bandStates[b].gainReduction = gr;

            // Apply gain reduction + makeup gain (linear domain)
            double gainLin = qPow(10.0, (gr + m_bandParams[b].gain) / 20.0);
            reconstructed += bandSample * gainLin;
        }

        result.output[i] = reconstructed;
        peakOut = qMax(peakOut, qFabs(reconstructed));
    }

    // Average gain reduction per band
    for (int b = 0; b < m_numBands; ++b)
        result.gainReduction[b] = m_bandStates[b].gainReduction;

    result.peakOut = peakOut;

    m_stats.totalFrames += n;
    m_stats.numBands = m_numBands;
    m_stats.sampleRate = m_sampleRate;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0) ?
        m_timeSum * 1000.0 / m_stats.totalFrames : 0.0;

    double peakGR = 0.0;
    for (double gr : result.gainReduction) peakGR = qMin(peakGR, gr);

    emit processDone(n, peakGR, elapsed);
    return result;
}

/* ---- Reset ---- */

void MultibandCompressor11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    for (int i = 0; i < m_numBands; ++i) {
        m_delayLines[i].fill(0.0);
        m_delayPos[i] = 0;
        m_bandStates[i] = BandState{};
    }
}
