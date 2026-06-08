/**
 * @file Deesser5.cpp
 * @brief Deesser5 实现
 *
 * 实现去齿音：频谱质心检测、带通滤波、频率选择性动态扩展。
 */

#include "utils/dsp218/Deesser5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Deesser5::Deesser5(QObject *parent) : QObject(parent)
{
    setParameters();
}

Deesser5::~Deesser5() = default;

/* ---- Configuration ---- */

void Deesser5::setParameters(double thresholdDb, double freqLow,
                               double freqHigh, double reductionDb,
                               int frameSize)
{
    m_thresholdDb = thresholdDb;
    m_freqLow = freqLow;
    m_freqHigh = freqHigh;
    m_reductionDb = reductionDb;
    m_frameSize = qMax(64, frameSize);

    // Hann window
    m_window.resize(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        m_window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / m_frameSize));

    // Envelope follower coefficients
    double attackMs = 0.5;
    double releaseMs = 50.0;
    m_attackCoeff = qExp(-1.0 / (m_sampleRate * attackMs * 0.001));
    m_releaseCoeff = qExp(-1.0 / (m_sampleRate * releaseMs * 0.001));

    designBandpass();
}

/* ---- Design band-pass filter ---- */

void Deesser5::designBandpass()
{
    // Second-order IIR band-pass via bilinear transform
    double f1 = m_freqLow / m_sampleRate;
    double f2 = m_freqHigh / m_sampleRate;
    double w1 = qTan(M_PI * f1);
    double w2 = qTan(M_PI * f2);

    double bw = w2 - w1;
    double w0Sq = w1 * w2;

    double den = 1.0 + bw + w0Sq;

    m_bpB.resize(3);
    m_bpA.resize(3);

    m_bpB[0] = bw / den;
    m_bpB[1] = 0.0;
    m_bpB[2] = -bw / den;

    m_bpA[0] = 1.0;
    m_bpA[1] = 2.0 * (w0Sq - 1.0) / den;
    m_bpA[2] = (1.0 - bw + w0Sq) / den;
}

/* ---- Apply band-pass filter ---- */

QVector<double> Deesser5::applyBandpass(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double y = m_bpB[0] * input[i];
        if (i >= 1) y += m_bpB[1] * input[i - 1] - m_bpA[1] * output[i - 1];
        if (i >= 2) y += m_bpB[2] * input[i - 2] - m_bpA[2] * output[i - 2];
        output[i] = y;
    }
    return output;
}

/* ---- Compute envelope ---- */

double Deesser5::computeEnvelope(const QVector<double>& signal)
{
    for (int i = 0; i < signal.size(); ++i) {
        double abs = qAbs(signal[i]);
        if (abs > m_envelope)
            m_envelope = m_attackCoeff * m_envelope + (1.0 - m_attackCoeff) * abs;
        else
            m_envelope = m_releaseCoeff * m_envelope + (1.0 - m_releaseCoeff) * abs;
    }
    return m_envelope;
}

/* ---- Spectral centroid ---- */

double Deesser5::spectralCentroid(const QVector<double>& frame) const
{
    int n = qMin(frame.size(), m_frameSize);

    // Windowed DFT (magnitude only for centroid)
    double sumMag = 0.0;
    double sumWeighted = 0.0;
    int halfN = n / 2;

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double w = (i < m_window.size()) ? m_window[i] : 1.0;
            double angle = -2.0 * M_PI * k * i / n;
            re += frame[i] * w * qCos(angle);
            im += frame[i] * w * qSin(angle);
        }
        double mag = qSqrt(re * re + im * im);
        double freq = k * m_sampleRate / n;
        sumMag += mag;
        sumWeighted += freq * mag;
    }

    return sumMag > 1e-10 ? sumWeighted / sumMag : 0.0;
}

/* ---- Detect sibilance ---- */

double Deesser5::detectSibilance(const QVector<double>& frame) const
{
    double centroid = spectralCentroid(frame);
    // Sibilance probability based on centroid being in sibilant frequency range
    if (centroid < m_freqLow) return 0.0;
    if (centroid > m_freqHigh) return 0.0;

    // Peak in sibilant range
    double rangeCenter = (m_freqLow + m_freqHigh) * 0.5;
    double rangeHalf = (m_freqHigh - m_freqLow) * 0.5;
    double dist = qAbs(centroid - rangeCenter) / rangeHalf;
    return qMax(0.0, 1.0 - dist);
}

/* ---- Process ---- */

QVector<double> Deesser5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());

    // Band-pass filter to isolate sibilant frequencies
    QVector<double> bpSignal = applyBandpass(input);

    // Compute envelope of sibilant band
    double env = computeEnvelope(bpSignal);
    double envDb = 20.0 * qLn(qMax(env, 1e-10)) / qLn(10.0);

    // Compute sibilance probability
    double sibLevel = detectSibilance(input);

    // Compute gain reduction
    double reduction = 0.0;
    if (envDb > m_thresholdDb && sibLevel > 0.3) {
        double overDb = envDb - m_thresholdDb;
        reduction = qMin(overDb * sibLevel, -m_reductionDb);
    }

    // Apply frequency-selective gain
    double gainDb = -reduction;
    double gainLin = qPow(10.0, gainDb / 20.0);

    for (int i = 0; i < input.size(); ++i) {
        // Attenuate only the sibilant band component
        output[i] = input[i] - bpSignal[i] * (1.0 - gainLin);
    }

    m_stats.totalFrames++;
    m_reductionSum += reduction;
    m_stats.avgReductionDb = m_reductionSum / m_stats.totalFrames;
    m_stats.frameSize = m_frameSize;
    m_stats.sibilanceRate = sibLevel;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(sibLevel, reduction, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Deesser5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reductionSum = 0.0;
    m_envelope = 0.0;
}
