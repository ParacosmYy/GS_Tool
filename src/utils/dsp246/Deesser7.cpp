/**
 * @file Deesser7.cpp
 * @brief Deesser7 实现
 *
 * 实现去齿音处理器：嘶嘶音频段峰值检测与侧链滤波动态处理。
 */

#include "utils/dsp246/Deesser7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Deesser7::Deesser7(QObject *parent) : QObject(parent)
{
    m_bpX.resize(3, 0.0);
    m_bpY.resize(3, 0.0);
    designBandpass();
}

Deesser7::~Deesser7() = default;

/* ---- Configuration ---- */

void Deesser7::setParams(const Params& params)
{
    m_params = params;
    designBandpass();
}

void Deesser7::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    designBandpass();
}

/* ---- Design band-pass filter (2nd order Butterworth) ---- */

void Deesser7::designBandpass()
{
    double f0 = m_params.frequency;
    double bw = m_params.bandwidth;
    double fs = m_sampleRate;

    double fL = qMax(20.0, f0 - bw / 2.0);
    double fH = qMin(fs / 2.0 - 1.0, f0 + bw / 2.0);
    double wL = 2.0 * M_PI * fL / fs;
    double wH = 2.0 * M_PI * fH / fs;

    // Simplified 2nd-order bandpass via bilinear transform
    double w0 = 2.0 * M_PI * f0 / fs;
    double Q = f0 / bw;
    double alpha = qSin(w0) / (2.0 * Q);

    m_bpB[0] = alpha;
    m_bpB[1] = 0.0;
    m_bpB[2] = -alpha;

    double a0 = 1.0 + alpha;
    m_bpA[0] = 1.0;
    m_bpA[1] = -2.0 * qCos(w0);
    m_bpA[2] = 1.0 - alpha;

    // Normalize
    for (int i = 0; i < 3; ++i) {
        m_bpB[i] /= a0;
        m_bpA[i] /= a0;
    }

    // Envelope follower coefficients
    double attackMs = qMax(0.01, m_params.attack);
    double releaseMs = qMax(0.1, m_params.release);
    m_attackCoeff = 1.0 - qExp(-1.0 / (attackMs * 0.001 * fs));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (releaseMs * 0.001 * fs));
}

/* ---- Band-pass filter single sample ---- */

double Deesser7::bandpassSample(double x)
{
    // Shift delay lines
    m_bpX[2] = m_bpX[1]; m_bpX[1] = m_bpX[0]; m_bpX[0] = x;
    m_bpY[2] = m_bpY[1]; m_bpY[1] = m_bpY[0];

    double y = m_bpB[0] * m_bpX[0] + m_bpB[1] * m_bpX[1] + m_bpB[2] * m_bpX[2]
             - m_bpA[1] * m_bpY[1] - m_bpA[2] * m_bpY[2];
    m_bpY[0] = y;
    return y;
}

/* ---- Compute gain reduction ---- */

double Deesser7::computeGainReduction(double levelDb) const
{
    if (levelDb < m_params.threshold) return 0.0;

    // Compression: reduce by (level - threshold) * (1 - 1/ratio)
    double overDb = levelDb - m_params.threshold;
    double reduction = overDb * (1.0 - 1.0 / m_params.ratio);
    return -reduction;  // Negative = attenuation
}

/* ---- Process ---- */

QVector<double> Deesser7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double totalReduction = 0.0;
    int detections = 0;

    for (int i = 0; i < n; ++i) {
        // Sidechain: band-pass filter to isolate sibilance
        double sidechain = bandpassSample(input[i]);

        // Envelope follower on sidechain signal
        double absSide = qAbs(sidechain);
        if (absSide > m_env)
            m_env += m_attackCoeff * (absSide - m_env);
        else
            m_env += m_releaseCoeff * (absSide - m_env);

        // Convert to dB
        double levelDb = 20.0 * qLn(qMax(m_env, 1e-10)) / qLn(10.0);

        // Compute gain reduction
        double gainReductionDb = computeGainReduction(levelDb);
        double gain = qPow(10.0, (gainReductionDb + m_params.makeupGain) / 20.0);

        if (gainReductionDb < -0.1) detections++;
        totalReduction += qAbs(gainReductionDb);

        output[i] = input[i] * gain;
    }

    m_stats.numFrames++;
    m_stats.sibilanceDetections += detections;
    m_reductionSum += totalReduction;
    m_stats.avgReduction = (m_stats.numFrames > 0)
        ? m_reductionSum / (m_stats.numFrames * qMax(1, n)) : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset state ---- */

void Deesser7::reset()
{
    m_bpX.fill(0.0);
    m_bpY.fill(0.0);
    m_env = 0.0;
}

/* ---- Reset statistics ---- */

void Deesser7::resetStatistics()
{
    reset();
    m_stats = Stats{}; m_timeSum = 0.0; m_reductionSum = 0.0;
}
