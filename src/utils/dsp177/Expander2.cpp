/**
 * @file Expander2.cpp
 * @brief Expander2 实现
 *
 * 实现动态扩展器：上下行压缩、比例依赖攻击/释放、自动增益。
 */

#include "utils/dsp177/Expander2.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Expander2::Expander2(QObject *parent)
    : QObject(parent)
{
}

Expander2::~Expander2() = default;

/* ---- Configuration ---- */

void Expander2::setThreshold(double dB) { m_threshold = dB; }
void Expander2::setRatio(double ratio) { m_ratio = qMax(1.0, ratio); }
void Expander2::setAttack(double ms) { m_attack = qMax(0.01, ms); }
void Expander2::setRelease(double ms) { m_release = qMax(0.01, ms); }
void Expander2::setKnee(double dB) { m_knee = qMax(0.0, dB); }
void Expander2::setMode(Mode mode) { m_mode = mode; }
void Expander2::setDetection(Detection det) { m_detection = det; }
void Expander2::setAutoGain(bool enabled) { m_autoGain = enabled; }

/* ---- dB conversion ---- */

double Expander2::toDb(double linear) const
{
    if (linear <= 0.0) return -120.0;
    return 20.0 * qLn(linear) / qLn(10.0);
}

/* ---- Gain computation ---- */

double Expander2::computeGain(double inputLeveldB) const
{
    double halfKnee = m_knee / 2.0;
    double tLow = m_threshold - halfKnee;
    double tHigh = m_threshold + halfKnee;

    double outputDb = inputLeveldB;

    if (m_mode == Downward) {
        /* Below threshold: expand (make quieter) */
        if (inputLeveldB < tLow) {
            outputDb = m_threshold + (inputLeveldB - m_threshold) * m_ratio;
        } else if (inputLeveldB < tHigh && m_knee > 0.0) {
            /* Soft knee interpolation */
            double x = (inputLeveldB - tLow) / m_knee;
            double kneeGain = (inputLeveldB - m_threshold) * m_ratio;
            double linGain = inputLeveldB - m_threshold;
            outputDb = m_threshold + kneeGain * (1.0 - x) + linGain * x;
        }
    } else {
        /* Upward: above threshold boost, below pass through */
        if (inputLeveldB > tHigh) {
            outputDb = m_threshold + (inputLeveldB - m_threshold) / m_ratio;
        } else if (inputLeveldB > tLow && m_knee > 0.0) {
            double x = (tHigh - inputLeveldB) / m_knee;
            double kneeGain = (inputLeveldB - m_threshold) / m_ratio;
            double linGain = inputLeveldB - m_threshold;
            outputDb = m_threshold + kneeGain * (1.0 - x) + linGain * x;
        }
    }

    return outputDb - inputLeveldB;
}

/* ---- Gain curve ---- */

QVector<QPair<double, double>> Expander2::gainCurve(double minDb, double maxDb,
                                                      int points) const
{
    QVector<QPair<double, double>> curve;
    double step = (maxDb - minDb) / (points - 1);
    for (int i = 0; i < points; ++i) {
        double inDb = minDb + i * step;
        curve.append({inDb, computeGain(inDb)});
    }
    return curve;
}

/* ---- Process audio frame ---- */

QVector<double> Expander2::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    /* Ratio-dependent attack/release: higher ratio = faster attack */
    double effectiveAttack = m_attack / m_ratio;
    double effectiveRelease = m_release / m_ratio;

    double coeffAttack = qExp(-1.0 / (effectiveAttack * m_sampleRate / 1000.0));
    double coeffRelease = qExp(-1.0 / (effectiveRelease * m_sampleRate / 1000.0));

    QVector<double> output(n);
    double maxGainReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        /* Envelope follower */
        double absVal = qAbs(input[i]);
        if (m_detection == RMS) {
            absVal = qSqrt(input[i] * input[i]);
        }

        double coeff = (absVal > m_envelope) ? coeffAttack : coeffRelease;
        m_envelope = coeff * m_envelope + (1.0 - coeff) * absVal;

        /* Compute gain */
        double envDb = toDb(m_envelope);
        double gainDb = computeGain(envDb);

        /* Track max reduction */
        if (gainDb < maxGainReduction)
            maxGainReduction = gainDb;

        /* Auto-gain: compensate for expansion energy */
        double gainLin = toLinear(gainDb);
        if (m_autoGain) {
            double compensation = toLinear(-gainDb * 0.5);
            gainLin *= qMin(compensation, 2.0);
        }

        output[i] = input[i] * gainLin;
    }

    m_stats.totalFrames++;
    m_stats.blockSize = n;
    m_stats.gainReduction = maxGainReduction;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(n, maxGainReduction);
    return output;
}

void Expander2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 0.0;
    m_currentGain = 1.0;
}
