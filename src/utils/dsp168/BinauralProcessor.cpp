/**
 * @file BinauralProcessor.cpp
 * @brief BinauralProcessor 实现
 *
 * 实现双耳空间音频处理：简化HRTF建模、ITD/ILD计算、FIR卷积、串扰消除。
 */

#include "utils/dsp168/BinauralProcessor.h"

#include <QElapsedTimer>
#include <QtMath>

BinauralProcessor::BinauralProcessor(QObject *parent)
    : QObject(parent)
{
}

BinauralProcessor::~BinauralProcessor() = default;

void BinauralProcessor::setSampleRate(int rate) { m_sampleRate = qMax(8000, rate); }
void BinauralProcessor::setHrtfLength(int length) { m_hrtfLen = qMax(8, length); }
void BinauralProcessor::setCrossTalkStrength(double strength) { m_xtStrength = qBound(0.0, strength, 1.0); }

int BinauralProcessor::computeITD(const SourcePosition& pos) const
{
    /* Woodworth formula: ITD ≈ (a/c)(θ + sinθ) where a=head radius, c=speed */
    double headRadius = 0.0875; /* meters */
    double speedOfSound = 343.0; /* m/s */
    double theta = qDegreesToRadians(pos.azimuth);
    double itd = (headRadius / speedOfSound) * (qAbs(theta) + qSin(qAbs(theta)));
    return qRound(itd * m_sampleRate);
}

double BinauralProcessor::computeILD(const SourcePosition& pos) const
{
    /* Simple ILD model: higher frequency → more shadowing */
    double azAbs = qAbs(pos.azimuth);
    /* Max attenuation ~12dB at 90 degrees */
    double attenDB = 12.0 * qSin(qDegreesToRadians(azAbs));
    return qPow(10.0, -attenDB / 20.0);
}

QVector<double> BinauralProcessor::generateHrtf(double azimuth, bool isIpsi) const
{
    QVector<double> hrtf(m_hrtfLen, 0.0);
    double theta = qDegreesToRadians(azimuth);

    /* Simplified HRTF: head-related pinna model */
    double gain = isIpsi ? 1.0 : computeILD(SourcePosition{azimuth, 0.0, 1.0});
    double directDelay = isIpsi ? 0.0 : static_cast<double>(computeITD(
        SourcePosition{azimuth, 0.0, 1.0})) / m_sampleRate;

    int delaySamples = qRound(directDelay * m_sampleRate);
    delaySamples = qMin(delaySamples, m_hrtfLen - 1);

    /* Direct path: sinc-like impulse */
    hrtf[qMax(0, delaySamples)] = gain * 0.8;

    /* Early reflections (pinna) */
    for (int i = 1; i < m_hrtfLen; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;
        double decay = qExp(-t * 8000.0);
        hrtf[i] += gain * decay * 0.1 * qSin(2.0 * M_PI * 3000.0 * t + theta);
    }

    return hrtf;
}

QVector<double> BinauralProcessor::convolve(const QVector<double>& signal,
                                             const QVector<double>& kernel)
{
    int n = signal.size();
    int k = kernel.size();
    QVector<double> result(n + k - 1, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            result[i + j] += signal[i] * kernel[j];
    result.resize(n);
    return result;
}

void BinauralProcessor::applyCrossTalk(QVector<double>& left, QVector<double>& right) const
{
    /* Simple crosstalk cancellation: L' = L - α*R, R' = R - α*L */
    int n = qMin(left.size(), right.size());
    double alpha = m_xtStrength * 0.3;
    QVector<double> origL = left;
    for (int i = 0; i < n; ++i) {
        left[i] = origL[i] - alpha * right[i];
        right[i] = right[i] - alpha * origL[i];
    }
}

QPair<QVector<double>, QVector<double>> BinauralProcessor::process(
    const QVector<double>& mono, const SourcePosition& pos)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> leftH = leftHrtf(pos);
    QVector<double> rightH = rightHrtf(pos);

    QVector<double> leftOut = convolve(mono, leftH);
    QVector<double> rightOut = convolve(mono, rightH);

    applyCrossTalk(leftOut, rightOut);

    m_stats.totalProcessed += mono.size();
    m_stats.lastFrameSize = mono.size();
    m_stats.lastSampleRate = m_sampleRate;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessed > 0)
        ? m_timeSum * 1000.0 / m_stats.totalProcessed : 0.0;

    emit processingCompleted(mono.size());
    return {leftOut, rightOut};
}

QVector<double> BinauralProcessor::leftHrtf(const SourcePosition& pos) const
{
    /* Ipsilateral ear (same side as source) */
    bool isIpsi = (pos.azimuth >= 0);
    return generateHrtf(pos.azimuth, isIpsi);
}

QVector<double> BinauralProcessor::rightHrtf(const SourcePosition& pos) const
{
    /* Contralateral ear */
    bool isIpsi = (pos.azimuth < 0);
    return generateHrtf(pos.azimuth, isIpsi);
}

void BinauralProcessor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
