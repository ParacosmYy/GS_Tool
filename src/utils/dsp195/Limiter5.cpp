/**
 * @file Limiter5.cpp
 * @brief Limiter5 实现
 *
 * 实现真峰值限制器：sinc插值ISP检测、前视延迟、增益平滑。
 */

#include "utils/dsp195/Limiter5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Limiter5::Limiter5(QObject *parent) : QObject(parent) {}
Limiter5::~Limiter5() = default;

/* ---- Configuration ---- */

void Limiter5::setCeiling(double dB) { m_ceilingDb = dB; }
void Limiter5::setThreshold(double dB) { m_thresholdDb = dB; }
void Limiter5::setRelease(double ms) { m_releaseMs = qMax(1.0, ms); }
void Limiter5::setLookahead(int samples) { m_lookahead = qMax(0, samples); }
void Limiter5::setOversampleRate(int rate) { m_oversampleRate = qMax(1, rate); }

/* ---- Sinc interpolation kernel ---- */

QVector<double> Limiter5::sincKernel(int taps, int oversample) const
{
    int halfLen = taps * oversample;
    QVector<double> kernel(halfLen * 2 + 1);
    double sum = 0.0;
    for (int i = 0; i <= halfLen * 2; ++i) {
        double x = (i - halfLen) / static_cast<double>(oversample);
        double s = (qAbs(x) < 1e-10) ? 1.0 : qSin(M_PI * x) / (M_PI * x);
        // Blackman window
        double w = 0.42 - 0.5 * qCos(2.0 * M_PI * i / (halfLen * 2))
                   + 0.08 * qCos(4.0 * M_PI * i / (halfLen * 2));
        kernel[i] = s * w;
        sum += kernel[i];
    }
    // Normalize
    for (auto& v : kernel) v /= qMax(sum, 1e-15);
    return kernel;
}

/* ---- Compute gain ---- */

double Limiter5::computeGain(double peak, double sampleRate)
{
    double ceilingLin = qPow(10.0, m_ceilingDb / 20.0);
    double threshLin = qPow(10.0, m_thresholdDb / 20.0);

    double targetGain = 1.0;
    if (peak > threshLin) {
        // Compute needed gain reduction
        targetGain = qMin(ceilingLin / qMax(peak, 1e-15), 1.0);
    }

    // Smooth gain: instant attack, exponential release
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * sampleRate / 1000.0));
    if (targetGain < m_gainSmooth) {
        m_gainSmooth = targetGain; // Instant attack
    } else {
        m_gainSmooth = releaseCoeff * m_gainSmooth + (1.0 - releaseCoeff) * targetGain;
    }
    return m_gainSmooth;
}

/* ---- Upsample ---- */

QVector<double> Limiter5::upsample(const QVector<double>& input) const
{
    int n = input.size();
    int os = m_oversampleRate;
    int outLen = n * os;

    // Zero-stuff
    QVector<double> stuffed(outLen, 0.0);
    for (int i = 0; i < n; ++i) stuffed[i * os] = input[i];

    // Apply anti-image LPF
    int taps = 16;
    auto kernel = sincKernel(taps, os);

    QVector<double> output(outLen, 0.0);
    int halfLen = kernel.size() / 2;
    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int k = 0; k < kernel.size(); ++k) {
            int idx = i - halfLen + k;
            if (idx >= 0 && idx < outLen) sum += stuffed[idx] * kernel[k];
        }
        output[i] = sum;
    }
    return output;
}

/* ---- Downsample ---- */

QVector<double> Limiter5::downsample(const QVector<double>& input) const
{
    int os = m_oversampleRate;
    int outLen = input.size() / os;
    QVector<double> output(outLen, 0.0);
    for (int i = 0; i < outLen; ++i)
        output[i] = input[i * os];
    return output;
}

/* ---- ISP detection ---- */

QVector<double> Limiter5::detectISP(const QVector<double>& input) const
{
    if (input.isEmpty()) return {};

    // Upsample for inter-sample peak detection
    QVector<double> up = upsample(input);

    // Find peaks in upsampled signal
    QVector<double> isps(input.size());
    for (int i = 0; i < input.size(); ++i) {
        double peak = qAbs(input[i]);
        // Check surrounding upsampled samples
        for (int j = 1; j < m_oversampleRate; ++j) {
            int idx = i * m_oversampleRate + j;
            if (idx < up.size())
                peak = qMax(peak, qAbs(up[idx]));
        }
        isps[i] = peak;
    }
    return isps;
}

/* ---- True-peak level ---- */

double Limiter5::truePeakLevel(const QVector<double>& input) const
{
    QVector<double> isps = detectISP(input);
    double maxISP = 0.0;
    for (double v : isps) maxISP = qMax(maxISP, v);
    return 20.0 * qLn(qMax(maxISP, 1e-15)) / qLn(10.0);
}

/* ---- Process ---- */

QVector<double> Limiter5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return input;

    double sampleRate = 44100.0; // default assumption

    // Initialize delay line if needed
    if (m_delayLine.size() != m_lookahead + 1) {
        m_delayLine.resize(m_lookahead + 1, 0.0);
        m_delayPos = 0;
    }

    // Detect inter-sample peaks
    QVector<double> isps = detectISP(input);

    QVector<double> output(n, 0.0);
    double peakIn = 0.0, peakOut = 0.0, maxISP = 0.0;
    double gainReductionSum = 0.0;

    for (int i = 0; i < n; ++i) {
        double peak = isps[i];
        peakIn = qMax(peakIn, qAbs(input[i]));
        maxISP = qMax(maxISP, peak);

        // Compute gain reduction
        double gain = computeGain(peak, sampleRate);
        gainReductionSum += (1.0 - gain);

        // Apply lookahead: store current in delay, read delayed
        m_delayLine[m_delayPos] = input[i] * gain;
        int readPos = (m_delayPos + 1) % (m_lookahead + 1);
        output[i] = m_delayLine[readPos];
        m_delayPos = (m_delayPos + 1) % (m_lookahead + 1);

        peakOut = qMax(peakOut, qAbs(output[i]));
    }

    // Update stats
    m_stats.totalSamples += n;
    m_stats.peakInput = qMax(m_stats.peakInput, peakIn);
    m_stats.peakOutput = qMax(m_stats.peakOutput, peakOut);
    m_stats.maxISP = qMax(m_stats.maxISP, maxISP);
    m_gainReductionCount += n;
    m_stats.avgGainReduction = gainReductionSum / n;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamples * 1000.0;

    emit processingCompleted(n, peakIn - peakOut, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Limiter5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_gainSmooth = 1.0;
    m_delayLine.clear();
    m_delayPos = 0;
    m_gainReductionCount = 0;
}
