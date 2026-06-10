/**
 * @file SignalGenerator9.cpp
 * @brief SignalGenerator9 实现
 *
 * 实现信号发生器：FM合成与算子堆叠的多载波波形生成及调制指数控制。
 */

#include "utils/signal285/SignalGenerator9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

SignalGenerator9::SignalGenerator9(QObject *parent)
    : QObject(parent) {}

SignalGenerator9::~SignalGenerator9() = default;

/* ---- Configuration ---- */

void SignalGenerator9::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); }

void SignalGenerator9::setOperators(const QVector<FMOperator>& ops)
{
    m_operators = ops;
    m_phase.resize(ops.size(), 0.0);
}

/* ---- Wrap phase to [0, 2π) ---- */

double SignalGenerator9::wrapPhase(double phase)
{
    constexpr double TWO_PI = 2.0 * M_PI;
    phase = fmod(phase, TWO_PI);
    if (phase < 0.0) phase += TWO_PI;
    return phase;
}

/* ---- Evaluate base waveform ---- */

double SignalGenerator9::evalWave(WaveType type, double phase) const
{
    phase = wrapPhase(phase);
    switch (type) {
    case Sine:
        return qSin(phase);
    case Square:
        return (phase < M_PI) ? 1.0 : -1.0;
    case Sawtooth:
        return 1.0 - 2.0 * phase / (2.0 * M_PI);
    case Triangle:
        return (phase < M_PI)
            ? (-1.0 + 2.0 * phase / M_PI)
            : (3.0 - 2.0 * phase / M_PI);
    case Noise:
        return 2.0 * (static_cast<double>(qrand()) / RAND_MAX) - 1.0;
    }
    return 0.0;
}

/* ---- Evaluate operator stack (recursive FM) ---- */

double SignalGenerator9::evalOperatorStack()
{
    int n = m_operators.size();
    if (n == 0) return 0.0;

    // Evaluate operators in reverse (modulators first)
    // Each operator's output modulates its carrier's phase
    QVector<double> outputs(n, 0.0);

    // Bottom-up: operators with no modulator first
    for (int i = n - 1; i >= 0; --i) {
        const auto& op = m_operators[i];
        double modSignal = 0.0;

        // If this operator has a modulator, get its output
        if (op.modulatorIdx >= 0 && op.modulatorIdx < n) {
            modSignal = outputs[op.modulatorIdx] * op.modulationIndex;
        }

        // FM synthesis: carrier phase = base_phase + modulator * modIndex
        double modPhase = m_phase[i] + modSignal;
        outputs[i] = op.amplitude * evalWave(op.waveform, modPhase);
    }

    // Sum all carrier outputs
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        if (m_operators[i].modulatorIdx < 0) {
            // Top-level operator (no one modulates through this)
            sum += outputs[i];
        }
    }

    // If no top-level operators found, sum all
    if (sum == 0.0) {
        for (double o : outputs) sum += o;
    }
    return sum;
}

/* ---- Main generation ---- */

SignalGenerator9::GenResult SignalGenerator9::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    GenResult result;
    result.numSamples = numSamples;

    int nOps = m_operators.size();
    if (nOps == 0 || numSamples <= 0) return result;

    result.samples.resize(numSamples);
    m_phase.resize(nOps, 0.0);

    double peak = 0.0;
    double sumSq = 0.0;

    for (int s = 0; s < numSamples; ++s) {
        // Evaluate the operator stack for this sample
        double sample = evalOperatorStack();
        result.samples[s] = sample;

        double absVal = qAbs(sample);
        if (absVal > peak) peak = absVal;
        sumSq += sample * sample;

        // Advance phase for each operator
        for (int i = 0; i < nOps; ++i) {
            double phaseInc = 2.0 * M_PI * m_operators[i].frequency / m_sampleRate;
            m_phase[i] = wrapPhase(m_phase[i] + phaseInc);
        }
    }

    result.peakAmplitude = peak;
    result.rmsLevel = (numSamples > 0) ? qSqrt(sumSq / numSamples) : 0.0;

    double elapsed = timer.elapsed();
    m_stats.numSamples = numSamples;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit generationDone(numSamples, peak, elapsed);

    return result;
}

/* ---- Generate a single cycle ---- */

QVector<double> SignalGenerator9::generateCycle(WaveType type, double freq, int samplesPerCycle)
{
    QVector<double> cycle(samplesPerCycle);
    for (int i = 0; i < samplesPerCycle; ++i) {
        double phase = 2.0 * M_PI * i / samplesPerCycle;
        cycle[i] = evalWave(type, phase);
    }
    return cycle;
}

/* ---- Reset ---- */

void SignalGenerator9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_phase.fill(0.0);
}
