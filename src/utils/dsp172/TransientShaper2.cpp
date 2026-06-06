/**
 * @file TransientShaper2.cpp
 * @brief TransientShaper2 实现
 *
 * 实现瞬态塑形器：包络跟随、多频带分解、瞬态/稳态增益控制。
 */

#include "utils/dsp172/TransientShaper2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransientShaper2::TransientShaper2(QObject *parent)
    : QObject(parent)
{
    reset();
}

TransientShaper2::~TransientShaper2() = default;

/* ---- Configuration ---- */

void TransientShaper2::setAttack(double ms) { m_attackMs = qMax(0.1, ms); }
void TransientShaper2::setSustain(double ms) { m_sustainMs = qMax(1.0, ms); }
void TransientShaper2::setTransientGain(double gainDb) { m_transientGainDb = gainDb; }
void TransientShaper2::setSustainGain(double gainDb) { m_sustainGainDb = gainDb; }

void TransientShaper2::setBandCount(int bands)
{
    m_bandCount = qBound(1, bands, 8);
    reset();
}

/* ---- Time constant coefficient ---- */

double TransientShaper2::timeConstant(double ms, double sampleRate)
{
    if (ms <= 0.0) return 0.0;
    return qExp(-1.0 / (ms * 0.001 * sampleRate));
}

/* ---- Envelope follower ---- */

double TransientShaper2::followEnvelope(double input, EnvelopeState& state)
{
    double abs = qFabs(input);
    double coeff = (abs > state.envelope) ? state.attackCoeff : state.releaseCoeff;
    state.envelope = coeff * state.envelope + (1.0 - coeff) * abs;
    return state.envelope;
}

/* ---- Bandpass filter (2nd order Butterworth) ---- */

double TransientShaper2::bandpass(double sample, BandState& band, double sr)
{
    if (sr <= 0.0) return sample;

    double fLow = band.lowCut;
    double fHigh = band.highCut;
    if (fHigh <= fLow) return sample;

    double fc = qSqrt(fLow * fHigh);
    double bw = fHigh - fLow;
    double omega = 2.0 * M_PI * fc / sr;
    double alpha = qSin(omega) * bw / (2.0 * fc);

    double b0 = alpha;
    double b1 = 0.0;
    double b2 = -alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * qCos(omega);
    double a2 = 1.0 - alpha;

    double y = (b0 * sample + b1 * band.x1 + b2 * band.x2
                - a1 * band.y1 - a2 * band.y2) / a0;

    band.x2 = band.x1;
    band.x1 = sample;
    band.y2 = band.y1;
    band.y1 = y;
    return y;
}

/* ---- Process single frame ---- */

QVector<double> TransientShaper2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int channels = input.size();
    QVector<double> output(channels, 0.0);

    /* Initialize bands if needed */
    if (m_bands.size() != m_bandCount) {
        m_bands.resize(m_bandCount);
        /* Distribute bands logarithmically from 80Hz to 16kHz */
        double logLow = qLn(80.0);
        double logHigh = qLn(16000.0);
        double step = (logHigh - logLow) / m_bandCount;
        for (int b = 0; b < m_bandCount; ++b) {
            m_bands[b].lowCut = qExp(logLow + b * step);
            m_bands[b].highCut = qExp(logLow + (b + 1) * step);
            m_bands[b].x1 = m_bands[b].x2 = 0.0;
            m_bands[b].y1 = m_bands[b].y2 = 0.0;
            m_bands[b].env.envelope = 0.0;
        }
    }

    /* Update envelope coefficients */
    m_transientEnv.attackCoeff = timeConstant(m_attackMs, m_sampleRate);
    m_transientEnv.releaseCoeff = timeConstant(m_attackMs * 2.0, m_sampleRate);
    m_sustainEnv.attackCoeff = timeConstant(m_sustainMs, m_sampleRate);
    m_sustainEnv.releaseCoeff = timeConstant(m_sustainMs * 4.0, m_sampleRate);

    /* Multiband transient detection */
    double transientSum = 0.0;
    double sustainSum = 0.0;

    for (int ch = 0; ch < channels; ++ch) {
        double mono = qFabs(input[ch]);

        /* Quick transient envelope */
        double transEnv = followEnvelope(mono, m_transientEnv);

        /* Slow sustain envelope */
        double sustEnv = followEnvelope(mono, m_sustainEnv);

        transientSum += transEnv;
        sustainSum += sustEnv;
    }

    /* Normalize */
    double avgTransient = (channels > 0) ? transientSum / channels : 0.0;
    double avgSustain = (channels > 0) ? sustainSum / channels : 0.0;

    /* Compute transient amount: ratio of fast to slow envelope */
    double transientAmount = 0.0;
    if (avgSustain > 1e-10)
        transientAmount = qMax(0.0, avgTransient / avgSustain - 1.0);
    transientAmount = qMin(transientAmount, 4.0);

    /* Compute gain */
    double transientGain = qPow(10.0, m_transientGainDb / 20.0);
    double sustainGain = qPow(10.0, m_sustainGainDb / 20.0);

    double gain = (transientAmount * transientGain
                   + (1.0 - transientAmount) * sustainGain);

    /* Apply gain per channel */
    for (int ch = 0; ch < channels; ++ch)
        output[ch] = input[ch] * gain;

    /* Record envelope history */
    m_transientHistory.append(avgTransient);
    m_sustainHistory.append(avgSustain);
    if (m_transientHistory.size() > 4096) {
        m_transientHistory.remove(0, 2048);
        m_sustainHistory.remove(0, 2048);
    }

    m_stats.totalFrames++;
    m_stats.lastPeakTransient = qMax(m_stats.lastPeakTransient, transientAmount);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit frameProcessed(transientAmount);
    return output;
}

/* ---- Process buffer ---- */

void TransientShaper2::processBuffer(QVector<double>& buffer, int channels)
{
    int frames = buffer.size() / channels;
    for (int f = 0; f < frames; ++f) {
        QVector<double> frame(channels);
        for (int ch = 0; ch < channels; ++ch)
            frame[ch] = buffer[f * channels + ch];

        frame = process(frame);

        for (int ch = 0; ch < channels; ++ch)
            buffer[f * channels + ch] = frame[ch];
    }
}

/* ---- Accessors ---- */

QVector<double> TransientShaper2::transientEnvelope() const
{
    return m_transientHistory;
}

QVector<double> TransientShaper2::sustainEnvelope() const
{
    return m_sustainHistory;
}

/* ---- Reset ---- */

void TransientShaper2::reset()
{
    m_transientEnv = {};
    m_sustainEnv = {};
    m_bands.clear();
    m_transientHistory.clear();
    m_sustainHistory.clear();
    m_stats.lastPeakTransient = 0.0;
}

/* ---- Statistics ---- */

void TransientShaper2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
