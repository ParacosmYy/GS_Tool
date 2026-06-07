/**
 * @file Reverb2.cpp
 * @brief Reverb2 实现
 *
 * 实现Schroeder混响：4梳状+2全通、预延迟、早期反射、阻尼。
 */

#include "utils/dsp190/Reverb2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Reverb2::Reverb2(QObject *parent) : QObject(parent)
{
    initFilters();
    generateEarlyReflections();
}

Reverb2::~Reverb2() = default;

/* ---- Configuration ---- */

void Reverb2::setSampleRate(double sr)
{
    m_sampleRate = qMax(8000.0, sr);
    initFilters();
    generateEarlyReflections();
}

void Reverb2::setPreDelay(double delayMs)
{
    m_preDelayMs = qMax(0.0, delayMs);
    int samples = static_cast<int>(m_preDelayMs * m_sampleRate / 1000.0);
    m_preBuf.resize(qMax(1, samples));
    m_preBuf.fill(0.0);
    m_prePos = 0;
}

void Reverb2::setRoomSize(double size) { m_roomSize = qBound(0.0, size, 1.0); initFilters(); }
void Reverb2::setDamping(double d) { m_damping = qBound(0.0, d, 1.0); initFilters(); }
void Reverb2::setWetDryMix(double wet) { m_wetMix = qBound(0.0, wet, 1.0); }
void Reverb2::setEarlyReflectionGain(double g) { m_earlyGain = qBound(0.0, g, 1.0); }

/* ---- Initialize Schroeder filter bank ---- */

void Reverb2::initFilters()
{
    m_combs.resize(4);
    m_allpasses.resize(2);

    // Schroeder comb delay lengths (in samples at 44100 Hz)
    static const int combDelays44[4] = {1557, 1617, 1491, 1422};
    // Allpass delay lengths
    static const int apDelays44[2] = {225, 556};

    double ratio = m_sampleRate / 44100.0;

    for (int i = 0; i < 4; ++i) {
        int len = qMax(1, static_cast<int>(combDelays44[i] * ratio));
        m_combs[i].buffer.resize(len, 0.0);
        m_combs[i].pos = 0;
        m_combs[i].feedback = 0.28 + m_roomSize * 0.56;
        m_combs[i].damp1 = m_damping;
        m_combs[i].damp2 = 1.0 - m_damping;
        m_combs[i].filterStore = 0.0;
    }

    for (int i = 0; i < 2; ++i) {
        int len = qMax(1, static_cast<int>(apDelays44[i] * ratio));
        m_allpasses[i].buffer.resize(len, 0.0);
        m_allpasses[i].pos = 0;
        m_allpasses[i].feedback = 0.5;
    }

    // Pre-delay buffer
    int preSamples = qMax(1, static_cast<int>(m_preDelayMs * m_sampleRate / 1000.0));
    m_preBuf.resize(preSamples, 0.0);
    m_prePos = 0;

    m_stats.combCount = 4;
    m_stats.allpassCount = 2;
}

/* ---- Generate early reflection pattern ---- */

void Reverb2::generateEarlyReflections()
{
    m_earlyTaps.clear();
    // Simulate early reflections with decreasing amplitude
    int base[] = {143, 271, 397, 523, 673, 811, 941, 1097};
    double gains[] = {0.84, 0.67, 0.52, 0.41, 0.32, 0.24, 0.18, 0.13};
    double ratio = m_sampleRate / 44100.0;

    for (int i = 0; i < 8; ++i) {
        int delay = qMax(1, static_cast<int>(base[i] * ratio));
        m_earlyTaps.append({delay, gains[i] * m_earlyGain});
    }
}

/* ---- Process comb filter with LP damping ---- */

double Reverb2::processComb(CombFilter& c, double input)
{
    double output = c.buffer[c.pos];
    // Lowpass damping
    c.filterStore = output * c.damp2 + c.filterStore * c.damp1;
    c.buffer[c.pos] = input + c.filterStore * c.feedback;
    c.pos = (c.pos + 1) % c.buffer.size();
    return output;
}

/* ---- Process allpass filter ---- */

double Reverb2::processAllpass(AllpassFilter& ap, double input)
{
    double buffered = ap.buffer[ap.pos];
    double output = -input + buffered;
    ap.buffer[ap.pos] = input + buffered * ap.feedback;
    ap.pos = (ap.pos + 1) % ap.buffer.size();
    return output;
}

/* ---- Process single sample ---- */

double Reverb2::processSample(double input)
{
    // Pre-delay
    double delayed = m_preBuf[m_prePos];
    m_preBuf[m_prePos] = input;
    m_prePos = (m_prePos + 1) % m_preBuf.size();

    // Early reflections (simple FIR)
    double early = 0.0;
    for (const auto& tap : m_earlyTaps)
        early += delayed * tap.second;

    // Schroeder reverb: 4 parallel comb filters
    double combSum = 0.0;
    for (auto& comb : m_combs)
        combSum += processComb(comb, delayed);

    // 2 cascaded allpass filters
    double revOut = combSum;
    for (auto& ap : m_allpasses)
        revOut = processAllpass(ap, revOut);

    // Mix dry + early reflections + late reverb
    double output = input * (1.0 - m_wetMix) + (early + revOut) * m_wetMix;

    m_stats.totalSamples++;
    return output;
}

/* ---- Process block ---- */

QVector<double> Reverb2::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processSample(input[i]);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamples > 0)
        ? m_timeSum / m_stats.totalSamples * 1000.0 : 0.0;

    emit processingCompleted(input.size(), timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Reverb2::reset()
{
    for (auto& c : m_combs) {
        c.buffer.fill(0.0);
        c.pos = 0;
        c.filterStore = 0.0;
    }
    for (auto& ap : m_allpasses) {
        ap.buffer.fill(0.0);
        ap.pos = 0;
    }
    m_preBuf.fill(0.0);
    m_prePos = 0;
}

/* ---- Reset statistics ---- */

void Reverb2::resetStatistics()
{
    m_stats = Stats{};
    m_stats.combCount = 4;
    m_stats.allpassCount = 2;
    m_timeSum = 0.0;
    reset();
}
