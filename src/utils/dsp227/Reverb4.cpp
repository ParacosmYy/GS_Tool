/**
 * @file Reverb4.cpp
 * @brief Reverb4 实现
 *
 * 实现FDN8反馈延迟网络混响与Householder反射矩阵扩散。
 */

#include "utils/dsp227/Reverb4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstring>

/* ---- Static delay line lengths (prime-ish, in samples at 44.1kHz) ---- */

const int Reverb4::kDelayLengths[Reverb4::kChannels] = {
    1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116
};

/* ---- Construction / Destruction ---- */

Reverb4::Reverb4(QObject *parent) : QObject(parent)
{
    // Initialize delay lines
    m_delayLines.resize(kChannels);
    m_delayPos.resize(kChannels, 0);
    for (int i = 0; i < kChannels; ++i) {
        m_delayLines[i].resize(kDelayLengths[i], 0.0);
        m_feedback[i] = 0.84;
        m_dampState[i] = 0.0;
    }
}

Reverb4::~Reverb4() = default;

/* ---- Configuration ---- */

void Reverb4::setParameters(double roomSize, double damping,
                              double wetLevel, double dryLevel)
{
    m_roomSize = qBound(0.1, roomSize, 1.0);
    m_damping = qBound(0.0, damping, 1.0);
    m_wetLevel = qBound(0.0, wetLevel, 1.0);
    m_dryLevel = qBound(0.0, dryLevel, 1.0);

    // Adjust feedback based on room size
    for (int i = 0; i < kChannels; ++i) {
        double baseFb = 0.84;
        m_feedback[i] = baseFb * (0.5 + 0.5 * m_roomSize);
    }
}

/* ---- Apply Householder reflection matrix ---- */

void Reverb4::applyHouseholder(double samples[kChannels]) const
{
    // Householder reflection: H = I - (2/n) * u * u^T
    // For n=8: factor = 2/8 = 0.25
    double sum = 0.0;
    for (int i = 0; i < kChannels; ++i)
        sum += samples[i];

    double factor = 2.0 / kChannels;
    for (int i = 0; i < kChannels; ++i)
        samples[i] -= factor * sum;

    // Normalize to preserve energy
    double energy = 0.0;
    for (int i = 0; i < kChannels; ++i)
        energy += samples[i] * samples[i];
    if (energy > 1e-10) {
        double scale = 1.0 / qSqrt(energy) * qSqrt(kChannels) * 0.5;
        for (int i = 0; i < kChannels; ++i)
            samples[i] *= scale;
    }
}

/* ---- Read from delay line ---- */

double Reverb4::readDelay(int ch) const
{
    return m_delayLines[ch][m_delayPos[ch]];
}

/* ---- Write to delay line ---- */

void Reverb4::writeDelay(int ch, double value)
{
    m_delayLines[ch][m_delayPos[ch]] = value;
    m_delayPos[ch] = (m_delayPos[ch] + 1) % m_delayLines[ch].size();
}

/* ---- Process block ---- */

QVector<double> Reverb4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int s = 0; s < n; ++s) {
        double inSample = input[s];

        // Read current delay outputs
        double delayed[kChannels];
        for (int i = 0; i < kChannels; ++i)
            delayed[i] = readDelay(i);

        // Sum delayed signals for output
        double wetSample = 0.0;
        for (int i = 0; i < kChannels; ++i)
            wetSample += delayed[i];
        wetSample /= kChannels;

        // Apply damping (one-pole low-pass) and feedback
        double feedbackIn[kChannels];
        for (int i = 0; i < kChannels; ++i) {
            m_dampState[i] = m_damping * delayed[i] +
                             (1.0 - m_damping) * m_dampState[i];
            feedbackIn[i] = m_feedback[i] * m_dampState[i];
        }

        // Apply Householder diffusion matrix
        applyHouseholder(feedbackIn);

        // Write back to delay lines with input
        for (int i = 0; i < kChannels; ++i)
            writeDelay(i, feedbackIn[i] + inSample * 0.25);

        // Mix dry and wet
        output[s] = m_dryLevel * inSample + m_wetLevel * wetSample;
    }

    m_stats.numProcessed += n;
    m_stats.totalOps++;
    m_stats.blockSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset delay lines ---- */

void Reverb4::reset()
{
    for (int i = 0; i < kChannels; ++i) {
        m_delayLines[i].fill(0.0);
        m_delayPos[i] = 0;
        m_dampState[i] = 0.0;
    }
}

/* ---- Tail response ---- */

QVector<double> Reverb4::tailResponse(int length) const
{
    // Simulate tail by running silence through a copy of the reverb state
    QVector<double> tail(length, 0.0);

    // Snapshot current state
    QVector<QVector<double>> delayCopy = m_delayLines;
    QVector<int> posCopy = m_delayPos;
    double dampCopy[kChannels];
    std::memcpy(dampCopy, m_dampState, sizeof(m_dampState));

    for (int s = 0; s < length; ++s) {
        double wet = 0.0;
        for (int i = 0; i < kChannels; ++i) {
            double val = delayCopy[i][posCopy[i]];
            dampCopy[i] = m_damping * val +
                          (1.0 - m_damping) * dampCopy[i];
            wet += val;
        }
        tail[s] = wet / kChannels;

        // Advance delay positions with feedback (no new input)
        double sum = 0.0;
        for (int i = 0; i < kChannels; ++i)
            sum += m_feedback[i] * dampCopy[i];
        double factor = 2.0 / kChannels;
        for (int i = 0; i < kChannels; ++i) {
            double fb = m_feedback[i] * dampCopy[i] - factor * sum;
            delayCopy[i][posCopy[i]] = fb;
            posCopy[i] = (posCopy[i] + 1) % delayCopy[i].size();
        }
    }
    return tail;
}

/* ---- Reset statistics ---- */

void Reverb4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
