/**
 * @file DynamicEQ.cpp
 * @brief DynamicEQ 实现
 *
 * 实现动态均衡器：多频段滤波器组、RMS/Peak检测、侧链、压缩/扩展。
 */

#include "utils/dsp169/DynamicEQ.h"

#include <QElapsedTimer>
#include <QtMath>

DynamicEQ::DynamicEQ(double sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(sampleRate)
{
}

DynamicEQ::~DynamicEQ() = default;

void DynamicEQ::setSampleRate(double sr) { m_sampleRate = qMax(1.0, sr); }
void DynamicEQ::setDetectMode(DetectMode mode) { m_detect = mode; }

void DynamicEQ::designPeakFilter(Biquad& bq, double freq, double gainDb, double q) const
{
    double w0 = 2.0 * M_PI * freq / m_sampleRate;
    double A = qPow(10.0, gainDb / 40.0);
    double cosW0 = qCos(w0);
    double sinW0 = qSin(w0);
    double alpha = sinW0 / (2.0 * q);

    double a0 = 1.0 + alpha / A;
    bq.b0 = (1.0 + alpha * A) / a0;
    bq.b1 = (-2.0 * cosW0) / a0;
    bq.b2 = (1.0 - alpha * A) / a0;
    bq.a1 = (2.0 * cosW0) / a0;
    bq.a2 = (1.0 - alpha / A) / a0;
}

double DynamicEQ::computeGain(double inputDb, const BandParams& bp) const
{
    if (inputDb <= bp.threshold) return 0.0;

    double overDb = inputDb - bp.threshold;
    double gain;
    if (bp.mode == Compress) {
        /* Compression: reduce gain above threshold */
        gain = -overDb * (1.0 - 1.0 / bp.ratio);
    } else {
        /* Expansion: increase gain below threshold (used inversely) */
        gain = overDb * (bp.ratio - 1.0);
    }
    /* Clamp to range */
    return qBound(-bp.range, gain, bp.range);
}

double DynamicEQ::updateEnvelope(double current, double target,
                                  double coeffAttack, double coeffRelease) const
{
    if (target > current)
        return current + coeffAttack * (target - current);
    return current + coeffRelease * (target - current);
}

int DynamicEQ::addBand(const BandParams& params)
{
    BandState state;
    state.params = params;
    designPeakFilter(state.filter, params.freq, 0.0, params.q);
    m_bands.append(state);
    m_stats.bandCount = m_bands.size();
    return m_bands.size() - 1;
}

void DynamicEQ::setBand(int index, const BandParams& params)
{
    if (index < 0 || index >= m_bands.size()) return;
    m_bands[index].params = params;
    designPeakFilter(m_bands[index].filter, params.freq, 0.0, params.q);
}

void DynamicEQ::removeBand(int index)
{
    if (index >= 0 && index < m_bands.size())
        m_bands.removeAt(index);
    m_stats.bandCount = m_bands.size();
}

QVector<double> DynamicEQ::process(const QVector<double>& input)
{
    return processWithSidechain(input, input);
}

QVector<double> DynamicEQ::processWithSidechain(const QVector<double>& input,
                                                  const QVector<double>& sidechain)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output = input;

    for (int b = 0; b < m_bands.size(); ++b) {
        BandState& band = m_bands[b];
        double sr = m_sampleRate;
        double coeffAttack = 1.0 - qExp(-1.0 / (band.params.attack * 0.001 * sr));
        double coeffRelease = 1.0 - qExp(-1.0 / (band.params.release * 0.001 * sr));

        for (int i = 0; i < n; ++i) {
            /* Detection */
            double detectSample = (i < sidechain.size()) ? sidechain[i] : 0.0;
            double levelDb;
            if (m_detect == RMS) {
                double rms = detectSample * detectSample;
                levelDb = 10.0 * qLog10(qMax(rms, 1e-30));
            } else {
                levelDb = 20.0 * qLog10(qMax(qAbs(detectSample), 1e-30));
            }

            /* Envelope follower */
            band.envLevel = updateEnvelope(band.envLevel, levelDb,
                                           coeffAttack, coeffRelease);

            /* Compute dynamic gain */
            double dynGain = computeGain(band.envLevel, band.params);

            /* Smooth gain transition */
            band.dynGain = updateEnvelope(band.dynGain, dynGain,
                                          coeffAttack, coeffRelease);

            /* Apply: design filter with base+dynamic gain and filter sample */
            double totalGain = band.params.gain + band.dynGain;
            designPeakFilter(band.filter, band.params.freq, totalGain, band.params.q);
            output[i] = band.filter.process(output[i]);
        }
    }

    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamples > 0)
        ? m_timeSum / m_stats.totalSamples : 0.0;

    emit processingCompleted(n);
    return output;
}

void DynamicEQ::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
