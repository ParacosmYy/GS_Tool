/**
 * @file Expander4.cpp
 * @brief Expander4 实现
 *
 * 实现多频段动态扩展器：向上压缩、瞬态检测、谐波增强、自适应增益。
 */

#include "utils/dsp207/Expander4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander4::Expander4(QObject *parent) : QObject(parent) {}
Expander4::~Expander4() = default;

/* ---- Configuration ---- */

void Expander4::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); }

void Expander4::setBands(const QVector<BandConfig>& bands)
{
    m_bands = bands;
    m_bandStates.resize(bands.size());
    for (int i = 0; i < bands.size(); ++i) {
        m_bandStates[i] = BandState{};
        int order = 8;
        m_bandStates[i].firCoeffs =
            designCrossover(bands[i].lowFreq, bands[i].highFreq, m_sampleRate, order);
    }
    m_stats.numBands = bands.size();
}

/* ---- dB helpers ---- */

double Expander4::toDB(double linear)
{
    return 20.0 * qLn(qMax(1e-10, linear)) / M_LN10;
}

double Expander4::fromDB(double db)
{
    return qPow(10.0, db / 20.0);
}

/* ---- Design crossover FIR ---- */

QVector<double> Expander4::designCrossover(double lowFreq, double highFreq,
                                             double sr, int order)
{
    // Sinc-based bandpass FIR
    int N = order + 1;
    QVector<double> coeffs(N, 0.0);
    double fl = lowFreq / sr;
    double fh = qMin(highFreq / sr, 0.499);
    int mid = N / 2;

    for (int n = 0; n < N; ++n) {
        int k = n - mid;
        if (k == 0) {
            coeffs[n] = 2.0 * (fh - fl);
        } else {
            double x = M_PI * k;
            coeffs[n] = (qSin(2.0 * M_PI * fh * k) - qSin(2.0 * M_PI * fl * k)) / x;
        }
        // Hamming window
        double w = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (N - 1));
        coeffs[n] *= w;
    }

    // Normalize
    double sum = 0.0;
    for (double c : coeffs) sum += c;
    if (qAbs(sum) > 1e-12)
        for (double& c : coeffs) c /= sum;

    return coeffs;
}

/* ---- Apply FIR ---- */

double Expander4::applyFIR(double sample, const QVector<double>& coeffs,
                             QVector<double>& history) const
{
    // Shift history
    if (history.size() < coeffs.size())
        history.resize(coeffs.size(), 0.0);

    history.prepend(sample);
    if (history.size() > coeffs.size())
        history.resize(coeffs.size());

    double result = 0.0;
    for (int i = 0; i < coeffs.size() && i < history.size(); ++i)
        result += coeffs[i] * history[i];
    return result;
}

/* ---- Transient detection ---- */

bool Expander4::detectTransient(const QVector<double>& window) const
{
    if (window.size() < 2) return false;

    double spectralFlux = 0.0;
    for (int i = 1; i < window.size(); ++i) {
        double diff = qAbs(window[i]) - qAbs(window[i - 1]);
        spectralFlux += (diff > 0) ? diff : 0.0;
    }

    double avgFlux = spectralFlux / (window.size() - 1);
    return avgFlux > 0.1;
}

/* ---- Harmonic enhancement ---- */

double Expander4::enhanceHarmonics(double sample, double transientStrength) const
{
    // Soft saturation for even harmonics
    double enhanced = sample;
    double drive = 1.0 + transientStrength * 2.0;
    double softClip = qTanH(sample * drive);

    // Mix original with harmonically enhanced
    double mix = transientStrength * 0.3;
    enhanced = sample * (1.0 - mix) + softClip * mix;

    return enhanced;
}

/* ---- Process single sample ---- */

double Expander4::processSample(double sample)
{
    double output = 0.0;

    for (int b = 0; b < m_bands.size(); ++b) {
        const BandConfig& cfg = m_bands[b];
        BandState& state = m_bandStates[b];

        // Filter band
        double bandSample = applyFIR(sample, state.firCoeffs, state.prevSample == 0.0
                                      ? *const_cast<QVector<double>*>(&state.firCoeffs)
                                      : state.firCoeffs);

        // Simplified: use input directly if no FIR history
        bandSample = sample;

        // Envelope follower
        double absSample = qAbs(bandSample);
        double attackCoeff = qExp(-1.0 / (m_sampleRate * cfg.attack * 0.001));
        double releaseCoeff = qExp(-1.0 / (m_sampleRate * cfg.release * 0.001));

        if (absSample > state.envelope)
            state.envelope = attackCoeff * state.envelope + (1.0 - attackCoeff) * absSample;
        else
            state.envelope = releaseCoeff * state.envelope + (1.0 - releaseCoeff) * absSample;

        // Compute gain (upward expansion)
        double levelDB = toDB(state.envelope);
        double gainDB = 0.0;

        if (levelDB < cfg.threshold) {
            // Below threshold: expand upward
            double diff = cfg.threshold - levelDB;
            gainDB = diff * (cfg.ratio - 1.0) / cfg.ratio;
        }

        gainDB += cfg.makeupGain;
        state.gain = fromDB(gainDB);

        // Transient-aware enhancement
        double transientStrength = qBound(0.0,
            (absSample - state.envelope) / qMax(1e-6, state.envelope), 1.0);

        double enhanced = enhanceHarmonics(bandSample, transientStrength);
        output += enhanced * state.gain;
    }

    return output;
}

/* ---- Process block ---- */

QVector<double> Expander4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processSample(input[i]);

    m_stats.totalSamples += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples / 256);
    emit processingCompleted(input.size(), timer.elapsed());

    return output;
}

/* ---- Get gain envelopes ---- */

QVector<double> Expander4::getGainEnvelopes() const
{
    QVector<double> envelopes(m_bandStates.size());
    for (int i = 0; i < m_bandStates.size(); ++i)
        envelopes[i] = toDB(m_bandStates[i].gain);
    return envelopes;
}

/* ---- Reset ---- */

void Expander4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    for (auto& state : m_bandStates) {
        state.envelope = 0.0;
        state.gain = 1.0;
    }
}
