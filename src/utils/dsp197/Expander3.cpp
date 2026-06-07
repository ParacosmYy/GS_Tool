/**
 * @file Expander3.cpp
 * @brief Expander3 实现
 *
 * 实现下行扩展器：程序依赖attack/release、立体声链路、一致性成像。
 */

#include "utils/dsp197/Expander3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander3::Expander3(QObject *parent) : QObject(parent) { updateCoefficients(); }
Expander3::~Expander3() = default;

/* ---- Configuration ---- */

void Expander3::setThreshold(double dB) { m_threshold = dB; }
void Expander3::setRatio(double r) { m_ratio = qMax(1.0, r); }
void Expander3::setAttack(double ms) { m_attack = qMax(0.01, ms); updateCoefficients(); }
void Expander3::setRelease(double ms) { m_release = qMax(0.01, ms); updateCoefficients(); }
void Expander3::setRange(double dB) { m_range = dB; }
void Expander3::setSampleRate(int sr) { m_sampleRate = qMax(8000, sr); updateCoefficients(); }
void Expander3::setStereoLinkEnabled(bool e) { m_stereoLink = e; }
void Expander3::setKneeWidth(double dB) { m_kneeWidth = qMax(0.0, dB); }

/* ---- Helpers ---- */

double Expander3::sampleToDB(double sample) { return linearToDB(qMax(1e-10, qAbs(sample))); }
double Expander3::dbToLinear(double dB) { return qPow(10.0, dB / 20.0); }
double Expander3::linearToDB(double linear) { return 20.0 * qLog10(qMax(1e-10, linear)); }

void Expander3::updateCoefficients()
{
    double attackSec = m_attack / 1000.0;
    double releaseSec = m_release / 1000.0;
    m_envAttackCoeff = qExp(-1.0 / (m_sampleRate * attackSec));
    m_envReleaseCoeff = qExp(-1.0 / (m_sampleRate * releaseSec));
}

/* ---- Program-dependent timing ---- */

double Expander3::programAttack(double inputdB) const
{
    // Faster attack for louder transients
    double factor = qBound(0.3, 1.0 + inputdB / 60.0, 3.0);
    return m_attack / factor;
}

double Expander3::programRelease(double inputdB) const
{
    // Slower release for signals near threshold
    double distFromThreshold = qAbs(inputdB - m_threshold);
    double factor = qBound(0.5, 1.0 + distFromThreshold / 30.0, 4.0);
    return m_release * factor;
}

/* ---- Compute gain ---- */

double Expander3::computeGain(double inputLeveldB) const
{
    if (inputLeveldB >= m_threshold)
        return 0.0;  // no expansion above threshold

    double overdB = inputLeveldB - m_threshold;
    double gaindB = overdB * (1.0 - 1.0 / m_ratio);

    // Soft knee
    if (m_kneeWidth > 0.0) {
        double halfKnee = m_kneeWidth / 2.0;
        if (overdB > -halfKnee && overdB < halfKnee) {
            double x = (overdB + halfKnee) / m_kneeWidth;
            gaindB = gaindB * x * x;
        }
    }

    // Clamp range
    gaindB = qMax(gaindB, m_range);
    return gaindB;
}

/* ---- Process mono ---- */

QVector<double> Expander3::processMono(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double peakReduction = 0.0;
    double totalReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double inDB = sampleToDB(input[i]);

        // Program-dependent envelope
        double progAttack = programAttack(inDB);
        double progRelease = programRelease(inDB);
        double attackCoeff = qExp(-1.0 / (m_sampleRate * progAttack / 1000.0));
        double releaseCoeff = qExp(-1.0 / (m_sampleRate * progRelease / 1000.0));

        if (inDB > m_envelope)
            m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * inDB;
        else
            m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * inDB;

        double gaindB = computeGain(m_envelope);
        double gainLinear = dbToLinear(gaindB);
        output[i] = input[i] * gainLinear;

        double reduction = qAbs(gaindB);
        peakReduction = qMax(peakReduction, reduction);
        totalReduction += reduction;
    }

    m_stats.totalFrames += n;
    m_stats.peakGainReduction = qMax(m_stats.peakGainReduction, peakReduction);
    m_stats.avgGainReduction = totalReduction / qMax(1, n);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalFrames / 256);

    emit processingCompleted(n, peakReduction, timer.elapsed());
    return output;
}

/* ---- Process stereo ---- */

QVector<double> Expander3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n < 2) return processMono(input);

    QVector<double> output(n);
    double peakReduction = 0.0;
    double totalReduction = 0.0;

    for (int i = 0; i < n - 1; i += 2) {
        double left = input[i];
        double right = input[i + 1];

        // Stereo link: use max of both channels for detection
        double detectLevel;
        if (m_stereoLink) {
            double lDB = sampleToDB(left);
            double rDB = sampleToDB(right);
            detectLevel = qMax(lDB, rDB);
        } else {
            detectLevel = sampleToDB(left);
        }

        // Envelope follower
        if (detectLevel > m_envelope)
            m_envelope = m_envAttackCoeff * m_envelope + (1.0 - m_envAttackCoeff) * detectLevel;
        else
            m_envelope = m_envReleaseCoeff * m_envelope + (1.0 - m_envReleaseCoeff) * detectLevel;

        double gaindB = computeGain(m_envelope);
        double gainLinear = dbToLinear(gaindB);

        // Apply identical gain for coherent stereo imaging
        output[i] = left * gainLinear;
        output[i + 1] = right * gainLinear;

        double reduction = qAbs(gaindB);
        peakReduction = qMax(peakReduction, reduction);
        totalReduction += reduction;
    }

    // Handle odd sample
    if (n % 2 == 1)
        output[n - 1] = input[n - 1] * dbToLinear(computeGain(sampleToDB(input[n - 1])));

    m_stats.totalFrames += n;
    m_stats.peakGainReduction = qMax(m_stats.peakGainReduction, peakReduction);
    m_stats.avgGainReduction = totalReduction / qMax(1, n / 2);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalFrames / 256);

    emit processingCompleted(n, peakReduction, timer.elapsed());
    return output;
}

/* ---- Envelope level ---- */

double Expander3::envelopeLevel() const { return m_envelope; }

/* ---- Reset ---- */

void Expander3::resetStatistics()
{
    m_stats = Stats{};
    m_stats.sampleRate = m_sampleRate;
    m_timeSum = 0.0;
    m_envelope = -120.0;
}
