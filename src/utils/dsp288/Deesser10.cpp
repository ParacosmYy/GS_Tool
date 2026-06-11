/**
 * @file Deesser10.cpp
 * @brief Deesser10 实现
 *
 * 实现去齿音：频谱质心跟踪齿音检测与自适应频率选择性增益衰减。
 */

#include "utils/dsp288/Deesser10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Deesser10::Deesser10(QObject *parent)
    : QObject(parent) {}

Deesser10::~Deesser10() = default;

/* ---- Configuration ---- */

void Deesser10::setParams(const Params& p)
{
    m_params = p;
    m_params.frequency = qBound(2000.0, m_params.frequency, 16000.0);
    m_params.bandwidth = qBound(500.0, m_params.bandwidth, 8000.0);
    m_params.threshold = qBound(-60.0, m_params.threshold, 0.0);
    m_params.ratio = qBound(1.0, m_params.ratio, 20.0);
    m_params.attack = qBound(0.1, m_params.attack, 50.0);
    m_params.release = qBound(1.0, m_params.release, 500.0);
    m_params.mix = qBound(0.0, m_params.mix, 1.0);
}

void Deesser10::setSampleRate(double rate) { m_sampleRate = qBound(1.0, rate, 192000.0); }
void Deesser10::setFFTSize(int size) { m_fftSize = qBound(64, size, 8192); }

/* ---- Hann window ---- */

double Deesser10::hann(int i, int N)
{
    if (N <= 1) return 1.0;
    return 0.5 * (1.0 - qCos(2.0 * M_PI * i / (N - 1)));
}

/* ---- Bit reversal ---- */

void Deesser10::bitReverse(QVector<double>& re, QVector<double>& im, int n)
{
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
}

/* ---- In-place FFT ---- */

void Deesser10::fft(QVector<double>& re, QVector<double>& im, int n)
{
    bitReverse(re, im, n);
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                double tRe = curRe * re[v] - curIm * im[v];
                double tIm = curRe * im[v] + curIm * re[v];
                re[v] = re[u] - tRe; im[v] = im[u] - tIm;
                re[u] += tRe; im[u] += tIm;
                double nr = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = nr;
            }
        }
    }
}

/* ---- Compute magnitude spectrum ---- */

void Deesser10::computeSpectrum(const QVector<double>& frame,
                                 QVector<double>& mag, QVector<double>& freqs)
{
    int n = m_fftSize;
    QVector<double> re(n, 0.0), im(n, 0.0);
    int len = qMin(frame.size(), n);
    for (int i = 0; i < len; ++i)
        re[i] = frame[i] * hann(i, len);

    fft(re, im, n);

    int halfN = n / 2;
    mag.resize(halfN);
    freqs.resize(halfN);
    for (int i = 0; i < halfN; ++i) {
        mag[i] = qSqrt(re[i] * re[i] + im[i] * im[i]) / n;
        freqs[i] = i * m_sampleRate / n;
    }
}

/* ---- Spectral centroid ---- */

double Deesser10::spectralCentroid(const QVector<double>& mag,
                                    const QVector<double>& freqs) const
{
    double weightSum = 0.0, magSum = 0.0;
    for (int i = 0; i < mag.size(); ++i) {
        weightSum += freqs[i] * mag[i];
        magSum += mag[i];
    }
    return (magSum > 1e-15) ? weightSum / magSum : 0.0;
}

/* ---- Apply gain to frame ---- */

void Deesser10::applyGain(QVector<double>& frame, double gainDb)
{
    double linear = qPow(10.0, gainDb / 20.0);
    for (int i = 0; i < frame.size(); ++i)
        frame[i] *= linear;
}

/* ---- Main process ---- */

Deesser10::ProcessResult Deesser10::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    int n = input.size();
    if (n == 0) return result;

    int hopSize = m_fftSize / 2;
    int numFrames = (n - m_fftSize) / hopSize + 1;
    if (numFrames < 1) numFrames = 1;

    result.output.resize(n);
    result.gainReduction.resize(numFrames);
    result.centroidTrack.resize(numFrames);

    // Copy input to output
    for (int i = 0; i < n; ++i) result.output[i] = input[i];

    double attackCoeff = qExp(-1.0 / (m_params.attack * m_sampleRate / 1000.0));
    double releaseCoeff = qExp(-1.0 / (m_params.release * m_sampleRate / 1000.0));

    double fLow = m_params.frequency - m_params.bandwidth / 2.0;
    double fHigh = m_params.frequency + m_params.bandwidth / 2.0;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;

        // Extract frame
        QVector<double> frame(m_fftSize, 0.0);
        for (int i = 0; i < m_fftSize && start + i < n; ++i)
            frame[i] = input[start + i];

        // Compute spectrum
        QVector<double> mag, freqs;
        computeSpectrum(frame, mag, freqs);

        // Compute spectral centroid
        double centroid = spectralCentroid(mag, freqs);
        result.centroidTrack[f] = centroid;

        // Measure energy in sibilance band
        double sibEnergy = 0.0, totalEnergy = 0.0;
        for (int i = 0; i < mag.size(); ++i) {
            double e = mag[i] * mag[i];
            totalEnergy += e;
            if (freqs[i] >= fLow && freqs[i] <= fHigh)
                sibEnergy += e;
        }

        // Detection: high centroid + band energy above threshold
        double sibDb = 10.0 * qLog10(sibEnergy + 1e-15);
        double threshDb = m_params.threshold;
        bool sibilant = (centroid > m_params.frequency * 0.7) && (sibDb > threshDb);

        if (sibilant) {
            result.sibilanceFrames++;
            double overDb = sibDb - threshDb;
            double targetGain = -overDb * (m_params.ratio - 1.0) / m_params.ratio;
            targetGain = qMax(targetGain, -40.0);
            m_envelope = targetGain + attackCoeff * (m_envelope - targetGain);
        } else {
            m_envelope = releaseCoeff * m_envelope;
        }

        result.gainReduction[f] = m_envelope;

        // Apply gain reduction to output
        double linearGain = qPow(10.0, m_envelope / 20.0);
        double wetGain = linearGain * m_params.mix + (1.0 - m_params.mix);
        for (int i = 0; i < m_fftSize && start + i < n; ++i)
            result.output[start + i] = input[start + i] * wetGain;
    }

    double elapsed = timer.elapsed();
    m_stats.totalFrames = numFrames;
    m_stats.sibilanceDetected += result.sibilanceFrames;
    m_stats.totalOps++;
    double totalReduction = 0.0;
    for (int i = 0; i < result.gainReduction.size(); ++i) totalReduction += result.gainReduction[i];
    m_stats.avgReductionDb = (numFrames > 0) ? totalReduction / numFrames : 0.0;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processDone(numFrames, result.sibilanceFrames, m_stats.avgReductionDb, elapsed);
    return result;
}

/* ---- Reset ---- */

void Deesser10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 1.0;
}
