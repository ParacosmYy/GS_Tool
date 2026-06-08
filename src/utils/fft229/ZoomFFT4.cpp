/**
 * @file ZoomFFT4.cpp
 * @brief ZoomFFT4 实现
 *
 * 实现缩放FFT：滑动窗口变换与多相分析滤波器组窄带频谱监测。
 */

#include "utils/fft229/ZoomFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ZoomFFT4::ZoomFFT4(QObject *parent) : QObject(parent) {}
ZoomFFT4::~ZoomFFT4() = default;

/* ---- Configure ---- */

bool ZoomFFT4::configure(double sampleRate, double centerFreq,
                           double bandwidth, int fftSize)
{
    if (sampleRate <= 0 || bandwidth <= 0 || fftSize < 16) return false;
    if (bandwidth > sampleRate) return false;
    if (qAbs(centerFreq) > sampleRate / 2.0) return false;

    m_sampleRate = sampleRate;
    m_centerFreq = centerFreq;
    m_bandwidth = bandwidth;
    m_fftSize = fftSize;

    // Zoom factor = ratio of full bandwidth to zoom bandwidth
    m_zoomFactor = qMax(1, static_cast<int>(sampleRate / bandwidth));
    m_decimationFactor = m_zoomFactor;

    // Polyphase filter: M phases, each of length L/M
    m_numPhases = m_decimationFactor;
    m_filterLength = m_numPhases * 8; // 8 taps per phase
    m_filterCoeffs.resize(m_filterLength);

    designPolyphaseFilter();

    // Initialize phase state
    m_phaseState.resize(m_numPhases);
    m_phasePos.resize(m_numPhases, 0);
    int tapsPerPhase = m_filterLength / m_numPhases;
    for (int i = 0; i < m_numPhases; ++i)
        m_phaseState[i].resize(tapsPerPhase, 0.0);

    m_stats.fftSize = fftSize;
    m_stats.zoomFactor = m_zoomFactor;
    m_stats.centerFreq = centerFreq;
    m_stats.bandwidth = bandwidth;
    return true;
}

/* ---- Design polyphase filter ---- */

void ZoomFFT4::designPolyphaseFilter()
{
    int n = m_filterLength;
    double cutoff = 1.0 / m_decimationFactor;

    // Windowed sinc lowpass
    for (int i = 0; i < n; ++i) {
        double t = i - (n - 1) / 2.0;
        double sinc = (qAbs(t) < 1e-10) ? 1.0 : qSin(M_PI * t * cutoff) /
            (M_PI * t * cutoff);
        // Hann window
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (n - 1)));
        m_filterCoeffs[i] = sinc * w * cutoff;
    }
}

/* ---- Heterodyne (frequency shift) ---- */

QVector<double> ZoomFFT4::heterodyne(const QVector<double>& input) const
{
    QVector<double> shifted(input.size());
    double phaseStep = -2.0 * M_PI * m_centerFreq / m_sampleRate;
    for (int i = 0; i < input.size(); ++i) {
        double phase = phaseStep * i;
        shifted[i] = input[i] * qCos(phase);
    }
    return shifted;
}

/* ---- Polyphase decimation ---- */

QVector<double> ZoomFFT4::polyphaseDecimate(const QVector<double>& input)
{
    int tapsPerPhase = m_filterLength / m_numPhases;
    QVector<double> decimated;
    decimated.reserve(input.size() / m_decimationFactor + 1);

    int blockIdx = 0;
    for (int n = 0; n < input.size(); n += m_decimationFactor) {
        double sum = 0.0;
        for (int p = 0; p < m_numPhases; ++p) {
            int inputIdx = n + m_numPhases - 1 - p;
            if (inputIdx >= 0 && inputIdx < input.size()) {
                // Push into phase state
                m_phaseState[p][m_phasePos[p] % tapsPerPhase] = input[inputIdx];
                // FIR convolution within phase
                for (int k = 0; k < tapsPerPhase; ++k) {
                    int coeffIdx = p + k * m_numPhases;
                    int stateIdx = (m_phasePos[p] - k + tapsPerPhase) % tapsPerPhase;
                    if (coeffIdx < m_filterLength)
                        sum += m_filterCoeffs[coeffIdx] * m_phaseState[p][stateIdx];
                }
                m_phasePos[p]++;
            }
        }
        decimated.append(sum);
    }
    return decimated;
}

/* ---- DFT (magnitude spectrum) ---- */

QVector<double> ZoomFFT4::computeDFT(const QVector<double>& input) const
{
    int n = input.size();
    int half = n / 2;
    QVector<double> magnitude(half);

    for (int k = 0; k < half; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            re += input[i] * qCos(angle);
            im += input[i] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im) / n;
    }
    return magnitude;
}

/* ---- Process ---- */

QVector<double> ZoomFFT4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty() || m_filterCoeffs.isEmpty()) return {};

    // Step 1: Frequency shift to baseband
    QVector<double> shifted = heterodyne(input);

    // Step 2: Polyphase decimation
    QVector<double> decimated = polyphaseDecimate(shifted);

    // Step 3: Windowed DFT on decimated data
    int fftLen = qMin(m_fftSize, decimated.size());
    QVector<double> windowed(fftLen);
    for (int i = 0; i < fftLen; ++i) {
        // Hann window
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (fftLen - 1)));
        windowed[i] = decimated[i] * w;
    }

    QVector<double> spectrum = computeDFT(windowed);

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit spectrumReady(spectrum.size(), m_centerFreq, timer.elapsed());
    return spectrum;
}

/* ---- Frequency bins ---- */

QVector<double> ZoomFFT4::frequencyBins() const
{
    int half = m_fftSize / 2;
    double freqRes = m_bandwidth / m_fftSize;
    QVector<double> bins(half);
    for (int i = 0; i < half; ++i)
        bins[i] = m_centerFreq - m_bandwidth / 2.0 + i * freqRes;
    return bins;
}

/* ---- Reset ---- */

void ZoomFFT4::reset()
{
    int tapsPerPhase = m_filterLength / qMax(1, m_numPhases);
    for (int i = 0; i < m_numPhases; ++i) {
        m_phaseState[i].fill(0.0);
        m_phasePos[i] = 0;
    }
}

/* ---- Reset statistics ---- */

void ZoomFFT4::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
