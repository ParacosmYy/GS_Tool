/**
 * @file MultibandCompressor2.cpp
 * @brief MultibandCompressor2 实现
 *
 * 实现4频带动态压缩器：Linkwitz-Riley交叉滤波、增益包络、软拐点压缩。
 */

#include "utils/dsp171/MultibandCompressor2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MultibandCompressor2::MultibandCompressor2(QObject *parent)
    : QObject(parent)
{
    /* Design initial crossovers */
    for (int i = 0; i < 3; ++i)
        designCrossover(m_lp[i], m_hp[i], m_xoFreqs[i]);
}

MultibandCompressor2::~MultibandCompressor2() = default;

/* ---- Configuration ---- */

void MultibandCompressor2::setSampleRate(int rate)
{
    m_sampleRate = qMax(1, rate);
    m_stats.sampleRate = m_sampleRate;
    for (int i = 0; i < 3; ++i)
        designCrossover(m_lp[i], m_hp[i], m_xoFreqs[i]);

    /* Recalculate envelope coefficients */
    for (int b = 0; b < 4; ++b) {
        double attackMs = m_bands[b].attack;
        double releaseMs = m_bands[b].release;
        m_env[b].attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * attackMs * 0.001));
        m_env[b].releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * releaseMs * 0.001));
    }
}

void MultibandCompressor2::setCrossoverFrequencies(double lowMid, double midHigh1, double high1High2)
{
    m_xoFreqs[0] = qBound(20.0, lowMid, m_sampleRate / 2.0 - 1.0);
    m_xoFreqs[1] = qBound(m_xoFreqs[0] + 1.0, midHigh1, m_sampleRate / 2.0 - 1.0);
    m_xoFreqs[2] = qBound(m_xoFreqs[1] + 1.0, high1High2, m_sampleRate / 2.0 - 1.0);
    for (int i = 0; i < 3; ++i)
        designCrossover(m_lp[i], m_hp[i], m_xoFreqs[i]);
}

void MultibandCompressor2::setBandParams(int band, const BandParams& params)
{
    if (band < 0 || band >= 4) return;
    m_bands[band] = params;
    m_env[band].attackCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * params.attack * 0.001));
    m_env[band].releaseCoeff = 1.0 - qExp(-1.0 / (m_sampleRate * params.release * 0.001));
}

/* ---- Crossover design (2nd-order Butterworth, cascaded for LR4) ---- */

void MultibandCompressor2::designCrossover(CrossoverFilter& lp, CrossoverFilter& hp, double freq)
{
    double omega = 2.0 * M_PI * freq / m_sampleRate;
    double sinW = qSin(omega);
    double cosW = qCos(omega);
    double alpha = sinW / (qSqrt(2.0)); /* Butterworth Q = 0.7071 */

    double a0 = 1.0 + alpha;
    lp.a0 = 1.0 / a0;
    lp.a1 = -2.0 * cosW / a0;
    lp.a2 = (1.0 - alpha) / a0;
    lp.b0 = (1.0 - cosW) / 2.0 / a0;
    lp.b1 = (1.0 - cosW) / a0;
    lp.b2 = lp.b0;

    hp.a0 = 1.0 / a0;
    hp.a1 = -2.0 * cosW / a0;
    hp.a2 = (1.0 - alpha) / a0;
    hp.b0 = (1.0 + cosW) / 2.0 / a0;
    hp.b1 = -(1.0 + cosW) / a0;
    hp.b2 = hp.b0;
}

double MultibandCompressor2::filterProcess(CrossoverFilter& f, double x)
{
    double y = f.b0 * x + f.b1 * f.x1 + f.b2 * f.x2
               - f.a1 * f.y1 - f.a2 * f.y2;
    f.x2 = f.x1; f.x1 = x;
    f.y2 = f.y1; f.y1 = y;
    return y;
}

/* ---- Compressor gain computation ---- */

double MultibandCompressor2::computeGain(double inputDb, const BandParams& p, GainEnvelope& env)
{
    /* Soft knee */
    double halfKnee = p.kneeWidth / 2.0;
    double threshold = p.threshold;
    double gainReduction = 0.0;

    if (inputDb < threshold - halfKnee) {
        gainReduction = 0.0;
    } else if (inputDb > threshold + halfKnee) {
        gainReduction = (threshold - inputDb) * (1.0 - 1.0 / p.ratio);
    } else {
        /* Quadratic interpolation within knee */
        double x = inputDb - threshold + halfKnee;
        double t = x / p.kneeWidth;
        gainReduction = t * t * (p.kneeWidth / 2.0) * (1.0 - 1.0 / p.ratio);
    }

    double targetGain = qPow(10.0, gainReduction / 20.0);

    /* Envelope follower */
    if (targetGain < env.gainLin)
        env.gainLin += (targetGain - env.gainLin) * env.attackCoeff;
    else
        env.gainLin += (targetGain - env.gainLin) * env.releaseCoeff;

    env.reductionDb = 20.0 * qLog10(qMax(1e-10, env.gainLin));
    return env.gainLin;
}

/* ---- Process ---- */

QVector<double> MultibandCompressor2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double x = input[i];

        /* 4-band split using 3 crossovers (cascaded for LR4) */
        double band[4];
        double lp0 = filterProcess(m_lp[0], x);
        double hp0 = filterProcess(m_hp[0], x);
        double lp1 = filterProcess(m_lp[1], hp0);
        double hp1 = filterProcess(m_hp[1], hp0);
        double lp2 = filterProcess(m_lp[2], hp1);
        double hp2 = filterProcess(m_hp[2], hp1);

        band[0] = filterProcess(m_lp[0], lp0);  /* Low (cascaded) */
        band[1] = filterProcess(m_lp[1], lp1);  /* Low-Mid */
        band[2] = lp2;                           /* Mid */
        band[3] = hp2;                           /* High */

        /* Process each band through compressor */
        double sum = 0.0;
        for (int b = 0; b < 4; ++b) {
            double absVal = qAbs(band[b]);
            double dbLevel = 20.0 * qLog10(qMax(1e-10, absVal));

            /* RMS tracking */
            m_env[b].rmsAccum += band[b] * band[b];
            m_env[b].rmsCount++;

            double gain = computeGain(dbLevel, m_bands[b], m_env[b]);
            double makeup = qPow(10.0, m_bands[b].makeupGain / 20.0);
            sum += band[b] * gain * makeup;
        }
        output[i] = sum;
    }

    m_stats.totalFrames += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum * 1000.0 / m_stats.totalFrames : 0.0;

    emit processingCompleted(n);
    return output;
}

/* ---- Accessors ---- */

QVector<double> MultibandCompressor2::bandGainReduction() const
{
    QVector<double> gr(4);
    for (int b = 0; b < 4; ++b)
        gr[b] = m_env[b].reductionDb;
    return gr;
}

QVector<double> MultibandCompressor2::bandLevels() const
{
    QVector<double> levels(4);
    for (int b = 0; b < 4; ++b) {
        if (m_env[b].rmsCount > 0) {
            double rms = qSqrt(m_env[b].rmsAccum / m_env[b].rmsCount);
            levels[b] = 20.0 * qLog10(qMax(1e-10, rms));
            m_env[b].rmsAccum = 0;
            m_env[b].rmsCount = 0;
        } else {
            levels[b] = -120.0;
        }
    }
    return levels;
}

void MultibandCompressor2::resetStatistics()
{
    m_stats = Stats{0, 0.0, m_sampleRate};
    m_timeSum = 0.0;
}
