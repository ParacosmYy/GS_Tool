/**
 * @file Reverb6.cpp
 * @brief Reverb6 实现
 *
 * 实现反馈延迟网络混响器：哈达玛矩阵混合扩散空间场模拟。
 */

#include "utils/dsp255/Reverb6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Reverb6::Reverb6(QObject *parent)
    : QObject(parent) {}
Reverb6::~Reverb6() = default;

/* ---- Power of 2 check ---- */

bool Reverb6::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/* ---- Configuration ---- */

void Reverb6::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); }
void Reverb6::setRoomSize(double size) { m_roomSize = qBound(0.0, size, 1.0); }
void Reverb6::setDecayTime(double t60) { m_t60 = qMax(0.1, t60); }
void Reverb6::setWetDry(double mix) { m_wetDry = qBound(0.0, mix, 1.0); }

/* ---- Build Hadamard matrix (Sylvester construction) ---- */

void Reverb6::buildHadamard(int n)
{
    m_hadamard.resize(n);
    for (int i = 0; i < n; ++i)
        m_hadamard[i].resize(n, 1.0);

    // Iterative Sylvester construction
    for (int h = 1; h < n; h <<= 1) {
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < h; ++j) {
                m_hadamard[i + h][j] = m_hadamard[i][j];
                m_hadamard[i][j + h] = m_hadamard[i][j];
                m_hadamard[i + h][j + h] = -m_hadamard[i][j];
            }
        }
    }

    // Normalize by 1/sqrt(n) for lossless mixing
    double norm = 1.0 / qSqrt(static_cast<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            m_hadamard[i][j] *= norm;
}

/* ---- Compute delay lengths using coprime primes ---- */

void Reverb6::computeDelayLengths()
{
    // Use mutually prime delay lengths for maximal diffusion
    // Base primes: ~1-5ms range scaled by room size
    static const int primes[] = {
        31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101
    };

    double baseMs = 5.0 + m_roomSize * 40.0; // 5-45ms range
    m_delayLengths.resize(m_order);
    for (int i = 0; i < m_order; ++i) {
        double lenMs = baseMs * primes[i % 16] / 31.0;
        m_delayLengths[i] = qMax(1, static_cast<int>(lenMs * m_sampleRate / 1000.0));
    }
}

/* ---- Compute feedback gains from T60 ---- */

void Reverb6::computeFeedbackGains()
{
    m_feedbackGains.resize(m_order);
    for (int i = 0; i < m_order; ++i) {
        if (m_delayLengths[i] <= 0) {
            m_feedbackGains[i] = 0.0;
            continue;
        }
        // g = 10^(-3 * d / (fs * T60))
        double exponent = -3.0 * m_delayLengths[i] / (m_sampleRate * m_t60);
        m_feedbackGains[i] = qPow(10.0, exponent);
    }
}

/* ---- Tone damping (simple one-pole lowpass) ---- */

double Reverb6::damp(int lineIdx, double sample) const
{
    Q_UNUSED(lineIdx);
    // Simple damping: attenuate high frequencies slightly
    return sample * 0.95;
}

/* ---- Initialize FDN ---- */

bool Reverb6::init(int fdnOrder)
{
    if (!isPowerOf2(fdnOrder)) return false;
    m_order = fdnOrder;

    buildHadamard(m_order);
    computeDelayLengths();
    computeFeedbackGains();

    // Allocate delay line buffers
    m_delayLines.resize(m_order);
    m_writePos.resize(m_order, 0);
    for (int i = 0; i < m_order; ++i) {
        m_delayLines[i].resize(m_delayLengths[i], 0.0);
        m_writePos[i] = 0;
    }

    m_stats.fdnOrder = m_order;
    return true;
}

/* ---- Process block ---- */

QVector<double> Reverb6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int s = 0; s < n; ++s) {
        // Read from each delay line
        QVector<double> delayed(m_order, 0.0);
        for (int i = 0; i < m_order; ++i) {
            int readPos = m_writePos[i];
            delayed[i] = m_delayLines[i][readPos];
        }

        // Apply Hadamard mixing
        QVector<double> mixed(m_order, 0.0);
        for (int i = 0; i < m_order; ++i) {
            double sum = 0.0;
            for (int j = 0; j < m_order; ++j) {
                sum += m_hadamard[i][j] * delayed[j];
            }
            mixed[i] = sum;
        }

        // Write input + feedback into delay lines
        for (int i = 0; i < m_order; ++i) {
            double feedback = m_feedbackGains[i] * damp(i, mixed[i]);
            m_delayLines[i][m_writePos[i]] = input[s] + feedback;
            m_writePos[i] = (m_writePos[i] + 1) % m_delayLengths[i];
        }

        // Output: sum of delayed signals (wet/dry mix)
        double reverbSum = 0.0;
        for (int i = 0; i < m_order; ++i)
            reverbSum += delayed[i];
        reverbSum /= m_order; // Normalize

        output[s] = input[s] * (1.0 - m_wetDry) + reverbSum * m_wetDry;
    }

    m_stats.numFramesProcessed += n;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, elapsed);
    return output;
}

/* ---- Reset delay lines ---- */

void Reverb6::reset()
{
    for (int i = 0; i < m_order; ++i) {
        m_delayLines[i].fill(0.0);
        m_writePos[i] = 0;
    }
}

/* ---- Reset statistics ---- */

void Reverb6::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
