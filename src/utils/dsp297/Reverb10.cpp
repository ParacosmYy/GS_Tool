/**
 * @file Reverb10.cpp
 * @brief Reverb10 实现
 *
 * 实现混响：后期反射FDN反馈延迟网络与早期反射随机光线追踪实现自然房间混响模拟。
 */

#include "utils/dsp297/Reverb10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Reverb10::Reverb10(QObject *parent)
    : QObject(parent)
{
    initFDN();
    initEarlyReflections();
}

Reverb10::~Reverb10() = default;

/* ---- Configuration ---- */

void Reverb10::setRoomParams(const RoomParams& params) { m_params = params; }
void Reverb10::setSampleRate(double sr) { m_sampleRate = qBound(8000.0, sr, 192000.0); }

/* ---- Initialize FDN delay lines (Schroeder all-pass series) ---- */

void Reverb10::initFDN()
{
    // Prime-based delay lengths in samples for 8-line FDN (44.1 kHz reference)
    static const int baseDelays[kFDNOrder] = {
        37, 59, 67, 73, 83, 97, 107, 131
    };

    double scale = m_sampleRate / 44100.0 * m_params.roomSize;
    m_delayLengths.resize(kFDNOrder);
    m_delayLines.resize(kFDNOrder);
    m_writePos.resize(kFDNOrder, 0);

    for (int i = 0; i < kFDNOrder; ++i) {
        m_delayLengths[i] = qMax(1, static_cast<int>(baseDelays[i] * scale));
        m_delayLines[i].resize(m_delayLengths[i], 0.0);
    }

    // Build Hadamard feedback matrix (orthogonal, lossless)
    for (int i = 0; i < kFDNOrder; ++i) {
        for (int j = 0; j < kFDNOrder; ++j) {
            // Hadamard entry: H[i][j] = (-1)^(popcount(i & j)) / sqrt(N)
            int xorVal = i & j;
            int popcount = 0;
            while (xorVal) { popcount += xorVal & 1; xorVal >>= 1; }
            m_feedbackMatrix[i][j] = ((popcount % 2 == 0) ? 1.0 : -1.0)
                                      / qSqrt(static_cast<double>(kFDNOrder));
        }
    }
}

/* ---- Initialize early reflection taps via stochastic ray tracing ---- */

void Reverb10::initEarlyReflections()
{
    // Simulate stochastic ray tracing: random delay times and attenuation
    int numTaps = 12;
    m_earlyTaps.resize(numTaps);
    m_earlyGains.resize(numTaps);

    double baseDelay = 0.001 * m_sampleRate * m_params.roomSize;
    for (int i = 0; i < numTaps; ++i) {
        // Exponential distribution for realistic ray arrival times
        double t = baseDelay * (1.0 + i * 0.7 + qrand() / static_cast<double>(RAND_MAX) * 0.3);
        m_earlyTaps[i] = static_cast<int>(t);
        // Inverse-square attenuation with random scattering
        double dist = 1.0 + i * 0.5;
        m_earlyGains[i] = m_params.earlyGain / (dist * dist) *
                           (0.8 + 0.4 * qrand() / static_cast<double>(RAND_MAX));
    }
}

/* ---- Flush all delay line state ---- */

void Reverb10::flush()
{
    for (int i = 0; i < kFDNOrder; ++i) {
        m_delayLines[i].fill(0.0);
        m_writePos[i] = 0;
    }
}

/* ---- Process early reflections ---- */

QVector<double> Reverb10::processEarly(const QVector<double>& input)
{
    int n = input.size();
    int maxTap = 0;
    for (int t : m_earlyTaps) maxTap = qMax(maxTap, t);

    QVector<double> output(n + maxTap, 0.0);

    for (int i = 0; i < n; ++i) {
        output[i] += input[i] * m_params.dryLevel;
        for (int t = 0; t < m_earlyTaps.size(); ++t) {
            int delay = m_earlyTaps[t];
            double gain = m_earlyGains[t];
            if (i + delay < output.size())
                output[i + delay] += input[i] * gain;
        }
    }
    // Trim to input length
    output.resize(n);
    return output;
}

/* ---- Process late reverb via FDN ---- */

QVector<double> Reverb10::processLate(const QVector<double>& input)
{
    int n = input.size();
    QVector<double> output(n, 0.0);

    double damping = m_params.damping;

    for (int i = 0; i < n; ++i) {
        // Read from delay lines at current write position
        double readVals[kFDNOrder];
        for (int j = 0; j < kFDNOrder; ++j)
            readVals[j] = m_delayLines[j][m_writePos[j]];

        // FDN output: sum of delayed signals
        double outSample = 0.0;
        for (int j = 0; j < kFDNOrder; ++j)
            outSample += readVals[j];
        outSample /= kFDNOrder;
        output[i] = outSample * m_params.lateGain;

        // Feedback: multiply by Hadamard matrix, add input
        double feedback[kFDNOrder] = {};
        for (int j = 0; j < kFDNOrder; ++j) {
            for (int k = 0; k < kFDNOrder; ++k)
                feedback[j] += m_feedbackMatrix[j][k] * readVals[k];
            // Apply damping (low-pass filter in feedback path)
            feedback[j] = feedback[j] * (1.0 - damping) + readVals[j] * damping;
            // Inject input signal
            feedback[j] += input[i] / kFDNOrder;
        }

        // Write back to delay lines
        for (int j = 0; j < kFDNOrder; ++j) {
            m_delayLines[j][m_writePos[j]] = feedback[j];
            m_writePos[j] = (m_writePos[j] + 1) % m_delayLengths[j];
        }
    }
    return output;
}

/* ---- Main process ---- */

Reverb10::ProcessResult Reverb10::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    int n = input.size();

    // Rebuild FDN if room size changed
    initFDN();
    initEarlyReflections();

    // Process early and late reflections
    auto early = processEarly(input);
    auto late = processLate(input);

    // Combine early + late
    result.output.resize(n);
    for (int i = 0; i < n; ++i) {
        result.output[i] = early[i] + late[i];
        result.earlyEnergy += early[i] * early[i];
        result.lateEnergy += late[i] * late[i];
    }
    result.earlyEnergy = qSqrt(result.earlyEnergy / qMax(1, n));
    result.lateEnergy = qSqrt(result.lateEnergy / qMax(1, n));

    double elapsed = timer.elapsed();
    result.totalTimeMs = elapsed;
    m_stats.lastFrameCount = n;
    m_stats.totalProcessings++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processDone(n, elapsed);
    return result;
}

/* ---- Reset ---- */

void Reverb10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
