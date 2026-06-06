/**
 * @file Compressor2.cpp
 * @brief Compressor2 实现
 *
 * 实现动态范围压缩器：软/硬拐点、侧链输入、并行(NY)压缩、RMS/Peak检测。
 */

#include "utils/dsp176/Compressor2.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Compressor2::Compressor2(QObject *parent)
    : QObject(parent)
{
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_params.attack * 0.001 * m_sampleRate));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_params.release * 0.001 * m_sampleRate));
}

Compressor2::~Compressor2() = default;

/* ---- Configuration ---- */

void Compressor2::setParameters(const Parameters& params)
{
    m_params = params;
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_params.attack * 0.001 * m_sampleRate));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_params.release * 0.001 * m_sampleRate));
}

void Compressor2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_params.attack * 0.001 * m_sampleRate));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_params.release * 0.001 * m_sampleRate));
}

/* ---- dB conversion ---- */

double Compressor2::toDb(double linear)
{
    return 20.0 * qLn(qMax(1e-10, qAbs(linear))) / M_LN2;
}

double Compressor2::fromDb(double db)
{
    return qExp(db * M_LN2 / 20.0);
}

/* ---- Gain reduction computation ---- */

double Compressor2::computeGainReduction(double inputDb) const
{
    double threshold = m_params.threshold;
    double ratio = m_params.ratio;
    double kneeW = m_params.kneeWidth;

    if (m_params.knee == HardKnee) {
        /* Hard knee: abrupt transition */
        if (inputDb <= threshold)
            return 0.0;
        return (inputDb - threshold) * (1.0 - 1.0 / ratio);
    }

    /* Soft knee: quadratic interpolation around threshold */
    double halfKnee = kneeW / 2.0;
    if (inputDb < threshold - halfKnee) {
        return 0.0;
    } else if (inputDb > threshold + halfKnee) {
        return (inputDb - threshold) * (1.0 - 1.0 / ratio);
    } else {
        /* Quadratic region */
        double x = inputDb - threshold + halfKnee;
        return x * x / (2.0 * kneeW) * (1.0 - 1.0 / ratio);
    }
}

/* ---- Process (single channel) ---- */

QVector<double> Compressor2::process(const QVector<double>& input)
{
    return processSidechain(input, input);
}

/* ---- Process with sidechain ---- */

QVector<double> Compressor2::processSidechain(const QVector<double>& input,
                                               const QVector<double>& sidechain)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(input.size(), sidechain.size());
    QVector<double> output(n);
    m_gainEnv.resize(n);

    double peakGR = 0.0;
    double sumGR = 0.0;

    /* RMS window for RMS detection mode */
    double rmsSum = 0.0;
    int rmsWindowSize = qMax(1, static_cast<int>(0.01 * m_sampleRate));

    for (int i = 0; i < n; ++i) {
        /* Level detection */
        double level;
        if (m_params.detection == RMS) {
            rmsSum += sidechain[i] * sidechain[i];
            if (i >= rmsWindowSize)
                rmsSum -= sidechain[i - rmsWindowSize] * sidechain[i - rmsWindowSize];
            level = qSqrt(rmsSum / qMin(i + 1, rmsWindowSize));
        } else {
            level = qAbs(sidechain[i]);
        }

        double levelDb = toDb(level);

        /* Compute target gain reduction */
        double targetGR = computeGainReduction(levelDb);

        /* Envelope follower: smooth attack/release */
        if (targetGR > m_envelope) {
            /* Attack: compressor is reducing more */
            m_envelope += m_attackCoeff * (targetGR - m_envelope);
        } else {
            /* Release: gain recovering */
            m_envelope += m_releaseCoeff * (targetGR - m_envelope);
        }

        m_gainEnv[i] = m_envelope;

        /* Apply gain reduction + makeup gain */
        double gain = fromDb(-m_envelope + m_params.makeupGain);

        /* Parallel (New York) compression: mix dry + wet */
        double wet = input[i] * gain;
        double dry = input[i] * fromDb(m_params.dryMix);
        output[i] = wet + dry * (m_params.dryMix > 0.0 ? 1.0 : 0.0);

        peakGR = qMax(peakGR, m_envelope);
        sumGR += m_envelope;
    }

    /* Update stats */
    m_stats.totalProcessed += n;
    m_stats.peakGainReduction = peakGR;
    m_stats.avgGainReduction = (n > 0) ? sumGR / n : 0.0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalProcessed / 1000);

    emit processingCompleted(n, peakGR);
    return output;
}

/* ---- Gain envelope ---- */

QVector<double> Compressor2::gainEnvelope() const
{
    return m_gainEnv;
}

/* ---- Transfer curve ---- */

QVector<QVector<double>> Compressor2::transferCurve(int points) const
{
    QVector<QVector<double>> curve(2);
    curve[0].resize(points);
    curve[1].resize(points);

    for (int i = 0; i < points; ++i) {
        double inputDb = -60.0 + 120.0 * i / (points - 1);
        curve[0][i] = inputDb;
        curve[1][i] = inputDb - computeGainReduction(inputDb) + m_params.makeupGain;
    }
    return curve;
}

/* ---- Reset ---- */

void Compressor2::reset()
{
    m_envelope = 0.0;
    m_gainEnv.clear();
}

void Compressor2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
