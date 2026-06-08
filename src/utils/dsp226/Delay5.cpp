/**
 * @file Delay5.cpp
 * @brief Delay5 实现
 *
 * 实现磁带延迟仿真：抖晃/颤动调制全通滤波、磁饱和非线性、磁带损耗低通。
 */

#include "utils/dsp226/Delay5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Delay5::Delay5(QObject *parent) : QObject(parent) {}
Delay5::~Delay5() = default;

/* ---- Initialize ---- */

bool Delay5::init(int sampleRate, double maxDelayMs)
{
    if (sampleRate < 8000 || maxDelayMs < 1.0) return false;
    m_sampleRate = sampleRate;
    m_maxDelayMs = maxDelayMs;
    m_bufferSize = static_cast<int>(qCeil(maxDelayMs * sampleRate / 1000.0)) + 2;
    m_delayBuffer.resize(m_bufferSize, 0.0);
    m_writePos = 0;
    m_wowPhase = 0.0;
    m_flutterPhase = 0.0;
    m_allpassX1 = 0.0;
    m_allpassY1 = 0.0;
    m_lossZ1 = 0.0;
    m_stats.sampleRate = sampleRate;
    return true;
}

/* ---- Set parameters ---- */

void Delay5::setParameters(const Parameters& params)
{
    m_params = params;
    m_params.feedback = qBound(0.0, params.feedback, 0.99);
    m_params.mix = qBound(0.0, params.mix, 1.0);
    m_params.saturation = qBound(0.0, params.saturation, 1.0);
}

/* ---- Modulated delay time ---- */

double Delay5::modulatedDelay() const
{
    double baseDelay = m_params.delayTimeMs * m_sampleRate / 1000.0;
    double wowMod = m_params.wowDepth * m_sampleRate * qSin(m_wowPhase);
    double flutterMod = m_params.flutterDepth * m_sampleRate * qSin(m_flutterPhase);
    double total = baseDelay + wowMod + flutterMod;
    return qBound(1.0, total, static_cast<double>(m_bufferSize - 1));
}

/* ---- Magnetic saturation ---- */

double Delay5::saturate(double x) const
{
    double drive = 1.0 + m_params.saturation * 4.0;
    double driven = x * drive;
    // Soft-clip via tanh approximation: 3rd-order Padé
    double abs_d = qAbs(driven);
    if (abs_d < 0.333) return driven / drive;
    double sign = (driven >= 0) ? 1.0 : -1.0;
    return sign * (1.0 - 1.0 / (1.0 + abs_d * abs_d * 0.5)) / drive * drive;
}

/* ---- Tape loss filter (1st-order IIR lowpass) ---- */

double Delay5::tapeLossFilter(double input)
{
    double omega = 2.0 * M_PI * m_params.tapeLossHz / m_sampleRate;
    double alpha = omega / (omega + 1.0);
    m_lossZ1 = alpha * input + (1.0 - alpha) * m_lossZ1;
    return m_lossZ1;
}

/* ---- Modulated allpass for wow/flutter coloration ---- */

double Delay5::allpassModulate(double input)
{
    // 1st-order allpass with time-varying coefficient
    double modDepth = m_params.wowDepth * 0.5 + m_params.flutterDepth * 0.3;
    double coeff = 0.5 + modDepth * qSin(m_flutterPhase * 3.0);
    coeff = qBound(0.0, coeff, 0.95);
    double output = coeff * (input - m_allpassY1) + m_allpassX1;
    m_allpassX1 = input;
    m_allpassY1 = output;
    return output;
}

/* ---- Fractional delay read (linear interpolation) ---- */

double Delay5::readDelay(double delaySamples) const
{
    double readPos = m_writePos - delaySamples;
    while (readPos < 0) readPos += m_bufferSize;
    while (readPos >= m_bufferSize) readPos -= m_bufferSize;
    int idx0 = static_cast<int>(readPos);
    double frac = readPos - idx0;
    int idx1 = (idx0 + 1) % m_bufferSize;
    return m_delayBuffer[idx0] * (1.0 - frac) + m_delayBuffer[idx1] * frac;
}

/* ---- Process single sample ---- */

double Delay5::processSample(double input)
{
    // Read from modulated delay position
    double delaySamps = modulatedDelay();
    double delayed = readDelay(delaySamps);

    // Apply tape loss lowpass
    delayed = tapeLossFilter(delayed);

    // Apply allpass modulation for wow/flutter coloration
    delayed = allpassModulate(delayed);

    // Apply magnetic saturation
    delayed = saturate(delayed);

    // Write input + feedback into delay buffer
    double feedbackSample = delayed * m_params.feedback;
    m_delayBuffer[m_writePos] = input + feedbackSample;
    m_writePos = (m_writePos + 1) % m_bufferSize;

    // Advance LFO phases
    m_wowPhase += 2.0 * M_PI * m_params.wowRate / m_sampleRate;
    m_flutterPhase += 2.0 * M_PI * m_params.flutterRate / m_sampleRate;

    // Output: dry/wet mix
    return input * (1.0 - m_params.mix) + delayed * m_params.mix;
}

/* ---- Process buffer ---- */

void Delay5::processBuffer(QVector<double>& buffer)
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < buffer.size(); ++i)
        buffer[i] = processSample(buffer[i]);

    m_stats.numProcessed += buffer.size();
    m_stats.totalOps++;
    m_stats.bufferSize = buffer.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(buffer.size(), timer.elapsed());
}

/* ---- Reset ---- */

void Delay5::reset()
{
    std::fill(m_delayBuffer.begin(), m_delayBuffer.end(), 0.0);
    m_writePos = 0;
    m_wowPhase = 0.0;
    m_flutterPhase = 0.0;
    m_allpassX1 = 0.0;
    m_allpassY1 = 0.0;
    m_lossZ1 = 0.0;
}

/* ---- Reset statistics ---- */

void Delay5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}
