/**
 * @file Goertzel10.cpp
 * @brief Goertzel10 实现
 *
 * 实现Goertzel算法：滑动窗口重叠相加连续实时单频DFT bin提取。
 */

#include "utils/fft263/Goertzel10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Goertzel10::Goertzel10(QObject *parent)
    : QObject(parent)
{
    m_buffer.resize(m_windowSize);
}

Goertzel10::~Goertzel10() = default;

/* ---- Configuration ---- */

void Goertzel10::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
    // Recompute coefficients for existing target frequencies
    for (auto& state : m_states)
        state.coeff = computeCoeff(state.frequency);
}

void Goertzel10::setWindowConfig(int windowSize, double overlapRatio)
{
    m_windowSize = qMax(8, windowSize);
    m_overlapSize = qBound(0, static_cast<int>(windowSize * qBound(0.0, overlapRatio, 0.75)),
                           m_windowSize - 1);
    m_buffer.resize(m_windowSize);
    m_bufferPos = 0;
    m_bufferFill = 0;
}

void Goertzel10::addTargetFrequency(double frequency)
{
    if (frequency <= 0.0 || frequency >= m_sampleRate / 2.0) return;

    GoertzelState state;
    state.frequency = frequency;
    state.coeff = computeCoeff(frequency);
    state.s0 = state.s1 = state.s2 = 0.0;
    m_states.append(state);
    m_targetFreqs.append(frequency);
}

void Goertzel10::setTargetFrequencies(const QVector<double>& frequencies)
{
    m_states.clear();
    m_targetFreqs.clear();
    for (double f : frequencies)
        addTargetFrequency(f);
}

/* ---- Goertzel coefficient ---- */

double Goertzel10::computeCoeff(double frequency) const
{
    // k = round(N * f / Fs), coeff = 2 * cos(2*pi*k/N)
    double k = qRound(m_windowSize * frequency / m_sampleRate);
    return 2.0 * qCos(2.0 * M_PI * k / m_windowSize);
}

/* ---- Reset state ---- */

void Goertzel10::resetState(GoertzelState& state)
{
    state.s0 = state.s1 = state.s2 = 0.0;
}

/* ---- Process single sample ---- */

void Goertzel10::processSample(double sample)
{
    for (auto& state : m_states) {
        state.s0 = sample + state.coeff * state.s1 - state.s2;
        state.s2 = state.s1;
        state.s1 = state.s0;
    }
}

/* ---- Extract result ---- */

Goertzel10::BinResult Goertzel10::extractResult(const GoertzelState& state) const
{
    BinResult result;
    result.frequency = state.frequency;

    // X[k] = s1 - s2 * e^(-j*2*pi*k/N)
    double k = qRound(m_windowSize * state.frequency / m_sampleRate);
    double omega = 2.0 * M_PI * k / m_windowSize;
    double re = state.s1 - state.s2 * qCos(omega);
    double im = state.s2 * qSin(omega);

    result.magnitude = qSqrt(re * re + im * im) * 2.0 / m_windowSize;
    result.phase = qAtan2(im, re);
    result.powerDb = 20.0 * qLog10(qMax(result.magnitude, 1e-10));
    return result;
}

/* ---- Apply Hann window ---- */

void Goertzel10::applyWindow(QVector<double>& data) const
{
    int n = data.size();
    for (int i = 0; i < n; ++i) {
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        data[i] *= w;
    }
}

/* ---- Process block ---- */

QVector<Goertzel10::BinResult> Goertzel10::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<BinResult> results;

    int inputLen = samples.size();
    if (inputLen == 0 || m_states.isEmpty()) return results;

    // Sliding window with overlap-add
    for (int i = 0; i < inputLen; ++i) {
        m_buffer[m_bufferPos] = samples[i];
        m_bufferPos = (m_bufferPos + 1) % m_windowSize;
        m_bufferFill = qMin(m_bufferFill + 1, m_windowSize);

        if (m_bufferFill >= m_windowSize) {
            // Reset Goertzel states for new window
            for (auto& state : m_states)
                resetState(state);

            // Copy and window the buffer (linearized from circular)
            QVector<double> windowed(m_windowSize);
            for (int j = 0; j < m_windowSize; ++j) {
                int idx = (m_bufferPos + j) % m_windowSize;
                windowed[j] = m_buffer[idx];
            }
            applyWindow(windowed);

            // Run Goertzel on windowed data
            for (int j = 0; j < m_windowSize; ++j)
                for (auto& state : m_states) {
                    state.s0 = windowed[j] + state.coeff * state.s1 - state.s2;
                    state.s2 = state.s1;
                    state.s1 = state.s0;
                }

            // Extract results from completed window
            if (m_bufferPos % qMax(1, m_windowSize - m_overlapSize) == 0) {
                for (const auto& state : m_states)
                    results.append(extractResult(state));
            }
        }
    }

    // If no complete windows yet, try partial processing
    if (results.isEmpty() && m_bufferFill > 0) {
        for (auto& state : m_states)
            resetState(state);

        QVector<double> partial(m_bufferFill);
        int start = (m_bufferPos - m_bufferFill + m_windowSize) % m_windowSize;
        for (int j = 0; j < m_bufferFill; ++j)
            partial[j] = m_buffer[(start + j) % m_windowSize];

        for (int j = 0; j < m_bufferFill; ++j)
            for (auto& state : m_states) {
                state.s0 = partial[j] + state.coeff * state.s1 - state.s2;
                state.s2 = state.s1;
                state.s1 = state.s0;
            }

        for (const auto& state : m_states) {
            BinResult r = extractResult(state);
            r.magnitude *= static_cast<double>(m_windowSize) / m_bufferFill;
            results.append(r);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.windowSize = m_windowSize;
    m_stats.overlapSize = m_overlapSize;
    m_stats.numBins = m_states.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit binsUpdated(m_states.size(), m_windowSize, elapsed);
    return results;
}

/* ---- Accessors ---- */

int Goertzel10::windowSize() const { return m_windowSize; }

/* ---- Reset ---- */

void Goertzel10::resetStatistics()
{
    m_states.clear();
    m_targetFreqs.clear();
    m_buffer.fill(0.0);
    m_bufferPos = 0;
    m_bufferFill = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
