/**
 * @file Reverb3.cpp
 * @brief Reverb3 实现
 *
 * 实现算法混响：Schroeder全通级联、Hadamard混合FDN、阻尼低通。
 */

#include "utils/dsp213/Reverb3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Reverb3::Reverb3(QObject *parent) : QObject(parent)
{
    initDelayLines();
}

Reverb3::~Reverb3() = default;

/* ---- Initialize delay lines ---- */

void Reverb3::initDelayLines()
{
    // Prime-based delay lengths for diffuse reverb
    // Schroeder allpass delays (samples at 44100 Hz)
    int apDelays[kAllpassCount] = {225, 556, 441, 341};
    // FDN delay lengths
    int fdnDelays[kFDNLines] = {3771, 4587, 5297, 6121};

    double srRatio = m_sampleRate / 44100.0;

    m_apBuf.resize(kAllpassCount);
    m_apPos.resize(kAllpassCount, 0);
    m_apGain.resize(kAllpassCount, 0.5);
    for (int i = 0; i < kAllpassCount; ++i) {
        int len = qMax(1, static_cast<int>(apDelays[i] * srRatio));
        m_apBuf[i].resize(len, 0.0);
        m_apGain[i] = 0.5 + 0.1 * i;  // Slightly increasing
    }

    m_fdnBuf.resize(kFDNLines);
    m_fdnPos.resize(kFDNLines, 0);
    m_fdnLen.resize(kFDNLines);
    m_lpState.resize(kFDNLines, 0.0);
    for (int i = 0; i < kFDNLines; ++i) {
        int len = qMax(1, static_cast<int>(fdnDelays[i] * m_roomSize * srRatio));
        m_fdnLen[i] = len;
        m_fdnBuf[i].resize(len, 0.0);
    }

    m_feedback = 0.84 * m_roomSize;
}

/* ---- 4x4 Hadamard mixing ---- */

void Reverb3::hadamardMix(double v[4])
{
    double a = v[0] + v[1];
    double b = v[0] - v[1];
    double c = v[2] + v[3];
    double d = v[2] - v[3];
    v[0] = (a + c) * 0.5;
    v[1] = (b + d) * 0.5;
    v[2] = (a - c) * 0.5;
    v[3] = (b - d) * 0.5;
}

/* ---- Process allpass cascade ---- */

double Reverb3::processAllpass(double input)
{
    double output = input;
    for (int i = 0; i < kAllpassCount; ++i) {
        int len = m_apBuf[i].size();
        double delayed = m_apBuf[i][m_apPos[i]];
        double g = m_apGain[i];
        double bufIn = output + g * delayed;
        m_apBuf[i][m_apPos[i]] = bufIn;
        output = delayed - g * bufIn;
        m_apPos[i] = (m_apPos[i] + 1) % len;
    }
    return output;
}

/* ---- Process FDN ---- */

double Reverb3::processFDN(double input)
{
    double out[kFDNLines];
    double feedback[kFDNLines];

    // Read from delay lines and apply damping low-pass
    for (int i = 0; i < kFDNLines; ++i) {
        out[i] = m_fdnBuf[i][m_fdnPos[i]];
        // One-pole low-pass for damping
        m_lpState[i] = out[i] * (1.0 - m_damping) + m_lpState[i] * m_damping;
        feedback[i] = m_lpState[i];
    }

    // Hadamard mixing
    double mix[kFDNLines];
    for (int i = 0; i < kFDNLines; ++i) mix[i] = feedback[i];
    hadamardMix(mix);

    // Write back with feedback + input
    for (int i = 0; i < kFDNLines; ++i) {
        int len = m_fdnBuf[i].size();
        m_fdnBuf[i][m_fdnPos[i]] = mix[i] * m_feedback + input;
        m_fdnPos[i] = (m_fdnPos[i] + 1) % len;
    }

    // Sum FDN outputs
    double sum = 0.0;
    for (int i = 0; i < kFDNLines; ++i)
        sum += out[i];
    return sum / kFDNLines;
}

/* ---- Set parameters ---- */

void Reverb3::setParameters(double roomSize, double damping, double wetLevel,
                             double sampleRate)
{
    m_roomSize = qBound(0.1, roomSize, 1.0);
    m_damping = qBound(0.0, damping, 0.99);
    m_wet = qBound(0.0, wetLevel, 1.0);
    m_dry = 1.0 - m_wet;
    m_sampleRate = qMax(8000.0, sampleRate);
    initDelayLines();
}

/* ---- Process single sample ---- */

double Reverb3::processOne(double input)
{
    double apOut = processAllpass(input);
    double fdnOut = processFDN(apOut);
    return input * m_dry + fdnOut * m_wet;
}

/* ---- Process buffer ---- */

QVector<double> Reverb3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i)
        output[i] = processOne(input[i]);

    m_stats.totalProcessed += n;
    m_stats.delayLines = kFDNLines;
    m_stats.roomSize = m_roomSize;
    m_stats.sampleRate = m_sampleRate;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessed > 0)
        ? m_timeSum * 1000.0 / m_stats.totalProcessed : 0.0;
    emit processingCompleted(n, timer.elapsed());

    return output;
}

/* ---- Reset delay lines ---- */

void Reverb3::reset()
{
    for (int i = 0; i < kAllpassCount; ++i) {
        m_apBuf[i].fill(0.0);
        m_apPos[i] = 0;
    }
    for (int i = 0; i < kFDNLines; ++i) {
        m_fdnBuf[i].fill(0.0);
        m_fdnPos[i] = 0;
        m_lpState[i] = 0.0;
    }
}

/* ---- Reset statistics ---- */

void Reverb3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
