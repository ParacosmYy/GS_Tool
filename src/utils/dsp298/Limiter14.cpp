/**
 * @file Limiter14.cpp
 * @brief Limiter14 实现
 *
 * 实现真峰值限制器：过采样样本间峰值检测与前瞻增益平滑实现广播合规响度归一化。
 */

#include "utils/dsp298/Limiter14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Limiter14::Limiter14(QObject *parent)
    : QObject(parent) {}

Limiter14::~Limiter14() = default;

/* ---- Configuration ---- */

void Limiter14::setConfig(const LimiterConfig& config)
{
    m_config = config;
    m_config.thresholdDb = qBound(-60.0, config.thresholdDb, 0.0);
    m_config.attackMs = qBound(0.1, config.attackMs, 100.0);
    m_config.releaseMs = qBound(1.0, config.releaseMs, 5000.0);
    m_config.lookaheadMs = qBound(0.0, config.lookaheadMs, 50.0);
    m_config.oversampleFactor = qBound(1, config.oversampleFactor, 8);
    m_config.sampleRate = qBound(8000.0, config.sampleRate, 192000.0);

    m_thresholdLin = qPow(10.0, m_config.thresholdDb / 20.0);

    // Allocate lookahead delay line
    int delaySamples = static_cast<int>(m_config.lookaheadMs * m_config.sampleRate / 1000.0);
    m_delayLine.fill(0.0, qMax(1, delaySamples));
    m_delayWritePos = 0;
    m_envelope = 1.0;
}

/* ---- Upsample via windowed sinc interpolation ---- */

QVector<double> Limiter14::upsample(const QVector<double>& input, int factor) const
{
    if (factor <= 1) return input;
    int n = input.size();
    int outN = n * factor;
    QVector<double> out(outN, 0.0);

    // Windowed sinc lowpass kernel (cutoff at pi/factor)
    int halfLen = 4 * factor; // filter half-length
    for (int i = 0; i < n; ++i) {
        out[i * factor] = input[i] * factor; // zero-stuff and scale
    }

    // Apply lowpass via convolution
    QVector<double> filtered(outN, 0.0);
    for (int i = 0; i < outN; ++i) {
        double sum = 0.0;
        double wSum = 0.0;
        for (int k = -halfLen; k <= halfLen; ++k) {
            int idx = i + k;
            if (idx < 0 || idx >= outN) continue;
            double sincVal = (k == 0) ? 1.0 : qSin(M_PI * k / static_cast<double>(factor))
                / (M_PI * k / static_cast<double>(factor));
            // Hann window
            double win = 0.5 * (1.0 - qCos(2.0 * M_PI * (k + halfLen) / (2.0 * halfLen + 1)));
            sum += out[idx] * sincVal * win;
            wSum += sincVal * win;
        }
        filtered[i] = (wSum > 1e-10) ? sum : 0.0;
    }
    return filtered;
}

/* ---- Downsample via decimation ---- */

QVector<double> Limiter14::downsample(const QVector<double>& upsampled, int factor) const
{
    if (factor <= 1) return upsampled;
    int n = upsampled.size() / factor;
    QVector<double> out;
    out.reserve(n);
    for (int i = 0; i < n; ++i) {
        out.append(upsampled[i * factor] / factor);
    }
    return out;
}

/* ---- Detect true peak from oversampled buffer ---- */

double Limiter14::detectTruePeak(const QVector<double>& upsampled) const
{
    double peak = 0.0;
    for (double s : upsampled)
        peak = qMax(peak, qAbs(s));
    return peak;
}

/* ---- Compute gain reduction ---- */

double Limiter14::computeGainReduction(double peakLin) const
{
    if (peakLin <= m_thresholdLin) return 1.0;
    return m_thresholdLin / peakLin;
}

/* ---- Smooth gain envelope ---- */

void Limiter14::smoothEnvelope(double targetGain, int numSamples)
{
    double attackCoeff = qExp(-1.0 / (m_config.attackMs * m_config.sampleRate / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_config.releaseMs * m_config.sampleRate / 1000.0));

    // Process each sample to smooth the gain envelope
    for (int i = 0; i < numSamples; ++i) {
        double coeff = (targetGain < m_envelope) ? attackCoeff : releaseCoeff;
        m_envelope = targetGain + coeff * (m_envelope - targetGain);
    }
}

/* ---- Main process ---- */

Limiter14::LimiterResult Limiter14::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    LimiterResult result;
    int n = input.size();
    if (n == 0) return result;

    int factor = m_config.oversampleFactor;
    int delayLen = m_delayLine.size();

    QVector<double> output;
    output.reserve(n);

    double peakIn = 0.0;
    double peakOut = 0.0;
    double maxGainReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        double sample = input[i];
        peakIn = qMax(peakIn, qAbs(sample));

        // Write to lookahead delay line
        m_delayLine[m_delayWritePos] = sample;
        int readPos = (m_delayWritePos + 1) % delayLen;
        double delayed = m_delayLine[readPos];
        m_delayWritePos = (m_delayWritePos + 1) % delayLen;

        // Upsample a small window around current sample for true peak detection
        int winSize = qMin(8, qMin(i + 1, n - i));
        QVector<double> window(winSize);
        for (int w = 0; w < winSize; ++w) {
            int srcIdx = i - winSize / 2 + w;
            if (srcIdx >= 0 && srcIdx < n)
                window[w] = input[srcIdx];
        }

        auto upsampled = upsample(window, factor);
        double truePeak = detectTruePeak(upsampled);
        double targetGain = computeGainReduction(truePeak);

        smoothEnvelope(targetGain, 1);

        double outSample = delayed * m_envelope;
        output.append(outSample);

        peakOut = qMax(peakOut, qAbs(outSample));
        double gr = -20.0 * qLog10(qMax(1e-10, m_envelope));
        maxGainReduction = qMax(maxGainReduction, gr);
    }

    result.output = output;
    result.numSamples = n;
    result.peakInputDb = 20.0 * qLog10(qMax(1e-10, peakIn));
    result.peakOutputDb = 20.0 * qLog10(qMax(1e-10, peakOut));
    result.gainReductionDb = maxGainReduction;

    double elapsed = timer.elapsed();
    m_stats.totalProcesses++;
    m_gainReductionSum += maxGainReduction;
    m_stats.avgGainReduction = m_gainReductionSum / m_stats.totalProcesses;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcesses;

    emit processDone(n, maxGainReduction, elapsed);
    return result;
}

/* ---- Reset state ---- */

void Limiter14::reset()
{
    m_delayLine.fill(0.0);
    m_delayWritePos = 0;
    m_envelope = 1.0;
    m_oversampleBuffer.clear();
}

/* ---- Reset statistics ---- */

void Limiter14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_gainReductionSum = 0.0;
}
